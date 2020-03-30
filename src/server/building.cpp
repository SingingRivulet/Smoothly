#include "building.h"
#include "../utils/cJSON.h"
#include <QFile>
#include <QByteArray>

namespace smoothly{
namespace server{

building::building(int thnum):buildingSolver("building.db",thnum)
{
    cache_building_hp.parent        = this;
    cache_building_transform.parent = this;

    printf(L_GREEN "[status]" NONE "get building config\n" );
    QFile file("../config/building.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/building.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();

    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;
            while (c){
                if(c->type==cJSON_Object){
                    auto idline = cJSON_GetObjectItem(c,"id");
                    if(idline==NULL)
                        continue;
                    if(idline->type!=cJSON_Number)
                        continue;
                    int id = idline->valueint;
                    if(config.find(id)!=config.end())
                        continue;

                    auto o = new conf;
                    config[id] = o;

                    auto line = c->child;
                    while(line){
                        if(strcmp(line->string,"hp")==0){
                            if(line->type==cJSON_Number){
                                o->hp = line->valueint;
                            }
                        }
                        line = line->next;
                    }
                }
                c=c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/building.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
}

building::~building()
{
    for(auto it:config){
        delete it.second;
    }
}

void building::createBuilding(const vec3 & position, const vec3 & rotation, const std::list<std::string> & conn, int id)
{
    auto it = config.find(id);
    if(it==config.end())
        return;
    conf * c = it->second;

    std::string uuid;
    getUUID(uuid);

    char buf[512],buf2[512];
    leveldb::WriteBatch batch;

    int cx = floor(position.X/32);
    int cy = floor(position.Z/32);

    //地图区块标识
    snprintf(buf    ,sizeof(buf)    ,"B_M_%d,%d#%s" ,   cx  ,   cy  ,   uuid.c_str());
    batch.Put(buf,uuid);

    //生命值
    snprintf(buf    , sizeof(buf)  , "BH_%s"  ,uuid.c_str());
    snprintf(buf2   , sizeof(buf2) , "%d"     ,c->hp);
    batch.Put(buf,buf2);

    //坐标，旋转
    snprintf(buf    , sizeof(buf)  , "B_T_%s" ,uuid.c_str());
    snprintf(buf2   , sizeof(buf2) , "%d %f %f %f %f %f %f" ,
             id,
             position.X , position.Y , position.Z ,
             rotation.X , rotation.Y , rotation.Z);
    batch.Put(buf,buf2);

    db->Write(leveldb::WriteOptions(), &batch);

    buildingSolver.requestAdd(uuid,conn);
    if(conn.empty()){
        buildingSolver.requestSetFloor(uuid,true);
    }

    boardcast_buildingAdd(uuid , id , position , rotation ,cx,cy);
}

void building::removeBuilding(const std::string & uuid)
{
    buildingSolver.requestRemove(uuid);//通知求解器
}

void building::damageBuilding(const std::string & uuid, int dthp)
{
    try{
        int & h = cache_building_hp[uuid];
        h-=dthp;
        if(h<=0){
            removeBuilding(uuid);
        }
    }catch(std::out_of_range & ){}
}

void building::getBuildingChunk(int x, int y, std::function<void (const std::string &, const vec3 &, const vec3 &,int id)> callback)
{
    char buf[128];
    snprintf(buf,sizeof(buf),"B_M_%d,%d#",x,y);

    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());

    for(it->Seek(buf); it->Valid(); it->Next()) {

        auto k = it->key().ToString();
        auto v = it->value().ToString();

        if(!isPrefix(buf,k))
            break;//前缀不匹配，说明搜索完了

        {
            try{
                buildingStatus & s = cache_building_transform[v];
                callback(v , s.position , s.rotation , s.id);
            }catch(std::out_of_range & ){
                //缓存异常
            }catch(...){
                logError();
            }
        }

    }
    delete it;
}

void building::loop()
{
    bullet::loop();
    cache_building_hp.removeExpire();
    buildingSolver.getRemoved([&](const std::string & u){
        releaseBuilding(u);
        return true;
    });
}

void building::release(){
    bullet::release();
    cache_building_hp.clear();
}

void building::releaseBuilding(const std::string & uuid)
{
    try{
        char buf[512];
        leveldb::WriteBatch batch;

        auto position = cache_building_transform[uuid].position;

        int cx = floor(position.X/32);
        int cy = floor(position.Z/32);

        //地图区块标识
        snprintf(buf    ,sizeof(buf)    ,"B_M_%d,%d#%s" ,   cx  ,   cy  ,   uuid.c_str());
        batch.Delete(buf);

        //生命值
        snprintf(buf    , sizeof(buf)  , "BH_%s"  ,uuid.c_str());
        batch.Delete(buf);

        //坐标，旋转
        snprintf(buf    , sizeof(buf)  , "B_T_%s" ,uuid.c_str());
        batch.Delete(buf);

        boardcast_buildingRemove(uuid,cx,cy);//通知客户端

        db->Write(leveldb::WriteOptions(), &batch);

        cache_building_hp.erase(uuid);
        cache_building_transform.erase(uuid);

    }catch(std::out_of_range & ){}
}

void building::cache_building_hp_t::onExpire(const std::string & uuid, int & hp)
{
    char buf[32];
    snprintf(buf,sizeof(buf),"%d",hp);
    parent->db->Put(leveldb::WriteOptions() , std::string("BH_")+uuid , buf);
}

void building::cache_building_hp_t::onLoad(const std::string & uuid, int & hp)
{
    std::string buf;
    parent->db->Get(leveldb::ReadOptions() , std::string("BH_")+uuid , &buf);
    if(buf.empty())
        throw std::out_of_range(std::string("cann't find building:")+uuid);
    hp=atoi(buf.c_str());
}

void building::cache_building_transform_t::onExpire(const std::string &, buildingStatus &)
{
    //不允许修改
}

void building::cache_building_transform_t::onLoad(const std::string & uuid, buildingStatus & transform)
{
    std::string buf;
    parent->db->Get(leveldb::ReadOptions() , std::string("B_T_")+uuid , &buf);
    if(buf.empty())
        throw std::out_of_range(std::string("cann't find building:")+uuid);

    sscanf(buf.c_str() , "%d %f %f %f %f %f %f" ,
           &transform.id,
           &transform.position.X  , &transform.position.Y  , &transform.position.Z ,
           &transform.rotation.X  , &transform.rotation.Y  , &transform.rotation.Z);

}

}//////server
}//////smoothly
