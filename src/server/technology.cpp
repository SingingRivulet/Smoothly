#include "technology.h"
#include <QFile>

namespace smoothly{
namespace server{

void technology::sendAddr_unlockedTech(const RakNet::SystemAddress & addr,const std::string & uuid){
    try{
        tech_user_t & u = cache_tech_user[uuid];
        for(auto it:u.unlocked){
            sendAddr_unlockTech(addr,false,it);
        }
        sendAddr_techTarget(addr,false,u.target);
    }catch(...){}
}

bool technology::checkTech(const RakNet::SystemAddress & addr, const std::string & uuid, int id){
    if(id==-1)
        return true;
    try{
        tech_user_t & u = cache_tech_user[uuid];
        return u.checkTech(addr,id);
    }catch(...){
        return false;
    }
}

void technology::setTechTarget(const RakNet::SystemAddress & addr, const std::string & uuid, int id){
    try{
        tech_user_t & u = cache_tech_user[uuid];
        u.setUnlockTarget(addr,id);
    }catch(...){
    }
}

void technology::loop(){
    mailPackage::loop();
    cache_tech_user.removeExpire();
}

void technology::release(){
    cache_tech_user.clear();
    mailPackage::release();
}

technology::technology(){
    cache_tech_user.parent = this;

    //加载配置
    printf(L_GREEN "[status]" NONE "get technology config\n" );
    QFile file("../config/technology.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/technology.json\n" );
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
                if(c->type == cJSON_Object){

                    cJSON * node_id = cJSON_GetObjectItem(c,"id");
                    cJSON * node_parent = cJSON_GetObjectItem(c,"parent");
                    cJSON * node_probability = cJSON_GetObjectItem(c,"probability");

                    if(node_id && node_parent && node_probability &&
                       node_id->type==cJSON_Number && node_parent->type==cJSON_Number && node_probability->type==cJSON_Number){

                        int id = node_id->valueint;

                        tech_conf_t c;
                        c.need = node_parent->valueint;
                        c.probability = node_probability->valueint;
                        tech_conf[id] = c;
                    }

                }
                c = c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/technology.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
}

void technology::cache_tech_user_t::onExpire(const std::string & uuid, technology::tech_user_t & t){
    std::string str;
    char key[256];
    snprintf(key,sizeof(key),"tech:%s",uuid.c_str());
    t.toString(str);
    parent->db->Put(leveldb::WriteOptions(), key, str);
}

void technology::cache_tech_user_t::onLoad(const std::string & uuid, technology::tech_user_t & t){
    t.parent = parent;
    t.uuid   = uuid;
    t.tech_loop_time = 0;

    //尝试从数据库读取
    char key[256];
    snprintf(key,sizeof(key),"tech:%s",uuid.c_str());
    std::string str;

    parent->db->Get(leveldb::ReadOptions(), key , &str);
    if(!str.empty()){
        t.loadString(str);//加载
    }else{
        t.init();
    }
}

void technology::tech_user_t::init(){
    this->target = -1;
    this->target_need = -1;
    this->target_prob = -1;
    this->unlocked.clear();
}

void technology::tech_user_t::loadString(const std::string & str){
    unlocked.clear();
    auto json = cJSON_Parse(str.c_str());
    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"target")==0){
                    target = c->valueint;
                }else if(strcmp(c->string,"target_need")==0){
                    target_need = c->valueint;
                }else if(strcmp(c->string,"target_prob")==0){
                    target_prob = c->valueint;
                }
            }else if(c->type==cJSON_Array){
                if(strcmp(c->string,"unlocked")==0){
                    auto item = c->child;
                    while(item){
                        if(item->type==cJSON_Number){
                            unlocked.insert(item->valueint);
                        }
                        item = item->next;
                    }
                }
            }
            c=c->next;
        }

        cJSON_Delete(json);
    }
}

void technology::tech_user_t::toString(std::string & str){
    cJSON * json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json,"target",target);
    cJSON_AddNumberToObject(json,"target_need",target_need);
    cJSON_AddNumberToObject(json,"target_prob",target_prob);

    auto u = cJSON_CreateArray();
    cJSON_AddItemToObject(json,"unlocked",u);
    for(auto it:unlocked){
        cJSON_AddItemToArray(u,cJSON_CreateNumber(it));
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

bool technology::tech_user_t::checkTech(const RakNet::SystemAddress & addr,int id){
    if(unlocked.find(id)!=unlocked.end()){
        if(target!=-1)
            tryUnlockTech(addr,id);
        return true;
    }else{
        return false;
    }
}

void technology::tech_user_t::tryUnlockTech(const RakNet::SystemAddress & addr,int activeId){
    int maxp;
    if(target_need==activeId || target_need==-1)
        maxp = 1000;
    else
        maxp = 10000;

    int randv = rand()%maxp;
    if(target!=-1 && randv<target_prob){
        unlocked.insert(target);
        parent->sendAddr_unlockTech(addr,true,target);
        target = -1;
        target_need = -1;
        target_prob = -1;
    }
}

void technology::tech_user_t::setUnlockTarget(const RakNet::SystemAddress & addr, int id){
    auto it = parent->tech_conf.find(id);
    if(it!=parent->tech_conf.end() && unlocked.find(it->second.need)!=unlocked.end()){
        target = id;
        target_need = it->second.need;
        target_prob = it->second.probability;
    }else{
        target = -1;
        target_need = -1;
        target_prob = -1;
    }
    parent->sendAddr_techTarget(addr,true,target);
}


}
}
