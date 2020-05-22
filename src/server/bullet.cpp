#include "bullet.h"
#include <QFile>
namespace smoothly{
namespace server{
////////////////
bullet::bullet(){

    printf(L_GREEN "[status]" NONE "get fire_cost config\n" );
    QFile file("../config/fire_cost.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/fire_cost.json\n" );
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
                    auto id = cJSON_GetObjectItem(c,"id");
                    auto cost_num = cJSON_GetObjectItem(c,"cost_num");
                    auto cost_id  = cJSON_GetObjectItem(c,"cost_id");
                    auto get_num = cJSON_GetObjectItem(c,"get_num");
                    auto get_id  = cJSON_GetObjectItem(c,"get_id");
                    auto dur_cost = cJSON_GetObjectItem(c,"dur_cost");
                    if(id && cost_num && cost_id && id->type==cJSON_Number && cost_num->type==cJSON_Number && cost_id->type==cJSON_Number){
                        fire_cost f;
                        f.id        = id->valueint;
                        f.cost_id   = cost_id->valueint;
                        f.cost_num  = cost_num->valueint;
                        if(get_num && get_id && get_num->type==cJSON_Number && get_id->type==cJSON_Number){
                            f.get_id  = get_id->valueint;
                            f.get_num = get_num->valueint;
                        }
                        if(dur_cost && dur_cost->type == cJSON_Number){
                            f.dur_cost = dur_cost->valueint;
                        }
                        fire_costs[f.id] = f;
                    }
                }
                c = c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/fire_cost.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
}

void bullet::shoot(const RakNet::SystemAddress & addr,const std::string & uuid,int id,const vec3 & from,const vec3 & dir){
    try{
        auto tuuid = cache_bag_inner[uuid].usingTool;
        if(tuuid.empty())//未装备武器
            return;
        auto it = fire_costs.find(id);
        if(it!=fire_costs.end()){
            auto dur_cost = it->second.dur_cost;
            if(cache_tools[tuuid].durability < dur_cost)//耐久不足
                return;
            if(!consume(addr,uuid,tuuid,dur_cost))//消耗耐久失败
                return;
            if(it->second.cost_num!=0){
                if(!addResource(addr,uuid,it->second.cost_id,it->second.cost_num))
                    return;
            }
            if(it->second.get_num!=0){
                addResource(addr,uuid,it->second.get_id,it->second.get_num);
            }

            boardcast_shoot(uuid,id,from,dir);
        }
    }catch(...){}
}
////////////////
}//////server
}//////smoothly
