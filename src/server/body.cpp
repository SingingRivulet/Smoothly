#include "body.h"
#include "../utils/uuid.h"
#include "../utils/cJSON.h"
#include <QFile>
#include <QByteArray>
namespace smoothly{
namespace server{
////////////////
body::body(){
    cache_bodyPosi.parent = this;
    cache_lookat.parent   = this;
    cache_bodyRota.parent = this;
    config.clear();
    printf(L_GREEN "[status]" NONE "get body config\n" );
    QFile file("../config/body.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/body.json\n" );
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
                    auto idnode = cJSON_GetObjectItem(c,"id");
                    if(idnode && idnode->type==cJSON_Number){
                        int id = idnode->valueint;
                        if(config.find(id)!=config.end()){
                            printf(L_RED "[error]" NONE "can't redefine body %d\n" , id);
                        }else{
                            auto ptr  = new bconf;
                            config[id]= ptr;
                            auto item = c->child;
                            while(item){
                                if(item->type==cJSON_Number){
                                    if(strcmp(item->string,"hp")==0){
                                        ptr->hp = item->valueint;
                                    }
                                }
                                item = item->next;
                            }
                        }
                    }else{
                        printf(L_RED "[error]" NONE "can't get id\n");
                    }
                }
                c=c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/body.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
    FILE * fp = fopen("../config/wearing_bullet.txt" , "r");
    printf(L_GREEN "[status]" NONE "get 'wearing_bullet'\n" );
    if(fp){
        char buf[128];
        while(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(strlen(buf)>0){
                int firingWearingId;
                int fireId;
                int fireDelta;
                if(sscanf(buf,"%d=%d,%d" , &firingWearingId , &fireId , &fireDelta)>=3){
                    wearingToBullet[firingWearingId] = fireId;
                }
            }
        }
        fclose(fp);
    }else{
        printf(L_RED "[error]" NONE "fail to load 'wearing_bullet'\n" );
    }
}
body::~body(){
    for(auto it:config){
        delete it.second;
    }
    config.clear();
}

void body::release(){
    removeTable::release();

    cache_bodyPosi.clear();
    cache_lookat.clear();
    cache_bodyRota.clear();
}

void body::loop(){
    removeTable::loop();

    cache_bodyPosi.removeExpire();
    cache_lookat.removeExpire();
    cache_bodyRota.removeExpire();
}

void body::setChunkOwnerByBody(const std::string & user,const std::string & buuid){
    try{
        auto ow = getOwner(buuid);
        if(user!=ow)
            return;
        auto posi = cache_nodePosi[buuid];
        setChunkOwner(posi.x , posi.y , ow);
    }catch(...){}
}
void body::updateBody(const std::string & uuid , int x , int y){
    leveldb::WriteBatch batch;
    char buf[256];
    try{
        int ox,oy;
        getBody(uuid,ox,oy);
        
        if(ox!=x || oy!=y){
            snprintf(buf,sizeof(buf),"mBody%d,%d:%s" , ox , oy , uuid.c_str());
            batch.Delete(buf);
            snprintf(buf,sizeof(buf),"mBody%d,%d:%s" ,  x ,  y , uuid.c_str());
            batch.Put(buf,uuid);
        }
        
    }catch(...){
        snprintf(buf,sizeof(buf),"mBody%d,%d:%s" , x , y , uuid.c_str());
        batch.Put(buf,uuid);
    }
    
    snprintf(buf,sizeof(buf),"%d %d" , x , y);
    batch.Put(std::string("mBodyp:")+uuid , buf);
    
    db->Write(leveldb::WriteOptions(), &batch);
}
void body::removeBody(const std::string & uuid){
    leveldb::WriteBatch batch;
    try{
        int ox,oy;
        getBody(uuid,ox,oy);
        char buf[256];
        snprintf(buf,sizeof(buf),"mBody%d,%d:%s" , ox , oy , uuid.c_str());
        batch.Delete(buf);
    }catch(...){
        logCharError();
    }
    batch.Delete(std::string("mBodyp:")+uuid);
    
    db->Write(leveldb::WriteOptions(), &batch);
}
void body::getBody(const std::string & uuid , int & x , int & y){
    std::string value;
    db->Get(leveldb::ReadOptions(), std::string("mBodyp:")+uuid , &value);
    if(value.empty())
        throw std::out_of_range("getBody");
    else{
        int ox,oy;
        sscanf(value.c_str(),"%d %d",&ox,&oy);
        x=ox;
        y=oy;
    }
}
void body::getBodies(int x,int y,std::list<std::string> & o){
    o.clear();
    char prefix[256];
    snprintf(prefix,sizeof(prefix),"mBody%d,%d:",x,y);
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        
        o.push_back(v);
    }
    delete it;
}
//======================================================================
void body::wearing_add(const std::string & user,const std::string & uuid , int d){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        wearing_add(uuid,d);
    }catch(...){
        logCharError();
    }
}
void body::wearing_remove(const std::string & user,const std::string & uuid , int d){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        wearing_remove(uuid,d);
    }catch(...){
        logCharError();
    }
}
void body::wearing_get(const std::string & user,const std::string & uuid , std::set<int> & o){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        wearing_get(uuid,o);
    }catch(...){
        logCharError();
    }
}
void body::setLookAt  (const std::string & user,const std::string & uuid , const vec3 & v){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        setLookAt(uuid,v);
    }catch(...){
        logCharError();
    }
}
void body::setStatus  (const std::string & user,const std::string & uuid , int s){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        setStatus(uuid,s);
    }catch(...){
        logCharError();
    }
}
void body::setPosition(const std::string & user,const std::string & uuid , const vec3 & v){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        setPosition(uuid,v);
    }catch(...){
        logCharError();
    }
}
void body::setRotation(const std::string & user,const std::string & uuid , const vec3 & v){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        setRotation(uuid,v);
    }catch(...){
        logCharError();
    }
}
void body::interactive(const std::string & user,const std::string & uuid , const std::string & s){
    try{
        auto ow = getOwner(uuid);
        if(user!=ow)
            return;
        interactive(uuid,s);
    }catch(...){
        logCharError();
    }
}
void body::wearing_add(const std::string & uuid , int d){
    try{
        auto p = getCharPosition(uuid);
        
        char key[256];
        char val[64];
        
        snprintf(key,sizeof(key),"bChar:%s:%d",uuid.c_str(),d);
        snprintf(val,sizeof(val),"%d",d);
        db->Put(leveldb::WriteOptions(), key, val);

        auto it = wearingToBullet.find(d);
        if(it!=wearingToBullet.end()){
            snprintf(key,sizeof(key),"sType:%s", uuid.c_str());
            snprintf(val,sizeof(val),"%d",it->second);
            db->Put(leveldb::WriteOptions(), key , std::string(val));
        }

        boardcast_wearing_add(uuid , p.x , p.y , d);
        
    }catch(...){
        logCharError();
    }
}
void body::wearing_remove(const std::string & uuid , int d){
    try{
        auto p = getCharPosition(uuid);
        
        char key[256];
        snprintf(key,sizeof(key),"bChar:%s:%d",uuid.c_str(),d);
        db->Delete(leveldb::WriteOptions(), key);
        
        auto it = wearingToBullet.find(d);
        if(it!=wearingToBullet.end()){
            snprintf(key,sizeof(key),"sType:%s", uuid.c_str());
            db->Delete(leveldb::WriteOptions(), key);
        }

        boardcast_wearing_remove(uuid , p.x , p.y , d);
    }catch(...){
        logCharError();
    }
}
void body::wearing_get(const std::string & uuid , std::set<int> & o){
    char prefix[256];
    snprintf(prefix,sizeof(prefix),"bChar:%s:",uuid.c_str());
    
    o.clear();
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        {
            int vi;
            if(sscanf(v.c_str(),"%d",&vi)>0){
                o.insert(vi);
            }
        }
    }
    delete it;
    
}
void body::setRotation(const std::string & uuid , const vec3 & v){
    try{
        vec3 & ov = cache_bodyRota[uuid];
        if(fabs(ov.X-v.X)>0.01 || fabs(ov.Y!=v.Y)>0.01 || fabs(ov.Z!=v.Z)>0.01){
            ov = v;
        }else{
            return;
        }
        auto p = getCharPosition(uuid);
        boardcast_setRotation(uuid , p.x , p.y , v);

    }catch(...){
        logCharError();
    }
}
void body::setLookAt(const std::string & uuid , const vec3 & v){
    try{
        vec3 & ov = cache_lookat[uuid];
        if(fabs(ov.X-v.X)>0.01 || fabs(ov.Y!=v.Y)>0.01 || fabs(ov.Z!=v.Z)>0.01){
            ov = v;
        }else{
            return;
        }
        auto p = getCharPosition(uuid);
        boardcast_setLookAt(uuid , p.x , p.y , v);
        
    }catch(...){
        logCharError();
    }
}
void body::setPosition(const std::string & uuid , const vec3 & v){
    vec3 & ov = cache_bodyPosi[uuid];
    if(fabs(ov.X-v.X)>0.01 || fabs(ov.Y!=v.Y)>0.01 || fabs(ov.Z!=v.Z)>0.01){
        ov = v;
    }else{
        return;
    }
    
    int cx = floor(v.X/32);
    int cy = floor(v.Z/32);
    
    try{
        auto p = getCharPosition(uuid);//发送到之前的chunk，便于范围边缘玩家收到删除
        boardcast_setPosition(uuid , p.x , p.y , v);
    }catch(...){
        logCharError();
    }
    
    try{
        ipair & op = cache_nodePosi[uuid];
        if(op.x!=cx || op.y!=cy){
            auto ow = getOwner(uuid);
            updateChunkDBVT(uuid,ow,cx,cy);
            updateNode(uuid,cx,cy,[&](int i,int j){
                sendChunk(ipair(i,j),ow);
            });
        }
    }catch(...){
        logCharError();
    }
    
}
void body::sendChunk(const ipair & p , const std::string & to){
    getNode(p.x , p.y , [&](const std::string & u){
        sendToUser(u,to);
    });
    sendRemoveTable(p,to);
}
void body::sendMapToUser(const std::string & to){
    std::set<ipair> m;
    getUserNodes(to,m,[&](const std::string &,int,int){});
    for(auto it:m){
        sendChunk(it,to);
    }
}
void body::setStatus(const std::string & uuid , int s){
    try{
        auto p = getCharPosition(uuid);
        
        char key[256];
        char val[512];
        snprintf(key,sizeof(key),"bCharS:%s",uuid.c_str());
        snprintf(val,sizeof(val),"%d" , s);
        
        db->Put(leveldb::WriteOptions(), key, val);
        
        boardcast_setStatus(uuid , p.x , p.y , s);
        
    }catch(...){
        logCharError();
    }
}
void body::interactive(const std::string & uuid , const std::string & s){
    try{
        auto p = getCharPosition(uuid);
        
        boardcast_interactive(uuid , p.x , p.y , s);
        
    }catch(...){
        logCharError();
    }
}
void body::HPInc(const std::string & uuid,int delta){
    try{
        auto p = getCharPosition(uuid);

        chunkACL_t & acl = cache_chunkACL[p];//区块权限控制
        if(!acl.allowCharacterDamage){//不允许受到伤害
            return;
        }

        int hp = getHP(uuid);
        hp+=delta;
        if(hp>0){
            
            char key[256];
            char val[128];
            snprintf(key,sizeof(key),"bCharHP:%s",uuid.c_str());
            snprintf(val,sizeof(val),"%d" , hp);
            db->Put(leveldb::WriteOptions(), key, val);
            
            boardcast_hp(uuid , p.x , p.y , hp);
        }else{
            removeCharacter(uuid);
        }
    }catch(...){
        logCharError();
    }
}

std::string body::addCharacter(const std::string & owner,int id,const vec3 & posi){
    auto it = config.find(id);
    if(it==config.end()){
        characterLogger->error("addCharacter:owner={} id={}" , owner , id);
        return std::string();
    }else{
        characterLogger->info("addCharacter:owner={} id={}" , owner , id);
    }
    std::string uuid;
    getUUID(uuid);
    addCharacter(uuid,owner,id,it->second->hp,posi);
    return uuid;
}

void body::addCharacter(const std::string & uuid,const std::string & owner,int id,int hp,const vec3 & posi){
    addNode(uuid , owner , floor(posi.X/32) , floor(posi.Z/32));
    
    char key[256];
    char val[512];
    leveldb::WriteBatch batch;
    
    snprintf(key,sizeof(key),"bCharRT:%s",uuid.c_str());//旋转
    batch.Put(key,"0 0 0");
    
    snprintf(key,sizeof(key),"bCharP:%s", uuid.c_str());//位置
    snprintf(val,sizeof(val),"%f %f %f" , posi.X , posi.Y , posi.Z);
    batch.Put(key,val);
    
    snprintf(key,sizeof(key),"bCharO:%s",uuid.c_str());//owner
    batch.Put(key,owner);
    
    snprintf(key,sizeof(key),"bCharLA:%s",uuid.c_str());//look at
    batch.Put(key,"0 0 1");
    
    snprintf(key,sizeof(key),"bCharID:%s",uuid.c_str());//id
    snprintf(val,sizeof(val),"%d" , id);
    batch.Put(key,val);
    
    snprintf(key,sizeof(key),"bCharS:%s",uuid.c_str());//status
    batch.Put(key,"0");
    
    snprintf(key,sizeof(key),"bCharHP:%s",uuid.c_str());//hp
    snprintf(val,sizeof(val),"%d" , hp);
    batch.Put(key,val);
    
    db->Write(leveldb::WriteOptions(), &batch);
    
    boardcast_createBody(uuid , floor(posi.X/32) , floor(posi.Z/32) , id,hp,0,owner,posi,vec3(0,0,0),vec3(0,0,1));
}
void body::removeCharacter(const std::string & uuid){
    try{
        auto p = getCharPosition(uuid);
        boardcast_bodyRemove(uuid , p.x , p.y);
    
        removeNode(uuid);
        char key[256];
        leveldb::WriteBatch batch;
        char prefix[256];
    
        snprintf(prefix,sizeof(prefix),"bChar:%s:",uuid.c_str());
    
    
        leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
        for(it->Seek(prefix); it->Valid(); it->Next()) {
        
            auto k = it->key().ToString();
        
            if(!isPrefix(prefix,k))
                break;//前缀不匹配，说明搜索完了
        
            batch.Delete(k);
        }
        delete it;
    
        snprintf(key,sizeof(key),"bCharRT:%s",uuid.c_str());//旋转
        batch.Delete(key);
    
        snprintf(key,sizeof(key),"bCharP:%s" ,uuid.c_str());//位置
        batch.Delete(key);
    
        snprintf(key,sizeof(key),"bCharO:%s" ,uuid.c_str());//owner
        batch.Delete(key);
    
        snprintf(key,sizeof(key),"bCharLA:%s",uuid.c_str());//look at
        batch.Delete(key);
    
        snprintf(key,sizeof(key),"bCharID:%s",uuid.c_str());//id
        batch.Delete(key);
        
        snprintf(key,sizeof(key),"bCharS:%s",uuid.c_str());//status
        batch.Delete(key);
    
        snprintf(key,sizeof(key),"bCharHP:%s",uuid.c_str());//hp
        batch.Delete(key);
    
        db->Write(leveldb::WriteOptions(), &batch);

        removeFromDBVT(uuid);

        characterLogger->info("removeCharacter:{}" , uuid);
    
    }catch(...){
        characterLogger->error("removeCharacter:{}" , uuid);
    }
}
vec3 body::getPosition(const std::string & uuid){
    return cache_bodyPosi[uuid];
}
vec3 body::getRotation(const std::string & uuid){
    return cache_bodyRota[uuid];
}
vec3 body::getLookAt(const std::string & uuid){
    return cache_lookat[uuid];
}
int body::getId(const std::string & uuid){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharID:%s", uuid.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        throw std::out_of_range("getId");
    else{
        return atoi(value.c_str());
    }
}
int body::getStatus(const std::string & uuid){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharS:%s", uuid.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        throw std::out_of_range("getStatus");
    else{
        return atoi(value.c_str());
    }
}
int body::getHP(const std::string & uuid){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharHP:%s", uuid.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        throw std::out_of_range("getHP");
    else{
        return atoi(value.c_str());
    }
}
std::string body::getOwner(const std::string & uuid){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharO:%s", uuid.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        throw std::out_of_range("getOwner");
    else{
        return value;
    }
}
void body::sendToUser(const std::string & uuid,const std::string & to){
    try{
        int  id       = getId(uuid);
        int  hp       = getHP(uuid);
        int  status   = getStatus(uuid);
        auto owner    = getOwner(uuid);
        auto position = getPosition(uuid);
        auto rotation = getRotation(uuid);
        auto lookAt   = getLookAt  (uuid);
        std::set<int> wearing;
        wearing_get(uuid,wearing);
        send_body(to,uuid,id,hp,status,owner,position,rotation,lookAt,wearing);
    }catch(...){
        logCharError();
    }
}
void body::getBody(
    const std::string & uuid , 
    std::function<void (
        int id,int hp,int status,const std::string & owner,
        const vec3 & p,const vec3 & r,const vec3 & l,const std::set<int> & wearing
    )> const & callback,
    std::function<void (void)> const & fail
){
    try{
        int  id       = getId(uuid);
        int  hp       = getHP(uuid);
        int  status   = getStatus(uuid);
        auto owner    = getOwner(uuid);
        auto position = getPosition(uuid);
        auto rotation = getRotation(uuid);
        auto lookAt   = getLookAt  (uuid);
        std::set<int> wearing;
        wearing_get(uuid,wearing);
        callback(id,hp,status,owner,position,rotation,lookAt,wearing);
    }catch(...){
        fail();
    }
}
ipair body::getCharPosition(const std::string & uuid){
    auto r = getPosition(uuid);
    return ipair(floor(r.X/32) , floor(r.Z/32));
}
std::string body::getMainControl(const std::string & user){
    char key[256];
    snprintf(key,sizeof(key),"bM:%s",user.c_str());
    std::string value;
    db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        throw std::out_of_range("getMainControl");
    else{
        return value;
    }
}
void body::setMainControl(const std::string & user,const std::string & uuid){
    char key[256];
    snprintf(key,sizeof(key),"bM:%s",user.c_str());
    db->Put(leveldb::WriteOptions(), key, uuid);
}

void body::cache_bodyPosi_t::onExpire(const std::string & uuid, vec3 & v){
    char key[256];
    char val[512];
    snprintf(key,sizeof(key),"bCharP:%s", uuid.c_str());
    snprintf(val,sizeof(val),"%f %f %f" , v.X , v.Y , v.Z);
    parent->db->Put(leveldb::WriteOptions(), key, val);
}

void body::cache_bodyPosi_t::onLoad(const std::string & uuid, vec3 & p){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharP:%s", uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        p=vec3(0,0,0);
    else{
        float x,y,z;
        sscanf(value.c_str(),"%f %f %f",&x,&y,&z);
        p=vec3(x,y,z);
    }
}

void body::cache_lookat_t::onExpire(const std::string & uuid, vec3 & v){
    char key[256];
    char val[512];
    snprintf(key,sizeof(key),"bCharLA:%s",uuid.c_str());
    snprintf(val,sizeof(val),"%f %f %f" , v.X , v.Y , v.Z);

    parent->db->Put(leveldb::WriteOptions(), key, val);
}

void body::cache_lookat_t::onLoad(const std::string & uuid, vec3 & p){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharLA:%s", uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        p=vec3(0,0,0);
    else{
        float x,y,z;
        sscanf(value.c_str(),"%f %f %f",&x,&y,&z);
        p=vec3(x,y,z);
    }
}

void body::cache_bodyRota_t::onExpire(const std::string & uuid, vec3 & v){
    char key[256];
    char val[512];
    snprintf(key,sizeof(key),"bCharRT:%s",uuid.c_str());
    snprintf(val,sizeof(val),"%f %f %f" , v.X , v.Y , v.Z);

    parent->db->Put(leveldb::WriteOptions(), key, val);
}

void body::cache_bodyRota_t::onLoad(const std::string & uuid, vec3 & p){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"bCharRT:%s", uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &value);
    if(value.empty())
        p=vec3(0,0,0);
    else{
        float x,y,z;
        sscanf(value.c_str(),"%f %f %f",&x,&y,&z);
        p=vec3(x,y,z);
    }
}

////////////////
}//////server
}//////smoothly
