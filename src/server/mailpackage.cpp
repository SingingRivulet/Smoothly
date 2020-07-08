#include "mailpackage.h"

namespace smoothly{
namespace server{

mailPackage::mailPackage()
{

}

void mailPackage::pickupMailPackage(const std::string & body, const std::string & mpuuid){
    try{
        mailPackage_t p,remain;
        bool remaining = false;

        bag_inner & b = cache_bag_inner[body];

        if(getMailPackage(mpuuid,p)){
            for(auto toolUUID:p.tool){
                try{
                    b.addTool(toolUUID);
                }catch(...){
                    //放回去
                    remain.tool.insert(toolUUID);
                    remaining = true;
                }
            }
            for(auto resource:p.resource){
                if(resource.num>0){
                    if(!b.addResource(resource.id,resource.num)){
                        remain.resource.push_back(resource);
                        remaining = true;
                    }
                }
            }
            if(remaining){
                putMailPackage(mpuuid,remain);
            }else{
                removeMailPackage(mpuuid);
            }
        }
    }catch(...){}
}

bool mailPackage::getMailPackage(const std::string & mpuuid, mailPackage::mailPackage_t & p){
    std::string key = std::string("mailPackage:")+mpuuid;
    std::string val;
    db->Get(leveldb::ReadOptions() , key , &val);
    if(!val.empty()){
        p.loadString(val);
        return true;
    }
    return false;
}
void mailPackage::putMailPackage(const std::string & mpuuid, mailPackage::mailPackage_t & p){
    std::string key = std::string("mailPackage:")+mpuuid;
    std::string val;
    p.toString(val);
    db->Put(leveldb::WriteOptions() , key , val);
}

void server::mailPackage::removeMailPackage(const std::string & mpuuid){
    std::string key = std::string("mailPackage:")+mpuuid;
    db->Delete(leveldb::WriteOptions() , key);
}

std::string mailPackage::getTimeUUID(){
    int64_t tm = time(NULL);
    int64_t ut = 0x7fffffffffffffff - tm;
    std::string uuid;
    getUUID(uuid);
    char buf[128];
    snprintf(buf,128,"%16lx-%s",ut,uuid.c_str());
    return buf;
}

void mailPackage::getUserMail(const std::string & user, std::function<void(const std::string &)> callback, const std::string & startAt, int num){
    std::string prefix = std::string("userMail:")+user+":"+startAt;

    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());

    int id = 0;

    for(it->Seek(prefix); it->Valid(); it->Next()) {

        auto k = it->key().ToString();
        auto v = it->value().ToString();

        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        {
            callback(v);
        }

        ++id;
        if(id>=num)
            break;
    }
    delete it;
}

void mailPackage::addMail(const std::string & user, const std::string & uuid, const std::string & text){
    std::string key = std::string("userMail:")+user+":"+uuid;
    db->Put(leveldb::WriteOptions() , key , text);
    sendUser_newMail(user);
}

void mailPackage::deleteMail(const std::string & user, const std::string & uuid){
    std::string key = std::string("userMail:")+user+":"+uuid;
    db->Delete(leveldb::WriteOptions() , key);
}

void mailPackage::addMail(const std::string & user, const std::string & title, const std::string & text, const std::string & package){
    auto uuid = getTimeUUID();

    cJSON * json = cJSON_CreateObject();

    cJSON_AddStringToObject(json,"user"     , user.c_str());
    cJSON_AddStringToObject(json,"uuid"     , uuid.c_str());
    cJSON_AddStringToObject(json,"title"    , title.c_str());
    cJSON_AddStringToObject(json,"text"     , text.c_str());
    cJSON_AddStringToObject(json,"package"  , package.c_str());

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        addMail(user,uuid,pp);
    }
    cJSON_Delete(json);
}

void mailPackage::sendMails(const RakNet::SystemAddress & addr , const std::string & user){
    getUserMail(user,[&](const std::string & text){
        sendAddr_mail(addr , text);
    });
}

std::string server::mailPackage::putMailPackage(mailPackage::mailPackage_t & p){
    std::string uuid;
    getUUID(uuid);
    putMailPackage(uuid,p);
    return uuid;
}

void mailPackage::mailPackage_t::toString(std::string & s){
    cJSON * json = cJSON_CreateObject();

    cJSON * res = cJSON_CreateObject();//资源
    cJSON_AddItemToObject(json,"resource",res);//加入对象
    {
        char buf[64];
        for(auto it:resource){
            snprintf(buf,sizeof(buf),"%d",it.id);
            cJSON_AddNumberToObject(res,buf,it.num);
        }
    }

    cJSON * tools = cJSON_CreateArray();//工具
    cJSON_AddItemToObject(json,"tools",tools);
    {
        for(auto it:tool){
            cJSON_AddItemToArray(tools,cJSON_CreateString(it.c_str()));
        }
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        s = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

void mailPackage::mailPackage_t::loadString(const std::string & s){
    auto json = cJSON_Parse(s.c_str());

    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Object){
                if(strcmp(c->string,"resource")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_Number){
                            int id = atoi(line->string);
                            int num = line->valueint;
                            resource.push_back(resource_t(id,num));
                        }
                        line = line->next;
                    }
                }
            }else if(c->type==cJSON_Array){
                if(strcmp(c->string,"tools")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_String){
                            auto uuid = line->valuestring;
                            tool.insert(uuid);
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

}
}
