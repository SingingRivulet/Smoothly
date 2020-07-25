#include "mission.h"
#include <QString>
#include <QStringList>

namespace smoothly{

void mission::msg_addMission(const char * uuid, float x, float y, float z){
    mission_node_t *& m = missions[uuid];
    if(m){
        releaseMission(m);
        m = NULL;
    }
    cmd_getMission(uuid);

    /*
    auto n = scene->addBillboardSceneNode(0,irr::core::dimension2df(2,2),vec3(x,y,z));
    n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    n->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    n->setMaterialTexture( 0 , texture_mission_point);
    auto t = scene->createDeleteAnimator(2000);
    n->addAnimator(t);
    t->drop();
    */
}

void mission::msg_setMission(const char * uuid, const char * text){

    if(missionParentUUID == uuid){
        //是父节点
        if(missionParent){
            delete missionParent;
        }

        mission_node_t * t = new mission_node_t;
        t->loadString(text);
        t->box = NULL;
        t->uuid = uuid;
        missionParent = t;

    }else{
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
            if(t->showPosition){
                auto n = scene->addBillboardSceneNode(0,irr::core::dimension2df(2,2),t->position);
                n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
                n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
                n->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
                if(t->parent.empty())
                    n->setMaterialTexture( 0 , texture_mission_point);
                else
                    n->setMaterialTexture( 0 , texture_mission_target);
                t->node = n;
            }
            t->uuid = uuid;

            it->second = t;
        }
    }
}

void mission::msg_submitMissionStatus(const char * , bool status){
    if(status){
        printString(L"提交成功",44);
    }else{
        printString(L"提交失败",44);
    }
}

void mission::msg_missionList(const std::vector<std::string> & new_missions){
    printString(L"更新任务",64);
    for(auto it:missions){
        if(it.second)
            releaseMission(it.second);
    }
    missions.clear();
    for(auto it:new_missions){
        missions[it] = NULL;
        cmd_getMission(it.c_str());
    }
}

void mission::msg_missionText(const char * uuid, const char * text){
    if(missionParent){
        delete missionParent;
        missionParent = NULL;
    }
    missionParentUUID = uuid;
    cmd_getMission(uuid);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    QStringList list = QString(text).split("\n");
    missionText_buffer.clear();
    if(strlen(text)<=0){
        missionText_buffer.push_back(L"当前无任务，请使用P键探索任务");
    }else{
        wchar_t buf[128];
        swprintf(buf,128,L"当前任务：%s",uuid);
        missionText_buffer.push_back(buf);
        for(int i = 0; i< list.size();++i){
            missionText_buffer.push_back(conv.from_bytes(list.at(i).toStdString()));
        }
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
    submitShowingMissions = false;
    showMissionText = false;
    lastSubmitMissionsTime = 0;
    openMissionEditBox = false;
    texture_mission_point = driver->getTexture("../../res/icon/mission.png");
    texture_mission_target = driver->getTexture("../../res/icon/mission_target.png");

    //放弃按钮
    button_mission_giveup = gui->addButton(irr::core::rect<s32>(64,height-256-32,128,height-256),0,-1,L"giveup");
    button_mission_giveup->setOverrideColor(video::SColor(255,0,0,0));
    button_mission_giveup->setVisible(false);

    missionParent = NULL;

    //扫描动画
    scan_animation = scene->addSphereSceneNode(1);
    irr::video::SMaterial & m = scan_animation->getMaterial(0);
    m.BlendOperation = irr::video::EBO_ADD;
    m.BackfaceCulling = false;
    scan_animation->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    scan_animation->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    scan_animation->setMaterialType((video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                        "../shader/scan.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                        "../shader/scan.ps.glsl", "main", irr::video::EPST_PS_1_1));
    scan_animation->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    scan_animation->getMaterial(0).BlendOperation = irr::video::EBO_ADD;
    scan_animation->setMaterialTexture(0,driver->getTexture("../../res/texture/scan/1.png"));
    scan_animation->setVisible(false);
    scan_animation_time = 0;
    scan_animation_showing = false;
}

mission::~mission(){
    for(auto it:missions){
        releaseMission(it.second);
    }
    missions.clear();
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
        auto mission = ((mission_node_t*)b->data);
        auto len = (s->posi - mission->position).getLengthSQ();
        if(len<64){
            s->m->push_back(mission);
        }
    },&subm);

}

int mission::printString(const std::vector<std::wstring> & str,int start){
    int h=start;
    for(auto it:str){
        if(h>=height)
            break;
        printString(it,h);
        h+=20;
    }
    return h;
}

void mission::printString(const std::wstring & str, int h){
    ttf->draw(str.c_str() , core::rect<s32>(64,h,width,height),video::SColor(255,255,255,255));
}

void mission::drawNearMission(int h){
    mission_result.clear();
    getMission(camera->getPosition() , mission_result);
    if(mission_result.size()>0){
        auto tm = time(0);

        wchar_t buf[128];
        swprintf(buf,128,L"任务分支：%s",mission_result[0]->uuid.c_str());
        printString(buf,h);

        int s = printString(mission_result[0]->description_buffer,h+20);
        if(submitShowingMissions && tm-lastSubmitMissionsTime>1){
            lastSubmitMissionsTime = tm;
            auto acc = getMissionAccepter();
            if(!acc.empty()){
                cmd_submitMission(mission_result[0]->uuid.c_str() , acc.c_str());
                printString(L"提交……",s);
            }
        }else{
            printString(L"按E选择此任务分支",s);
        }
    }
}

void mission::onDraw(){
    technology::onDraw();
    if(showMissionText){
        auto h = printString(missionText_buffer);
        drawNearMission(h+20);
        button_mission_giveup->setVisible(!missionParentUUID.empty());
    }else{
        drawNearMission(64);
    }
}

void mission::loop(){
    technology::loop();

    if(scan_animation_showing){
        auto t = timer->getTime();
        auto delta = t-scan_animation_time;
        if(delta < 5000 && delta > 1){

            scan_animation->setVisible(true);
            scan_animation->setPosition(camera->getPosition());
            float sc = delta*0.1;
            scan_animation->setScale(vec3(sc,sc,sc));

        }else{

            scan_animation->setVisible(false);

        }
        if(delta>5000){
            scan_animation_showing = false;
        }
    }
}

void mission::addMissionWindow(){
    auto p = camera->getPosition();
    if(isMyChunk(floor(p.X/32),floor(p.Z/32)))
        openMissionEditBox = true;
}

void mission::addMission(const std::string & title, const std::string & text, bool showPosi){
    auto p = camera->getPosition();
    cmd_addMission(p.X,p.Y,p.Z,true,showPosi,missionParentUUID.c_str(),title.c_str(),text.c_str());
    wchar_t buf[128];
    swprintf(buf,128,L"添加任务：（%d,%d,%d）",(int)p.X,(int)p.Y,(int)p.Z);
    setTerrainMapMessage(buf);
}

void mission::goParentMission(){
    printf("goParentMission\n");
    cmd_goParentMission();
}

void mission::setFullMapMode(bool m){
    technology::setFullMapMode(m);
    showMissionText = m;
    if(!m)
        button_mission_giveup->setVisible(false);
}

void mission::releaseMission(mission::mission_node_t * t){
    if(t->box)
        t->box->autodrop();
    if(t->node)
        t->node->remove();
    delete t;
}

}
