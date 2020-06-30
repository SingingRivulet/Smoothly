#include "mission.h"
#include <QString>
#include <QStringList>

namespace smoothly{

void mission::msg_addMission(const char * uuid, float , float , float ){
    mission_node_t *& m = missions[uuid];
    if(m){
        releaseMission(m);
        m = NULL;
    }
    cmd_getMission(uuid);
}

void mission::msg_setMission(const char * uuid, const char * text){
    auto it = missions.find(uuid);
    if(it!=missions.end()){
        if(it->second){
            releaseMission(it->second);
        }
        mission_node_t * t = new mission_node_t;
        t->loadString(text);
        if(t->needArrive){
            t->box = missions_indexer.add(
                        t->position - vec3(2.5,2.5,2.5),
                        t->position + vec3(2.5,2.5,2.5),t);
        }else{
            t->box = NULL;
        }
        it->second = t;
    }
}

void mission::msg_submitMissionStatus(const char * uuid, bool status){

}

void mission::msg_missionList(const std::vector<std::string> & new_missions){
    for(auto it:missions){
        releaseMission(it.second);
    }
    missions.clear();
    for(auto it:new_missions){
        missions[it] = NULL;
        cmd_getMission(it.c_str());
    }
}

void mission::msg_missionText(const char * uuid, const char * text){
    missionParentUUID = uuid;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    QStringList list = QString(text).split("\n");
    missionText_buffer.clear();
    for(int i = 0; i< list.size();++i){
        missionText_buffer.push_back(conv.from_bytes(list.at(i).toStdString()));
    }
}


void mission::mission_node_t::updateBuffer(){
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    QStringList list = QString(description.c_str()).split("\n");
    description_buffer.clear();
    for(int i = 0; i< list.size();++i){
        description_buffer.push_back(conv.from_bytes(list.at(i).toStdString()));
    }
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
                    updateBuffer();
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

mission::mission(){
    showMissions = false;
    submitShowingMissions = false;
    showMissionText = false;
    lastSubmitMissionsTime = 0;
}

mission::~mission(){
    for(auto it:missions){
        releaseMission(it.second);
    }
    missions.clear();;
}

void mission::getMission(const vec3 & posi,std::vector<mission_node_t*> & m){
    struct subm_t{
            vec3 posi;
            std::vector<mission_node_t*> * m;
    }subm;
    subm.posi = posi;
    subm.m = &m;

    missions_indexer.fetchByPoint(posi,[](dbvt3d::AABB * b , void * arg){
        auto s = (subm_t*)arg;
        auto mession = ((mission_node_t*)b->data);
        auto len = (s->posi - mession->position).getLengthSQ();
        if(len<64){
            s->m->push_back(mession);
        }
    },&subm);

}

void mission::printString(const std::vector<std::wstring> & str){
    int h=64;
    for(auto it:str){
        if(h>=height)
            break;
        ttf->draw(it.c_str() , core::rect<s32>(64,h,width,height),video::SColor(255,255,255,255));
        h+=20;
    }
}

void mission::drawNearMission(){
    mession_result.clear();
    getMission(camera->getPosition() , mession_result);
    if(mession_result.size()>0){
        printString(mession_result[0]->description_buffer);
        auto tm = time(0);
        if(submitShowingMissions && tm-lastSubmitMissionsTime>1){
            lastSubmitMissionsTime = tm;
            auto acc = getMissionAccepter();
            if(!acc.empty())
                cmd_submitMission(mession_result[0]->uuid.c_str() , acc.c_str());
        }
    }
}

void mission::onDraw(){
    technology::onDraw();
    if(showMissions){
        drawNearMission();
    }else{
        if(showMissionText){
            printString(missionText_buffer);
        }
    }
}

void mission::addMissionWindow(){

}

void mission::releaseMission(mission::mission_node_t * t){
    t->box->autodrop();
    delete t;
}

}
