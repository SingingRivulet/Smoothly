#include "bag.h"
#include <QFile>
#include <QByteArray>
namespace smoothly{
namespace server{

void bag::bag_tool::toString(std::string & str){
    if(conf==NULL)
        return;
    cJSON * json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json,"id",conf->id);
    cJSON_AddNumberToObject(json,"durability",durability);
    cJSON_AddNumberToObject(json,"power",power);
    cJSON_AddStringToObject(json,"inbag",inbag.c_str());

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

void bag::bag_tool::loadString(const std::string & str){
    int id=0;
    durability = 0;
    conf = NULL;
    inbag.clear();

    bool arg1=false,arg2=false;
    auto json = cJSON_Parse(str.c_str());
    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"id")==0 && !arg1){
                    id = c->valueint;
                    arg1 = true;
                }else if(strcmp(c->string,"durability")==0 && !arg2){
                    durability = c->valueint;
                    arg2 = true;
                }else if(strcmp(c->string,"power")==0){
                    power = c->valueint;
                }
            }else if(c->type==cJSON_String){
                if(strcmp(c->string,"inbag")==0){
                    inbag = c->valuestring;
                }
            }
            c=c->next;
        }

        cJSON_Delete(json);
    }

    if(arg1 && arg2){
        auto it = parent->tool_config.find(id);
        if(it!=parent->tool_config.end()){
            this->conf = it->second;
            if(this->conf){

            }else{
                throw std::out_of_range("load bag_tool config fail");
            }
        }else{
            throw std::out_of_range("load bag_tool config fail");
        }
    }else{
        throw std::out_of_range("load bag_tool fail");
    }
}

bool bag::bag_inner::addResource(int id, int num){
    if(num==0)
        return true;
    auto cit = parent->resource_config.find(id);
    if(cit!=parent->resource_config.end()){
        bag_resource_conf * c = cit->second;
        int wei = abs(num)*c->weight;
        auto rit = resources.find(id);
        if(rit==resources.end()){
            if(num>0){
                if(wei > maxWeight-weight){
                    //超过重量
                    return false;
                }else{
                    weight+=wei;
                    resources.insert(std::pair<int,int>(id,num));
                    return true;
                }
            }else{
                return false;
            }
        }else{
            if(num>0){
                if(wei > maxWeight-weight){
                    //超过重量
                    return false;
                }else{
                    weight+=wei;
                    rit->second+=num;
                    return true;
                }
            }else{
                if(weight<wei){
                    return false;
                }
                if(rit->second < abs(num)){//数量不足
                    return false;
                }
                rit->second+=num;//num是负数
                weight-=wei;
                if(rit->second<=0){
                    resources.erase(rit);
                }
                return true;
            }
        }
    }else{
        //没有相关物品
        return false;
    }
}

void bag::bag_inner::addTool(const std::string & tuuid){
    bag_tool & tool = parent->cache_tools[tuuid];//找不到的话，会throw掉

    if(!tool.inbag.empty()){//有主的物品
        if(tool.inbag==this->uuid){//自己拥有
            if(tools.find(tuuid)!=tools.end()){
                throw std::runtime_error("reput");
            }
        }else
            throw std::runtime_error("tool.inbag!=NULL");
    }

    //检查重量
    if(tool.conf==NULL)
        throw std::runtime_error("tool.conf=NULL");
    if(tool.conf->weight > maxWeight-weight)
        throw std::runtime_error("tool.conf->weight > maxWeight-weight");

    //放入
    tools.insert(tuuid);
    tool.inbag = uuid;
    weight += tool.conf->weight;
}

void bag::bag_inner::removeTool(const std::string & uuid){
    bag_tool & tool = parent->cache_tools[uuid];//找不到的话，会throw掉

    if(tool.conf==NULL)
        throw std::runtime_error("tool.conf=NULL");

    if(tool.inbag.empty())
        throw std::runtime_error("tool.inbag=NULL");

    if(tool.inbag!=uuid)
        throw std::runtime_error("tool.inbag!=uuid");

    //丢弃在数据库端不会清除
    weight -= tool.conf->weight;
    tools.erase(uuid);
    tool.inbag.clear();
}

void bag::bag_inner::toString(std::string & str){
    cJSON * json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json,"maxWeight",maxWeight);
    cJSON_AddStringToObject(json,"usingTool",usingTool.c_str());

    cJSON * res = cJSON_CreateObject();//资源
    cJSON_AddItemToObject(json,"resource",res);//加入对象
    {
        char buf[64];
        for(auto it:resources){
            snprintf(buf,sizeof(buf),"%d",it.first);
            cJSON_AddNumberToObject(res,buf,it.second);
        }
    }

    cJSON * tool = cJSON_CreateArray();//工具
    cJSON_AddItemToObject(json,"tools",tool);
    {
        for(auto it:tools){
            cJSON_AddItemToArray(tool,cJSON_CreateString(it.c_str()));
        }
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        str = pp;
        free(pp);
    }
    cJSON_Delete(json);
}

void bag::bag_inner::loadString(const std::string & str){
    auto json = cJSON_Parse(str.c_str());

    if(json){

        auto ust = cJSON_GetObjectItem(json,"usingTool");
        if(ust && ust->type==cJSON_String){
            usingTool = ust->valuestring;
        }else{
            usingTool.clear();
        }

        auto mxd = cJSON_GetObjectItem(json,"maxWeight");
        if(mxd && mxd->type==cJSON_Number){
            maxWeight = mxd->valueint;
        }

        auto res = cJSON_GetObjectItem(json,"resource");
        if(res){
            auto line = res->child;
            while(line){
                if(line->type == cJSON_Number){
                    int id = atoi(line->string);
                    int num = line->valueint;
                    try{
                        addResource(id,num);
                    }catch(...){

                    }
                }
                line = line->next;
            }
        }

        auto tool = cJSON_GetObjectItem(json,"tools");
        if(tool){
            auto line = tool->child;
            while(line){
                if(line->type == cJSON_String){
                    auto uuid = line->valuestring;
                    try{
                        addTool(uuid);
                    }catch(...){

                    }
                }
                line = line->next;
            }
        }

        cJSON_Delete(json);
    }
}

void bag::cache_tools_t::onExpire(const std::string & uuid, bag::bag_tool & t){
    //有conf的话，写入数据库
    if(t.conf){
        std::string str;
        t.toString(str);
        char key[256];
        snprintf(key,sizeof(key),"bagTool:%s",uuid.c_str());
        parent->db->Put(leveldb::WriteOptions(), key, str);
    }
}

void bag::cache_tools_t::onLoad(const std::string & uuid, bag::bag_tool & t){
    t.parent = parent;
    //尝试从数据库读取，失败则throw
    std::string str;
    char key[256];
    snprintf(key,sizeof(key),"bagTool:%s",uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(), key , &str);
    if(str.empty())//数据库中找不到
        throw std::runtime_error("nofound");
    t.loadString(str);//这一句出错会自动throw
}

void bag::cache_bag_inner_t::onExpire(const std::string & uuid, bag::bag_inner & b){
    std::string str;
    char key[256];
    snprintf(key,sizeof(key),"bagInner:%s",uuid.c_str());
    b.toString(str);
    parent->db->Put(leveldb::WriteOptions(), key, str);
}

void bag::cache_bag_inner_t::onLoad(const std::string & uuid, bag::bag_inner & b){
    b.parent = parent;
    b.uuid   = uuid;

    //尝试从数据库读取
    char key[256];
    snprintf(key,sizeof(key),"bagInner:%s",uuid.c_str());
    std::string str;

    parent->db->Get(leveldb::ReadOptions(), key , &str);
    if(!str.empty()){
        b.loadString(str);//加载
    }else{
        //没找到的话，尝试获取id，初始化背包
        int id = parent->getId(uuid);//找不到会导致throw
        auto it = parent->maxWeights.find(id);
        if(it==parent->maxWeights.end()){
            throw std::runtime_error("it=maxWeights.end()");
        }
        b.maxWeight = it->second;
        b.weight = 0;
    }
}

std::string bag::createTool(int id){
    auto it = tool_config.find(id);
    if(it==tool_config.end())
        return std::string();

    bag_tool_conf * c = it->second;

    cJSON * json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json,"id",id);
    cJSON_AddNumberToObject(json,"durability",c->durability);
    cJSON_AddStringToObject(json,"inbag","");

    char * pp = cJSON_PrintUnformatted(json);
    std::string uuid;
    if(pp){
        getUUID(uuid);
        char key[256];
        snprintf(key,sizeof(key),"bagTool:%s",uuid.c_str());
        db->Put(leveldb::WriteOptions(), key, pp);
        free(pp);
    }
    cJSON_Delete(json);
    return uuid;
}

bool bag::consume(const RakNet::SystemAddress & addr, const std::string & bag_uuid, const std::string & tool_uuid, int num, int pwn)noexcept{
    if(bag_uuid.empty())
        return false;
    try{
        bag_tool & t = cache_tools[tool_uuid];//找不到会throw
        if(t.inbag!=bag_uuid)
            return false;
        else{
            if(t.durability<num){
                sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
                return false;
            }
            if(t.power<pwn){
                sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
                return false;
            }
            t.durability-=num;
            t.power-=pwn;
            sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
            return true;
        }
    }catch(...){
        //没找到返回false
        return false;
    }
}

bool bag::consume(const RakNet::SystemAddress & addr, const std::string & tool_uuid, int num , int pwn)noexcept{
    try{
        bag_tool & t = cache_tools[tool_uuid];//找不到会throw

        if(t.durability<num){
            sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
            return false;
        }
        if(t.power<pwn){
            sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
            return false;
        }
        t.durability-=num;
        t.power-=pwn;
        sendAddr_bag_toolDur(addr,tool_uuid,t.durability,t.power);
        return true;
    }catch(...){
        //没找到返回false
        return false;
    }
}

bag::bag(){
    cache_tools.parent = this;
    cache_bag_inner.parent = this;
    resource_config.clear();
    maxWeights.clear();
    printf(L_GREEN "[status]" NONE "get bag config\n" );
    QFile file("../config/bag.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/bag.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();
    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Object){

            auto mxd = cJSON_GetObjectItem(json,"maxWeights");
            if(mxd && mxd->type==cJSON_Array){
                printf(L_GREEN "[status]" NONE "load bag maxWeights from json\n" );
                auto line = mxd->child;
                while (line) {
                    if(line->type==cJSON_Object){

                        auto id = cJSON_GetObjectItem(line,"id");
                        auto weight = cJSON_GetObjectItem(line,"weight");
                        if(id && weight && id->type==cJSON_Number && weight->type==cJSON_Number){
                            maxWeights[id->valueint] = weight->valueint;
                        }else{
                            printf(L_RED "[error]" NONE "can not set maxWeights\n");
                        }

                    }
                    line = line->next;
                }
            }

            auto res = cJSON_GetObjectItem(json,"resource");
            if(res && res->type==cJSON_Array){
                printf(L_GREEN "[status]" NONE "load bag resource from json\n" );
                auto line = res->child;
                while (line) {
                    if(line->type==cJSON_Object){

                        auto id = cJSON_GetObjectItem(line,"id");
                        auto weight = cJSON_GetObjectItem(line,"weight");
                        if(id && weight && id->type==cJSON_Number && weight->type==cJSON_Number){

                            auto it = resource_config.find(id->valueint);
                            if(it==resource_config.end()){
                                auto p = new bag_resource_conf;
                                p->id = id->valueint;
                                p->weight = weight->valueint;
                                resource_config[id->valueint] = p;
                            }else{
                                printf(L_RED "[error]" NONE "can not redefine resource:%d\n",id->valueint);
                            }

                        }

                    }
                    line = line->next;
                }
            }

            auto tool = cJSON_GetObjectItem(json,"tool");
            if(tool && tool->type==cJSON_Array){
                printf(L_GREEN "[status]" NONE "load bag tool from json\n" );
                auto line = tool->child;
                while (line) {
                    if(line->type==cJSON_Object){

                        auto id = cJSON_GetObjectItem(line,"id");
                        auto durability = cJSON_GetObjectItem(line,"durability");
                        auto weight = cJSON_GetObjectItem(line,"weight");
                        auto power = cJSON_GetObjectItem(line,"power");
                        if(id && weight && durability && power &&
                           id->type==cJSON_Number && weight->type==cJSON_Number && durability->type==cJSON_Number && power->type==cJSON_Number){

                            auto it = tool_config.find(id->valueint);
                            if(it==tool_config.end()){
                                auto p = new bag_tool_conf;
                                p->id = id->valueint;
                                p->weight = weight->valueint;
                                p->durability = durability->valueint;
                                p->power = power->valueint;

                                auto wearing = cJSON_GetObjectItem(line,"wearing");//wearing的配置
                                if(wearing && wearing->type==cJSON_Array){
                                    auto wearing_item = wearing->child;
                                    while(wearing_item){
                                        if(wearing_item->type==cJSON_Number){
                                            p->wearing.insert(wearing_item->valueint);
                                        }
                                        wearing_item = wearing_item->next;
                                    }
                                }

                                tool_config[id->valueint] = p;
                            }else{
                                printf(L_RED "[error]" NONE "can not redefine tool:%d\n",id->valueint);
                            }

                        }

                    }
                    line = line->next;
                }
            }

        }else{
            printf(L_RED "[error]" NONE "root in ../config/bag.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
    //////////////////////////////////////////
    //test
    //连接背包
    //bag_inner & b = cache_bag_inner["df400259-237b-473e-b65f-0abaf27f46f5"];
    //测试资源添加
    //b.addResource(1001,1);
    //printf("resource num:%d\n",b.resources[1001]);
    //printf("weight:%d\n",b.weight);
    //测试添加装备
    //auto uuid = createTool(1001);
    //b.addTool(uuid);
    //for(auto it:b.tools){
    //    std::cout<<"tool:"<<it<<std::endl;
    //}
    //测试消耗耐久
    ////29b9d5f6-56d4-4bef-8fcb-2d283dcf2ec6
    //bag_tool & t = cache_tools["29b9d5f6-56d4-4bef-8fcb-2d283dcf2ec6"];
    //printf("tool dur:%d\n",t.durability);
    //consume("df400259-237b-473e-b65f-0abaf27f46f5","29b9d5f6-56d4-4bef-8fcb-2d283dcf2ec6",1);
}

bag::~bag(){
    for(auto it:tool_config){
        delete it.second;
    }
    tool_config.clear();
    for(auto it:resource_config){
        delete it.second;
    }
    resource_config.clear();
}

void bag::sendBagToAddr(const RakNet::SystemAddress & addr,const std::string & uuid){
    try{
        bag_inner & b =  cache_bag_inner[uuid];
        std::string str;
        b.toString(str);
        sendAddr_bag(addr,uuid,str);
    }catch(...){

    }
}

bool bag::addResource(const RakNet::SystemAddress & addr, const std::string & uuid, int id, int delta){
    try{
        bag_inner & b =  cache_bag_inner[uuid];
        bool res = b.addResource(id,delta);
        auto it = b.resources.find(id);
        if(it!=b.resources.end()){
            sendAddr_bag_resourceNum(addr,uuid,id,it->second);
        }else{
            sendAddr_bag_resourceNum(addr,uuid,id,0);
        }
        return res;
    }catch(...){
        return false;
    }
}

void bag::useTool(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID){
    if(uuid.empty())
        return;
    if(toolUUID.empty()){
        try{
            bag_inner & b = cache_bag_inner[uuid];//获取背包 失败会导致throw
            if(!b.usingTool.empty()){//有使用中的装备
                //卸下装备
                try{
                    bag_tool & ot = cache_tools[b.usingTool];
                    for(auto it:ot.conf->wearing){
                        wearing_remove(uuid,it);
                    }
                }catch(...){}
            }
            b.usingTool.clear();
            sendAddr_bag_tool_use(addr,uuid,"");//发送到客户端
        }catch(...){}
    }else{
        try{
            bag_tool & t = cache_tools[toolUUID];//失败会导致throw
            if(t.inbag != uuid)//不是自己的装备
                return;
            bag_inner & b = cache_bag_inner[uuid];//获取背包 失败会导致throw
            if(!b.usingTool.empty()){//有使用中的装备
                //卸下装备
                try{
                    bag_tool & ot = cache_tools[b.usingTool];
                    for(auto it:ot.conf->wearing){
                        wearing_remove(uuid,it);
                    }
                }catch(...){}
            }
            //设置新的装备
            b.usingTool = toolUUID;//先设置
            //设置wearing
            for(auto it:t.conf->wearing){
                wearing_add(uuid,it);
            }
            sendAddr_bag_tool_use(addr,uuid,toolUUID);//发送到客户端
        }catch(...){}
    }
}

void bag::reloadTool(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID){
    try{
        bag_tool & t = cache_tools[toolUUID];//失败会导致throw
        if(t.inbag!=uuid)
            return;
        t.reload();
        sendAddr_bag_toolDur(addr,toolUUID,t.durability,t.power);
    }catch(...){}
}

void bag::release(){
    cache_tools.clear();
    cache_bag_inner.clear();
    body::release();
}

void bag::loop(){
    body::loop();
    cache_tools.removeExpire();
    cache_bag_inner.removeExpire();
}

////////////////

////////////////
}
}
