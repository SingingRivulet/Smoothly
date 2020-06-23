#include "making.h"
#include <QFile>

namespace smoothly{
namespace server{

making::making(){
    printf(L_GREEN "[status]" NONE "get making config\n" );
    QFile file("../config/making.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/making.json\n" );
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
                    cJSON * node_isTool = cJSON_GetObjectItem(c,"isTool");
                    cJSON * node_needTech = cJSON_GetObjectItem(c,"needTech");
                    cJSON * node_needResource = cJSON_GetObjectItem(c,"needResource");
                    cJSON * node_outId = cJSON_GetObjectItem(c,"outId");

                    if(node_id && node_isTool && node_needTech && node_outId &&
                       node_id->type==cJSON_Number &&
                       node_isTool->type==cJSON_Number &&
                       node_needTech->type==cJSON_Number &&
                       node_outId->type==cJSON_Number){

                        making_config_t c;

                        int id = node_id->valueint;

                        c.isTool = (node_isTool->valueint!=0);
                        c.needTech = node_needTech->valueint;
                        c.outId = node_outId->valueint;

                        if(node_needResource && node_needResource->type==cJSON_Object){
                            auto line = node_needResource->child;
                            while(line){
                                if(line->type == cJSON_Number){
                                    int id = atoi(line->string);
                                    int num = line->valueint;
                                    c.needResource.push_back(std::pair<int,int>(id,num));
                                }
                                line = line->next;
                            }
                        }

                        making_config[id] = c;
                    }

                }
                c = c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/making.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
}

bool making::make(const RakNet::SystemAddress & addr, const std::string & user, const std::string & bag, int id, const vec3 & outPosition){
    try{
        auto confit = making_config.find(id);
        if(confit!=making_config.end()){

            making_config_t & conf = confit->second;

            bag_inner & b =  cache_bag_inner[bag];

            for(auto need:conf.needResource){
                auto res = b.resources.find(need.first);
                if(res==b.resources.end()){
                    return false;//资源不存在
                }
                if(res->second < need.second){
                    return false;//资源不够
                }
            }
            if(!checkTech(addr,user,conf.needTech))
                return false;//科技未解锁

            //开始制造
            for(auto need:conf.needResource){
                addResource(addr , bag , need.first , -abs(need.second));
            }
            package_item pkg;
            if(conf.isTool){
                auto tuuid = createTool(conf.outId);
                pkg.tool.insert(tuuid);
            }else{
                pkg.resource.push_back(resource_t(conf.outId,1));
            }
            pkg.skin = 0;
            pkg.position = outPosition;
            putPackage(pkg);
            return true;
        }
    }catch(...){}
    return false;
}


}
}
