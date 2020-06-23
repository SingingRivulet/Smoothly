#include "mission.h"
#include <sstream>

namespace smoothly{
namespace server{

mission::mission(int thnum):building(thnum){
    cache_mission_children.parent = this;
    cache_mission_node.parent = this;
}

void mission::loop(){
    building::loop();
    cache_mission_children.removeExpire();
    cache_mission_node.removeExpire();
}

void mission::release(){
    building::release();
    cache_mission_children.clear();
    cache_mission_node.clear();
}

bool mission::isDone(const std::string & user, const std::string & mission_uuid){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"missionDone:%s:%s",mission_uuid.c_str(),user.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    return !value.empty();
}

void mission::setDone(const std::string & user, const std::string & mission_uuid){
    char key[256];
    snprintf(key,sizeof(key),"missionDone:%s:%s",mission_uuid.c_str(),user.c_str());
    db->Put(leveldb::WriteOptions(), key , user);
}

void mission::setNowMissionParent(const std::string & user, const std::string & mission_uuid){
    char key[256];
    snprintf(key,sizeof(key),"nowMission:%s",user.c_str());
    db->Put(leveldb::WriteOptions(), key , mission_uuid);
}

void mission::getNowMissionParent(const std::string & user, std::string & mission_uuid){
    char key[256];
    snprintf(key,sizeof(key),"nowMission:%s",user.c_str());
    db->Get(leveldb::ReadOptions(), key , &mission_uuid);
}

bool mission::submitMission(const RakNet::SystemAddress & addr, const std::string & user, const std::string & body, const std::string & mission_uuid){
    try{
        mission_node_t & node = cache_mission_node[mission_uuid];
        if(!node.parent.empty()){
            if(!isDone(user , node.parent)){
                return false;
            }
        }

        if(getHP(body)<=0)//单位不存在或已死亡
            return false;

        //检查是否到达目标
        if(node.needArrive){
            auto bposi = getPosition(body);
            vec3 dpos(bposi.X - node.position.X,
                      bposi.Y - node.position.Y,
                      bposi.Z - node.position.Z);
            if((dpos.X*dpos.X + dpos.Y*dpos.Y + dpos.Z*dpos.Z)>125)
                return false;
        }

        //检查资源
        bag_inner & b = cache_bag_inner[body];
        for(auto need : node.need){
            auto res = b.resources.find(need.id);
            if(res==b.resources.end()){
                return false;//资源不存在
            }
            if(res->second < need.num){
                return false;//资源不够
            }
        }

        //接受
        mission_children_t & ch = cache_mission_children[mission_uuid];
        setNowMissionParent(user,mission_uuid);//把已经完成的任务设置为父节点
        sendAddr_missionList(addr,ch.children);//发送到客户端

        setDone(user,mission_uuid);//设置任务已完成

        //消耗资源
        for(auto need:node.need){
            addResource(addr , body , need.id , -abs(need.num));
        }
        //发放奖励

        return true;
    }catch(...){}
    return false;
}

void mission::getChunkMissions(int x, int y, std::function<void (const std::string &, const vec3 &)> callback){
    char prefix[128];
    snprintf(prefix,sizeof(prefix),"missionChunk:%d,%d:",x,y);

    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());

    for(it->Seek(prefix); it->Valid(); it->Next()) {

        auto k = it->key().ToString();
        auto v = it->value().ToString();

        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        {
            std::istringstream iss(v);
            std::string uuid;
            vec3 posi;
            iss>>uuid;
            iss>>posi.X;
            iss>>posi.Y;
            iss>>posi.Z;
            callback(uuid,posi);
        }
    }
    delete it;
}

void mission::setChunkMissions(const std::string & uuid, const vec3 & posi){
    int x = floor(posi.X/32);
    int y = floor(posi.Z/32);
    char key[128];
    char val[1024];
    snprintf(key,sizeof(key),"missionChunk:%d,%d:%s",x,y,uuid.c_str());
    snprintf(val,sizeof(val),"%s %f %f %f",uuid.c_str(),posi.X,posi.Y,posi.Z);
    db->Put(leveldb::WriteOptions(), key , val);
}

void mission::addMission(const std::string & uuid, mission::mission_node_t & m){
    std::string val;
    m.toString(val);
    char key[256];
    snprintf(key,sizeof(key),"missionNode:%s",uuid.c_str());
    db->Put(leveldb::WriteOptions(), key , val);
    if(m.parent.empty()){
        setChunkMissions(uuid,m.position);
    }
}

void mission::cache_mission_node_t::onExpire(const std::string & , mission::mission_node_t & ){
    //无需设置
}

void mission::cache_mission_node_t::onLoad(const std::string & uuid, mission::mission_node_t & n){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"missionNode:%s",uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty()){
        throw std::runtime_error("mission::cache_mission_node_t::onLoad");
    }
    n.loadString(value);
}

void mission::cache_mission_children_t::onExpire(const std::string & , mission::mission_children_t & ){
    //无需设置
}

void mission::cache_mission_children_t::onLoad(const std::string & uuid, mission::mission_children_t & n){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"missionChildren:%s",uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty()){
        throw std::runtime_error("mission::cache_mission_children_t::onLoad");
    }
    n.loadString(value);
}

void mission::mission_node_t::loadString(const std::string & s){
    position = vec3(0,0,0);
    needArrive = true;
    description.clear();
    parent.clear();
    need.clear();
    reward.clear();

    auto json = cJSON_Parse(s.c_str());

    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"x")==0){
                    position.X = c->valuedouble;
                }else if(strcmp(c->string,"y")==0){
                    position.Y = c->valuedouble;
                }else if(strcmp(c->string,"z")==0){
                    position.Z = c->valuedouble;
                }else if(strcmp(c->string,"needArrive")==0){
                    needArrive = (c->valueint!=0);
                }else if(strcmp(c->string,"showPosition")==0){
                    showPosition = (c->valueint!=0);
                }
            }else if(c->type==cJSON_String){
                if(strcmp(c->string,"parent")==0){
                    parent = c->valuestring;
                }else if(strcmp(c->string,"description")==0){
                    description = c->valuestring;
                }
            }else if(c->type==cJSON_Object){
                if(strcmp(c->string,"need")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_Number){
                            int id = atoi(line->string);
                            int num = line->valueint;
                            need.push_back(resource_t(id,num));
                        }
                        line = line->next;
                    }
                }else if(strcmp(c->string,"reward")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_Number){
                            int id = atoi(line->string);
                            int num = line->valueint;
                            reward.push_back(resource_t(id,num));
                        }
                        line = line->next;
                    }
                }
            }
            c = c->next;
        }

        cJSON_Delete(json);
    }
}

void mission::mission_node_t::toString(std::string & str){
    cJSON * json = cJSON_CreateObject();

    cJSON_AddStringToObject(json,"parent",parent.c_str());
    cJSON_AddStringToObject(json,"description",description.c_str());

    cJSON_AddNumberToObject(json,"needArrive",needArrive?1:0);
    cJSON_AddNumberToObject(json,"showPosition",showPosition?1:0);

    cJSON_AddNumberToObject(json,"x",position.X);
    cJSON_AddNumberToObject(json,"y",position.Y);
    cJSON_AddNumberToObject(json,"z",position.Z);

    cJSON * res = cJSON_CreateObject();//资源
    cJSON_AddItemToObject(json,"need",res);//加入对象
    {
        char buf[64];
        for(auto it:need){
            snprintf(buf,sizeof(buf),"%d",it.id);
            cJSON_AddNumberToObject(res,buf,it.num);
        }
    }
    res = cJSON_CreateObject();//资源
    cJSON_AddItemToObject(json,"reward",res);//加入对象
    {
        char buf[64];
        for(auto it:reward){
            snprintf(buf,sizeof(buf),"%d",it.id);
            cJSON_AddNumberToObject(res,buf,it.num);
        }
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

void mission::mission_children_t::loadString(const std::string & s){
    children.clear();
    auto json = cJSON_Parse(s.c_str());

    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Array){
                if(strcmp(c->string,"children")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_String){
                            children.push_back(line->valuestring);
                        }
                        line = line->next;
                    }
                }
            }
            c = c->next;
        }

        cJSON_Delete(json);
    }
}

void mission::mission_children_t::toString(std::string & str){
    cJSON * json = cJSON_CreateObject();

    cJSON * ch = cJSON_CreateArray();//资源
    cJSON_AddItemToObject(json,"children",ch);//加入对象
    {
        for(auto it:children){
            cJSON_AddItemToArray(ch,cJSON_CreateString(it.c_str()));
        }
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

}
}
