#include "package.h"
namespace smoothly{
namespace server{

std::string package::getPackagePrefix(int x, int y){
    char buf[256];
    snprintf(buf,sizeof(buf),"pkg#%d,%d:",x,y);
    return buf;
}

std::string package::getPackageKey(int x, int y, const std::string & uuid){
    char buf[256];
    snprintf(buf,sizeof(buf),"pkg#%d,%d:%s",x,y,uuid.c_str());
    return buf;
}

void package::putPackage(package::package_item & p){
    int cx = floor(p.position.X/32.f);
    int cy = floor(p.position.Z/32.f);
    getUUID(p.uuid);
    std::string key=getPackageKey(cx,cy,p.uuid);
    std::string value;
    p.toString(value);
    db->Put(leveldb::WriteOptions(), key, value);
    boardcast_packageAdd(cx,cy,p.uuid,value);
}

void package::putPackage(const RakNet::SystemAddress & addr, package::package_item & p, const std::string & bagUUID){
    try{
        bag_inner & b = cache_bag_inner[bagUUID];//失败的话，这一步直接return了

        //useTool(addr,bagUUID,"");
        if(!b.usingTool.empty()){//有使用中的装备
            //扔东西时会卸下装备
            try{
                bag_tool & ot = cache_tools[b.usingTool];
                for(auto it:ot.conf->wearing){
                    wearing_remove(bagUUID,it);
                }
                b.usingTool.clear();
                sendAddr_bag_tool_use(addr,bagUUID,"");//发送到客户端
            }catch(...){}
        }

        package_item putting;
        for(auto toolUUID:p.tool){
            auto tit = b.tools.find(toolUUID);
            if(tit!=b.tools.end()){//这个物体是背包拥有的
                try{
                    bag_tool & t = cache_tools[toolUUID];//找不到会导致throw
                    if(t.inbag == bagUUID && t.conf){//确认在背包中
                        b.weight -= t.conf->weight;
                        t.inbag.clear();
                        b.tools.erase(tit);
                        putting.tool.insert(toolUUID);
                        sendAddr_bag_tool_remove(addr,bagUUID,toolUUID);
                    }
                }catch(...){}
            }
        }
        for(auto resource:p.resource){
            try{
                if(resource.num>0){
                    if(b.addResource(resource.id , -resource.num)){//拿出资源
                        putting.resource.push_back(resource);
                        auto it = b.resources.find(resource.id);
                        if(it!=b.resources.end()){
                            sendAddr_bag_resourceNum(addr,bagUUID,resource.id,it->second);
                        }else{
                            sendAddr_bag_resourceNum(addr,bagUUID,resource.id,0);
                        }
                    }
                }
            }catch(...){}
        }
        putting.skin = p.skin;
        putting.position = p.position;
        putPackage(p);
    }catch(...){
    }
}

void package::putPackage_resource(const RakNet::SystemAddress & addr, const std::string & bag,int skin,const vec3 & position, int id, int num){
    if(num<=0)
        return;
    package_item p;
    p.skin = skin;
    p.position = position;
    p.resource.push_back(package_item::resource_t(id,num));
    putPackage(addr,p,bag);
}

void package::putPackage_tool(const RakNet::SystemAddress & addr, const std::string & bag,int skin,const vec3 & position, const std::string & tool){
    if(tool.empty())
        return;
    package_item p;
    p.skin = skin;
    p.position = position;
    p.tool.insert(tool);
    putPackage(addr,p,bag);
}
void package::pickupPackage(const RakNet::SystemAddress & addr,int cx, int cy, const std::string & uuid, const std::string & bagUUID){
    try{
        bag_inner & b = cache_bag_inner[bagUUID];//失败的话，这一步直接return了
        package::package_item p;
        bool remaining = false;
        package_item remain;
        if(pickupPackage(cx,cy,uuid,p)){//捡起资源
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

            std::string str;
            b.toString(str);
            sendAddr_bag(addr,bagUUID,str);

            if(remaining){
                remain.skin = p.skin;
                remain.position = p.position;
                putPackage(remain);
            }
        }
    }catch(...){}
}


#define getPackage \
    std::string value;\
    std::string key = getPackageKey(cx,cy,uuid);\
    db->Get(leveldb::ReadOptions(), key , &value);\
    if(value.empty())\
        return false;\
    p.loadString(value);

bool package::pickupPackage(int cx,int cy,const std::string & uuid, package::package_item & p){
    getPackage;
    db->Delete(leveldb::WriteOptions(), key);
    boardcast_packageRemove(cx,cy,uuid);
    return true;
}

bool package::readPackage(int cx,int cy,const std::string & uuid, package::package_item & p){
    getPackage;
    return true;
}

void package::fetchPackageByChunk(int cx, int cy, std::function<void (package::package_item &)> callback){
    std::string prefix = getPackagePrefix(cx,cy);
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());

    package_item item;

    for(it->Seek(prefix); it->Valid(); it->Next()) {

        auto k = it->key().ToString();
        auto v = it->value().ToString();

        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了

        //执行处理
        if(!v.empty()){
            item.loadString(v);
            callback(item);
        }

    }
    delete it;
}

void package::package_item::toString(std::string & str){
    cJSON * json = cJSON_CreateObject();

    cJSON_AddStringToObject(json,"uuid",uuid.c_str());
    cJSON_AddNumberToObject(json,"skin",skin);
    cJSON_AddNumberToObject(json,"x",position.X);
    cJSON_AddNumberToObject(json,"y",position.Y);
    cJSON_AddNumberToObject(json,"z",position.Z);

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
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

void package::package_item::loadString(const std::string & s){
    clear();
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
                }else if(strcmp(c->string,"skin")==0){
                    skin = c->valuedouble;
                }
            }else if(c->type==cJSON_String){
                if(strcmp(c->string,"uuid")==0){
                    uuid = c->valuestring;
                }
            }else if(c->type==cJSON_Object){
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
