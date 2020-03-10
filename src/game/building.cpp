#include "building.h"
#include <QFile>

namespace smoothly{

building::building(){
    loadConfig();
}

building::~building(){
    for(auto it:buildingChunks){
        buildingChunk * c = it.second;
        for(auto it2:c->bodies){
            //释放建筑
            releaseBuilding(it2);
        }
        delete c;
    }
    buildingChunks.clear();
    releaseConfig();
}

bool building::buildingChunkCreated(int x, int y) const{
    auto it = buildingChunks.find(ipair(x,y));
    if(it==buildingChunks.end())
        return false;
    return it->second->started;
}

void building::buildingChunkStart(int x, int y){
    seekChunk(x,y)->started = true;
}

void building::buildingChunkFetch(int x, int y){
    cmd_getBuilding(x,y);
}

void building::buildingChunkRelease(int x, int y){
    auto it = buildingChunks.find(ipair(x,y));
    if(it==buildingChunks.end())
        return;
    buildingChunk * c = it->second;
    for(auto it2:c->bodies){
        //释放建筑
        bodies.erase(it2->uuid);
        releaseBuilding(it2);
    }
    //释放建筑chunk
    delete it->second;
    buildingChunks.erase(it);
}

void building::msg_addBuilding(const char *uuid, int id, float px, float py, float pz, float rx, float ry, float rz){
    buildingAdd(vec3(px,py,pz),vec3(rx,ry,rz),id,uuid);
}

void building::msg_removeBuilding(const char *uuid){
    auto it = bodies.find(uuid);
    if(it==bodies.end())
        return;

    buildingBody * b = it->second;

    b->inchunk->bodies.erase(b);
    bodies.erase(it);
    releaseBuilding(b);
}

void building::msg_startChunk(int x, int y){
    buildingChunkStart(x,y);
}

void building::buildingAdd(const vec3 &p, const vec3 &r, int id, const std::string &uuid){
    auto it = bodies.find(uuid);
    if(it!=bodies.end())//已经存在
        return;
    int cx = floor(p.X);
    int cy = floor(p.Z);

    auto b = createBuilding(p,r,id,uuid);
    if(b==NULL)
        return;

    auto c = seekChunk(cx,cy);
    b->inchunk = c;
    c->bodies.insert(b);

    bodies[uuid]=b;
}
void building::releaseBuilding(building::buildingBody * p){
    if(p->rigidBody){
        dynamicsWorld->removeRigidBody(p->rigidBody);
        delete p->rigidBody;
    }
    if(p->bodyState)
        delete p->bodyState;
    p->node->remove();
    delete p;
}
building::buildingBody *building::createBuilding(const vec3 &p, const vec3 &r, int id, const std::string &uuid){
    auto cit = config.find(id);
    if(cit==config.end())
        return NULL;
    conf * c = cit->second;

    auto res = new buildingBody;

    //创建节点
    res->node = scene->addAnimatedMeshSceneNode(c->mesh,0,-1,p,r);
    res->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    res->node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    res->node->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    res->node->addShadowVolumeSceneNode();
    if(c->haveShader){
        res->node->setMaterialType((irr::video::E_MATERIAL_TYPE)c->shader);
    }
    res->node->updateAbsolutePosition();//更新矩阵

    res->uuid = uuid;

    res->info.ptr = res;
    res->info.type = BODY_BUILDING;

    //创建物体
    if(c->haveBody){
        res->bodyState=setMotionState(res->node->getAbsoluteTransformation().pointer());//创建状态
        res->rigidBody=createBody(c->bodyShape.compound,res->bodyState);//创建物体
        res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);//设置碰撞模式
        res->rigidBody->setUserPointer(&(res->info));
        dynamicsWorld->addRigidBody(res->rigidBody);//添加物体
    }else{
        res->bodyState=NULL;
        res->rigidBody=NULL;
    }

    return res;
}

building::buildingChunk *building::seekChunk(int x, int y){
    buildingChunk * c;
    auto it = buildingChunks.find(ipair(x,y));
    if(it==buildingChunks.end()){
        c = new buildingChunk(x,y);
        buildingChunks[ipair(x,y)] = c;
    }else
        c = it->second;
    return c;
}

void building::loadConfig(){
    config.clear();
    printf("[status]get building config\n" );
    QFile file("../config/building.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/building.json\n" );
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
                            printf("[error]can't redefine body %d\n" , id);
                        }else{
                            auto mnode = cJSON_GetObjectItem(c,"mesh");
                            if(mnode && mnode->type==cJSON_String){
                                auto mesh = scene->getMesh(mnode->valuestring);
                                if(mesh){
                                    auto ptr  = new conf;
                                    config[id]= ptr;
                                    ptr->mesh = mesh;
                                    auto item = c->child;

                                    while (item) {
                                        if(item->type==cJSON_String){
                                            if(strcmp(item->string,"body")==0){
                                                ptr->haveBody = true;
                                                ptr->bodyShape.init(item->valuestring);
                                            }
                                        }else if(item->type==cJSON_Object) {
                                            if(strcmp(item->string,"shader")==0){
                                                auto vs = cJSON_GetObjectItem(item,"vs");
                                                auto ps = cJSON_GetObjectItem(item,"ps");
                                                if(vs && ps && vs->type==cJSON_String && ps->type==cJSON_String){
                                                    ptr->haveShader = true;
                                                    ptr->shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                                                vs->valuestring, "main", irr::video::EVST_VS_1_1,
                                                                ps->valuestring, "main", irr::video::EPST_PS_1_1);
                                                }
                                            }
                                        }
                                        item = item->next;
                                    }

                                }else{
                                    printf("[error]fail to load mesh\n");
                                }
                            }else{
                                printf("[error]mesh have not been defined\n");
                            }
                        }
                    }else{
                        printf("[error]can't get id\n");
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/building.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}

void building::releaseConfig(){
    for(auto it:config){
        delete it.second;
    }
}

void building::removeBuilding(building::buildingBody * b){
    b->inchunk->bodies.erase(b);
    bodies.erase(b->uuid);
    releaseBuilding(b);
}

}
