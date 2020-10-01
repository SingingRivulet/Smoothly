#include "building.h"
#include <QFile>

namespace smoothly{

building::building(){
    buildingShaderCallback.parent = this;
    buildingPrev = NULL;
    buildingPrevConf = NULL;
    buildingPrevId = -4;
    showningDes    = 0;
    texture_collPoint = driver->getTexture("../../res/icon/collPoint.png");
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushlightuserdata(L,this);
    luaL_dofile(L, "../script/building.lua");
    loadConfig();
    addDefaultBuilding();
    pathFindingInit();
    buildingSelect = 0;
}

building::~building(){
    lua_close(L);
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

    buildingChunk * c = it->second;

    if(!it->second->started)
        return false;

    //检查上下左右
    if(c->nearx0==NULL || !c->nearx0->started)
        return false;
    if(c->nearx1==NULL || !c->nearx1->started)
        return false;
    if(c->neary0==NULL || !c->neary0->started)
        return false;
    if(c->neary1==NULL || !c->neary1->started)
        return false;

    if(c->nearx0->neary0==NULL || !c->nearx0->neary0->started)
        return false;
    if(c->nearx1->neary1==NULL || !c->nearx1->neary1->started)
        return false;
    if(c->neary0->nearx0==NULL || !c->neary0->nearx0->started)
        return false;
    if(c->neary1->nearx1==NULL || !c->neary1->nearx1->started)
        return false;

    return true;
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
    it->second->unlink();
    //释放建筑chunk
    delete it->second;
    buildingChunks.erase(it);
}

void building::loop(){
    //weather::loop();
    buildingUpdate();
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

void building::updateLOD(int x, int y, int lv){
    auto it = buildingChunks.find(ipair(x,y));
    if(it==buildingChunks.end())
        return;
    buildingChunk * c = it->second;
    for(auto it2:c->bodies){
        //更新lod
        for(int i=0;i<4;++i){
            if(it2->node[i])//重置lod
                it2->node[i]->setVisible(false);
        }
        if(lv==0)
            continue;
        for(int i=lv-1;i>=0;--i){
            //反向遍历lod列表
            if(it2->node[i]){
                //找到可用的最大lod
                it2->node[i]->setVisible(true);
                break;
            }
        }
    }
}

void building::onDraw(){
    weather::onDraw();
    int cw = screenCenter.X;
    int ch = screenCenter.Y;
    if(buildingPrev){
        //画准星

        irr::video::SColor col(64,255,255,0);
        if(buildingAllowBuild){
            col.set(255,255,0,255);
        }
        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw-10,ch),irr::core::vector2d<irr::s32>(cw-20,ch+3),col);
        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw-10,ch),irr::core::vector2d<irr::s32>(cw-20,ch-3),col);

        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw+10,ch),irr::core::vector2d<irr::s32>(cw+20,ch+3),col);
        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw+10,ch),irr::core::vector2d<irr::s32>(cw+20,ch-3),col);

        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw,ch+10),irr::core::vector2d<irr::s32>(cw+3,ch+20),col);
        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw,ch+10),irr::core::vector2d<irr::s32>(cw-3,ch+20),col);

        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw,ch-10),irr::core::vector2d<irr::s32>(cw+3,ch-20),col);
        driver->draw2DLine(irr::core::vector2d<irr::s32>(cw,ch-10),irr::core::vector2d<irr::s32>(cw-3,ch-20),col);

        if(buildingAllowBuild){

        }else{
            driver->draw2DLine(irr::core::vector2d<irr::s32>(cw-5,ch-5),irr::core::vector2d<irr::s32>(cw+5,ch+5),irr::video::SColor(255,255,0,0));
            driver->draw2DLine(irr::core::vector2d<irr::s32>(cw+5,ch-5),irr::core::vector2d<irr::s32>(cw-5,ch+5),irr::video::SColor(255,255,0,0));
        }
    }
    if(buildingPrevConf && buildingPrevConf->desTexture){
        clock_t ntm = clock();
        clock_t dtm = ntm-showningDes;
        int ms = dtm/(CLOCKS_PER_SEC/1000);
        if(ms<3000){
            driver->draw2DImage(buildingPrevConf->desTexture,
                                irr::core::rect<irr::s32>(cw-64,ch+16,cw+64,ch+16+128),
                                irr::core::rect<irr::s32>(0,0,128,128),
                                0,0,true);
        }
    }
}

bool building::getCollMap(int x, int y, int z){
    auto it = collTable.find(blockPosition(x,y,z));
    if(it==collTable.end())
        return false;
    return it->second;
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
    b->getVoxels([&](const blockPosition & p){
       markVoxel(p,1);
    });
    bodies[uuid]=b;
}
void building::releaseBuilding(building::buildingBody * p){
    if(p->rigidBody){
        dynamicsWorld->removeRigidBody(p->rigidBody);
        p->getVoxels([&](const blockPosition & posi){
            markVoxel(posi,-1);
        });
        delete p->rigidBody;
    }
    if(p->bodyState)
        delete p->bodyState;
    for(int i = 0;i<4;++i){
        if(p->node[i]){
            p->node[i]->remove();
        }
    }
    if(p->bb){
        p->bb->autodrop();
    }
    delete p;
}
building::buildingBody *building::createBuilding(const vec3 &p, const vec3 &r, int id, const std::string &uuid){
    auto cit = config.find(id);
    if(cit==config.end())
        return NULL;
    buildingConf * c = cit->second;

    auto res = new buildingBody;
    res->parent = this;

    res->config = c;

    int ml=3;
    for(int i = 0;i<4;++i){
        if(c->mesh[i]==NULL){
            ml=i-1;
            if(ml<0)
                ml=0;
            break;
        }
        //创建节点
        res->node[i] = scene->addMeshSceneNode(c->mesh[i],0,-1,p,r);
        res->node[i]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
        res->node[i]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
        res->node[i]->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        if(c->haveShader){
            res->node[i]->setMaterialType((irr::video::E_MATERIAL_TYPE)c->shader);
        }
        res->node[i]->setMaterialTexture(0,shadowMapTexture);
        res->node[i]->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
        if(c->texture){
            res->node[i]->setMaterialTexture(1,c->texture);
        }
        res->node[i]->setVisible(false);
        res->node[i]->updateAbsolutePosition();//更新矩阵
    }
    res->node[ml]->setVisible(true);//显示最低lod级别
    res->shadowNode = createShadowNode(c->mesh[ml],0,-1,p,r);//创建光影

    res->uuid = uuid;

    res->info.ptr = res;
    res->info.type = BODY_BUILDING;

    //0级lod一定存在的
    //创建物体
    if(c->haveBody){
        res->bodyState=setMotionState(res->node[0]->getAbsoluteTransformation().pointer());//创建状态
        res->rigidBody=createBody(c->bodyShape.compound,res->bodyState);//创建物体
        res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);//设置碰撞模式
        res->rigidBody->setUserPointer(&(res->info));
        dynamicsWorld->addRigidBody(res->rigidBody);//添加物体
    }else{
        res->bodyState=NULL;
        res->rigidBody=NULL;
    }

    if(c->canBuildOn){
        irr::core::aabbox3d<irr::f32> box=c->fetchBB;
        res->node[0]->getAbsoluteTransformation().transformBoxEx(box);
        res->bb = dbvt.add(box.MinEdge , box.MaxEdge , res);
    }else{
        res->bb = NULL;
    }

    return res;
}

building::buildingChunk *building::seekChunk(int x, int y){
    buildingChunk * c;
    auto it = buildingChunks.find(ipair(x,y));
    if(it==buildingChunks.end()){
        c = new buildingChunk(x,y);
        linkChunk(c,x,y);
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
                        if(id>0){
                            if(config.find(id)!=config.end()){
                                printf("[error]can't redefine body %d\n" , id);
                            }else{
                                auto mnode = cJSON_GetObjectItem(c,"mesh");
                                if(mnode && mnode->type==cJSON_String){
                                    auto mesh = scene->getMesh(mnode->valuestring);
                                    if(mesh){
                                        auto ptr  = new buildingConf;
                                        ptr->id   = id;
                                        config[id]= ptr;
                                        ptr->mesh[0] = mesh;

                                        auto meshv2_j = cJSON_GetObjectItem(c,"meshv2");
                                        if(meshv2_j && meshv2_j->type==cJSON_String){
                                            auto meshv2 = scene->getMesh(meshv2_j->valuestring);
                                            if(meshv2){
                                                ptr->mesh[1] = meshv2;
                                                auto meshv3_j = cJSON_GetObjectItem(c,"meshv3");
                                                if(meshv3_j && meshv3_j->type==cJSON_String){
                                                    auto meshv3 = scene->getMesh(meshv3_j->valuestring);
                                                    if(meshv3){
                                                        ptr->mesh[2] = meshv3;
                                                        auto meshv4_j = cJSON_GetObjectItem(c,"meshv4");
                                                        if(meshv4_j && meshv4_j->type==cJSON_String){
                                                            auto meshv4 = scene->getMesh(meshv4_j->valuestring);
                                                            if(meshv4){
                                                                ptr->mesh[3] = meshv4;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        auto item = c->child;

                                        while (item) {
                                            if(item->type==cJSON_String){
                                                if(strcmp(item->string,"body")==0){
                                                    ptr->haveBody = true;
                                                    ptr->bodyShape.init(item->valuestring);
                                                }else if(strcmp(item->string,"attachHandler")==0){
                                                    lua_settop(L,0);
                                                    lua_getglobal(L,item->valuestring);//获取函数
                                                    if(lua_isfunction(L,-1)){//检查
                                                        ptr->attachHandler = luaL_ref(L,LUA_REGISTRYINDEX);
                                                        ptr->useAttachHandler = true;
                                                    }
                                                    lua_settop(L,0);
                                                }
                                            }else if(item->type==cJSON_Object) {
                                                if(strcmp(item->string,"shader")==0){
                                                    auto vs = cJSON_GetObjectItem(item,"vs");
                                                    auto ps = cJSON_GetObjectItem(item,"ps");
                                                    if(vs && ps && vs->type==cJSON_String && ps->type==cJSON_String){
                                                        ptr->haveShader = true;
                                                        ptr->shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                                                    vs->valuestring, "main", irr::video::EVST_VS_1_1,
                                                                    ps->valuestring, "main", irr::video::EPST_PS_1_1,
                                                                    &buildingShaderCallback);
                                                    }
                                                }else if(strcmp(item->string,"fetchBB")==0){
                                                    auto from = cJSON_GetObjectItem(item,"from");
                                                    if(from){
                                                        auto x = cJSON_GetObjectItem(from,"x");
                                                        auto y = cJSON_GetObjectItem(from,"y");
                                                        auto z = cJSON_GetObjectItem(from,"z");
                                                        if(x && y && z && x->type==cJSON_Number && y->type==cJSON_Number && z->type==cJSON_Number){
                                                            ptr->fetchBB.MinEdge.set(x->valuedouble,y->valuedouble,z->valuedouble);
                                                        }
                                                    }
                                                    auto to = cJSON_GetObjectItem(item,"to");
                                                    if(to){
                                                        auto x = cJSON_GetObjectItem(to,"x");
                                                        auto y = cJSON_GetObjectItem(to,"y");
                                                        auto z = cJSON_GetObjectItem(to,"z");
                                                        if(x && y && z && x->type==cJSON_Number && y->type==cJSON_Number && z->type==cJSON_Number){
                                                            ptr->fetchBB.MaxEdge.set(x->valuedouble,y->valuedouble,z->valuedouble);
                                                        }
                                                    }
                                                }
                                            }else if(item->type==cJSON_Number){
                                                if(strcmp(item->string,"deltaHei")==0){
                                                    ptr->autoAttach.deltaHei = item->valuedouble;
                                                }else if(strcmp(item->string,"deltaHor")==0){
                                                    ptr->useAutoAttach = true;
                                                    ptr->autoAttach.deltaHor = item->valuedouble;
                                                }else if(strcmp(item->string,"canBuildOn")==0){
                                                    ptr->canBuildOn = (item->valueint!=0);
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

struct	RayResult : public btCollisionWorld::RayResultCallback
{
    RayResult(const btVector3&	rayFromWorld,const btVector3&	rayToWorld)
    :m_rayFromWorld(rayFromWorld),
    m_rayToWorld(rayToWorld)
    {
    }

    btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
    btVector3	m_rayToWorld;

    btVector3	m_hitNormalWorld;
    btVector3	m_hitPointWorld;

    std::function<bool(physical::bodyInfo *)> filter;

    virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
    {
        //caller already does the filter on the m_closestHitFraction
        btAssert(rayResult.m_hitFraction <= m_closestHitFraction);

        auto c = rayResult.m_collisionObject;
        if(c==NULL)
            return rayResult.m_hitFraction;
        auto p = (physical::bodyInfo*)c->getUserPointer();
        if(p==NULL)
            return rayResult.m_hitFraction;
        if(!filter(p))
            return rayResult.m_hitFraction;

        m_closestHitFraction = rayResult.m_hitFraction;
        m_collisionObject = rayResult.m_collisionObject;
        if (normalInWorldSpace)
        {
            m_hitNormalWorld = rayResult.m_hitNormalLocal;
        } else
        {
            ///need to transform normal into worldspace
            m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
        }
        m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
        return rayResult.m_hitFraction;
    }

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const
    {
        bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
        collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
        auto p = (btCollisionObject*)proxy0->m_clientObject;
        if(p){
            auto i = (building::bodyInfo*)p->getUserPointer();
            if(i){
                collides = collides && (i->type==building::BODY_TERRAIN || i->type==building::BODY_BUILDING);
            }
        }
        return collides;
    }
};
void building::fetchByRay(const vec3 &from, const vec3 &to, std::function<void (const vec3 &, physical::bodyInfo *)> callback, std::function<bool(bodyInfo *)> filter){
    btVector3 bfrom(from.X,from.Y,from.Z),bto(to.X,to.Y,to.Z);//转换为bullet向量
    RayResult rayCallback(bfrom,bto);
    rayCallback.filter = filter;
    dynamicsWorld->rayTest(bfrom, bto, rayCallback);//使用bullet的rayTest接口
    if (rayCallback.hasHit()){
        vec3 targ(rayCallback.m_hitPointWorld.getX(),
                  rayCallback.m_hitPointWorld.getY(),
                  rayCallback.m_hitPointWorld.getZ());

        auto o = rayCallback.m_collisionObject;
        auto i = (bodyInfo*)o->getUserPointer();
        callback(targ , i);
    }
}

void building::buildingStart(){
    if(buildingPrev!=NULL)
        return;
    if(unlockedBuilding.find(buildingPrevId)==unlockedBuilding.end())
        return;
    auto cit = config.find(buildingPrevId);
    if(cit==config.end())
        return;
    buildingPrevConf    = cit->second;
    buildingAllowBuild  = false;
    buildingTarget      = NULL;
    buildingPrev = scene->addMeshSceneNode(buildingPrevConf->mesh[0]);
    buildingPrev->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    buildingPrev->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    buildingPrev->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    buildingPrev->setMaterialTexture(0,shadowMapTexture);
    if(buildingPrevConf->texture){
        buildingPrev->setMaterialTexture(1,buildingPrevConf->texture);
    }

    if(buildingPrevConf->haveShader){
        buildingPrev->setMaterialType((irr::video::E_MATERIAL_TYPE)buildingPrevConf->shader);
    }
}

void building::buildingUpdate(){
    if(buildingPrev==NULL)
        return;
    showningDes         = clock();

    auto ori    = camera->getPosition();
    auto dir    = camera->getTarget()-ori;
    dir.normalize();
    auto start  = ori;
    auto end    = ori+dir*20;

    vec3 camAng = dir.getHorizontalAngle();//摄像机的旋转角度
    buildingPrev->setPosition(end);
    buildingPrev->setRotation(camAng);

    buildingAllowBuild = false;

    fetchByRay(start , end,[&](const vec3 & p,bodyInfo * b){
        //获取到瞄准点
        buildingTarget = NULL;
        if(b->type==BODY_TERRAIN){
            buildingAllowBuild = true;
            auto ang=dir.getHorizontalAngle();
            ang.Z=0;
            ang.X=0;
            buildingPrev->setPosition(p);//更新n的坐标
            buildingPrev->setRotation(ang);//更新n的旋转欧拉角
            return;
        }else
        if(b->type==BODY_BUILDING){
            auto tb = (buildingBody*)b->ptr;
            if(tb){
                buildingAllowBuild = true;
                buildingTarget = tb;
                if(tb->config->id==buildingPrevId && buildingPrevConf->useAutoAttach){
                    //自动吸附
                    irr::core::vector2df p1,p2,p3,p4;

                    vec3 rt(0,tb->node[0]->getRotation().Y,0);
                    vec3 dir=rt.rotationToDirection();

                    p1.set(dir.X,dir.Z);
                    p2=-p1;
                    p3=p1;
                    p4=p1;
                    rotate2d(p3, M_PI/2);
                    rotate2d(p4, -M_PI/2);

                    auto dhor = buildingPrevConf->autoAttach.deltaHor;
                    p1.normalize();p1*=dhor;
                    p2.normalize();p2*=dhor;
                    p3.normalize();p3*=dhor;
                    p4.normalize();p4*=dhor;

                    std::vector<vec3> rp;

                    auto dhei = buildingPrevConf->autoAttach.deltaHei;
                    vec3 trg = tb->node[0]->getPosition();

                    if(buildingPrevConf->autoAttach.allowed[0])rp.push_back(vec3(p1.X+trg.X , trg.Y+dhei , p1.Y+trg.Z));//计算出四个吸附点位置
                    if(buildingPrevConf->autoAttach.allowed[1])rp.push_back(vec3(p2.X+trg.X , trg.Y+dhei , p2.Y+trg.Z));
                    if(buildingPrevConf->autoAttach.allowed[2])rp.push_back(vec3(p3.X+trg.X , trg.Y+dhei , p3.Y+trg.Z));
                    if(buildingPrevConf->autoAttach.allowed[3])rp.push_back(vec3(p4.X+trg.X , trg.Y+dhei , p4.Y+trg.Z));

                    float mind = mhtDist(rp[0],p);
                    int minp   = 0;

                    for(uint i=0;i<rp.size();i++){
                        float tmp = mhtDist(rp[i],p);//计算出曼哈顿距离，找出最小的
                        //printf("%f ",tmp);
                        if(tmp<mind){
                            mind = tmp;
                            minp = i;
                        }
                    }

                    buildingPrev->setPosition(rp[minp]);//更新n的坐标
                    buildingPrev->setRotation(rt);//更新n的旋转欧拉角
                    return;
                }else{
                    //自定义处理方式
                    if(buildingPrevConf->useAttachHandler){
                        lua_settop(L,0);//初始化lua堆栈
                        lua_rawgeti(L,LUA_REGISTRYINDEX,buildingPrevConf->attachHandler);//得到函数
                        if(lua_isfunction(L,-1)){

                            lua_newtable(L);//创建数组

                            lua_newtable(L);//瞄准位置
                            lua_pushnumber(L,p.X);
                            lua_rawseti(L, -2, 1);
                            lua_pushnumber(L,p.Y);
                            lua_rawseti(L, -2, 2);
                            lua_pushnumber(L,p.Z);
                            lua_rawseti(L, -2, 3);

                            lua_rawseti(L, -2, 1);

                            lua_newtable(L);//目标物体位置
                            auto trg = tb->node[0]->getPosition();
                            lua_pushnumber(L,trg.X);
                            lua_rawseti(L, -2, 1);
                            lua_pushnumber(L,trg.Y);
                            lua_rawseti(L, -2, 2);
                            lua_pushnumber(L,trg.Z);
                            lua_rawseti(L, -2, 3);

                            lua_rawseti(L, -2, 2);

                            lua_newtable(L);//目标物体旋转
                            auto trgRot = tb->node[0]->getRotation();
                            lua_pushnumber(L,trgRot.X);
                            lua_rawseti(L, -2, 1);
                            lua_pushnumber(L,trgRot.Y);
                            lua_rawseti(L, -2, 2);
                            lua_pushnumber(L,trgRot.Z);
                            lua_rawseti(L, -2, 3);

                            lua_rawseti(L, -2, 3);

                            lua_newtable(L);//相机位置
                            lua_pushnumber(L,start.X);
                            lua_rawseti(L, -2, 1);
                            lua_pushnumber(L,start.Y);
                            lua_rawseti(L, -2, 2);
                            lua_pushnumber(L,start.Z);
                            lua_rawseti(L, -2, 3);

                            lua_rawseti(L, -2, 4);

                            lua_pushinteger(L,tb->config->id);//目标id
                            lua_rawseti(L, -2, 5);

                            if(lua_pcall(L, 1, 1, 0) != 0){
                                printf("[building:%d]script error %s\n",buildingPrevId, lua_tostring(L,-1));
                            }else{
                                if(lua_istable(L,-1) && luaL_len(L,-1)>=6){

                                    vec3 outPosi,outRot;

                                    lua_rawgeti(L,-1,1);
                                    outPosi.X=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_rawgeti(L,-1,2);
                                    outPosi.Y=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_rawgeti(L,-1,3);
                                    outPosi.Z=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_rawgeti(L,-1,4);
                                    outRot.X=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_rawgeti(L,-1,5);
                                    outRot.Y=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_rawgeti(L,-1,6);
                                    outRot.Z=lua_tonumber(L,-1);
                                    lua_pop(L,1);

                                    lua_settop(L,0);
                                    buildingPrev->setPosition(outPosi);//更新n的坐标
                                    buildingPrev->setRotation(outRot); //更新n的旋转欧拉角
                                    return;
                                }
                            }
                        }
                        lua_settop(L,0);
                    }
                }
            }
        }
        auto ang=dir.getHorizontalAngle();
        ang.Z=0;
        ang.X=0;
        buildingPrev->setPosition(p);//更新n的坐标
        buildingPrev->setRotation(ang);//更新n的旋转欧拉角
    });

    buildingPrev->updateAbsolutePosition();
}

void building::buildingApply()
{
    if(buildingPrev==NULL || buildingAllowBuild==false)
        return;
    auto p = buildingPrev->getPosition();
    auto r = buildingPrev->getRotation();

    irr::core::aabbox3d<irr::f32> box=buildingPrevConf->fetchBB;
    buildingPrev->getAbsoluteTransformation().transformBoxEx(box);

    {
        int count=0;
        dbvt3d::AABB bb;
        bb.from=box.MinEdge;
        bb.to=box.MaxEdge;
        dbvt.collisionTest(&bb,[](dbvt3d::AABB *  , void * argp){
            auto arg=(int*)argp;
            ++(*arg);
        },&count);
        if(count>8)
            return;
    }

    cmd_addBuilding(buildingPrevId , p.X , p.Y , p.Z , r.X , r.Y , r.Z,[&](RakNet::BitStream * bs){
        std::unordered_set<std::string> cons;
        if(buildingTarget)
            cons.insert(buildingTarget->uuid);
        else{
            //建地面上
            return;
        }

        dbvt3d::AABB bb;
        bb.from=box.MinEdge;
        bb.to=box.MaxEdge;
        dbvt.collisionTest(&bb,[](dbvt3d::AABB * box , void * argp){//搜索附近的建筑物
            auto bd=(buildingBody*)(box->data);
            auto arg=(std::unordered_set<std::string>*)argp;
            arg->insert(bd->uuid);
        },&cons);

        for(auto it:cons)
            bs->Write(RakNet::RakString(it.c_str()));
    });

    if(buildingPrev)
        buildingPrev->remove();
    buildingPrev = NULL;
    buildingPrevConf = NULL;
    buildingAllowBuild = false;
}

void building::buildingEnd(bool apply){
    if(buildingAllowBuild && apply)
        buildingApply();
    if(buildingPrev)
        buildingPrev->remove();
    buildingPrev = NULL;
    buildingPrevConf = NULL;
    buildingAllowBuild=false;
}

void building::switchBuilding(){
    if(availableBuilding.empty())
        return;
    int id;
    try{
        id = availableBuilding.at(buildingSelect);
    }catch(...){
        buildingSelect=0;
        try{
            id = availableBuilding.at(buildingSelect);
        }catch(...){
            return;
        }
    }

    buildingPrevId = id;
    auto cit = config.find(buildingPrevId);
    if(cit==config.end())
        return;

    buildingPrevConf = cit->second;

    showningDes         = clock();

    ++buildingSelect;
}

void building::selectBuilding(int id){
    if(availableBuilding.empty())
        return;

    buildingPrevId = id;
    auto cit = config.find(buildingPrevId);
    if(cit==config.end())
        return;

    buildingPrevConf = cit->second;

    showningDes         = clock();

    ++buildingSelect;
}

void building::cancle(){
    buildingEnd(false);
}

void building::removeBuilding(building::buildingBody * b){
    b->inchunk->bodies.erase(b);
    bodies.erase(b->uuid);
    releaseBuilding(b);
}

void building::buildingChunk::unlink(){
    if(nearx0){
        nearx0->nearx1 = NULL;
        nearx0 = NULL;
    }
    if(nearx1){
        nearx1->nearx0 = NULL;
        nearx1 = NULL;
    }
    if(neary0){
        neary0->neary1 = NULL;
        neary0 = NULL;
    }
    if(neary1){
        neary1->neary0 = NULL;
        neary1 = NULL;
    }
}
#define findChunk(x,y) \
    auto it = buildingChunks.find(ipair(x,y)); \
    if(it!=buildingChunks.end())

void building::linkChunk(building::buildingChunk * c, int x, int y){
    {
        findChunk(x+1,y){
            c->nearx1 = it->second;
            it->second->nearx0 = c;
        }else{
            c->nearx1 = NULL;
        }
    }
    {
        findChunk(x-1,y){
            c->nearx0 = it->second;
            it->second->nearx1 = c;
        }else{
            c->nearx0 = NULL;
        }
    }
    {
        findChunk(x,y+1){
            c->neary1 = it->second;
            it->second->neary0 = c;
        }else{
            c->neary1 = NULL;
        }
    }
    {
        findChunk(x,y-1){
            c->neary0 = it->second;
            it->second->neary1 = c;
        }else{
            c->neary0 = NULL;
        }
    }
}

void building::BuildingShaderCallback::OnSetConstants(video::IMaterialRendererServices * services, s32 userData){
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMapSize"),&parent->shadowMapSize, 1);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipY"),&parent->clipY, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipYUp"),&parent->clipYUp, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("enableClipY"),&parent->enableClipY, 1);

    s32 var0 = 0;
    s32 var1 = 1;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMap"),&var0, 1);

    irr::f32 sas = parent->scan_animation_showing;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_showing"),&sas, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_size"),&parent->scan_animation_size, 1);

    auto campos = parent->camera->getPosition();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("campos"),&campos.X, 3);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowFactor"),&parent->shadowFactor, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("tex"),&var1, 1);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("shadowMatrix") , parent->shadowMatrix.pointer() , 16);
    core::matrix4 world = parent->driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("modelMatrix") , world.pointer() , 16);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("transformMatrix") , (parent->camera->getProjectionMatrix()*parent->camera->getViewMatrix()).pointer() , 16);
}

}
