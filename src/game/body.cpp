#include "body.h"

namespace smoothly{

void body::setBodyPosition(const std::string & uuid , const vec3 & posi){
    int cx = floor(posi.X/32);
    int cy = floor(posi.Z/32);

    auto it=bodies.find(uuid);
    if(it!=bodies.end()){
        bodyItem * b = it->second;
        if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的
            setCharacterChunk(uuid,cx,cy);
        }else{
            if(!chunkLoaded(cx,cy)){//在chunk外，删除物体
                releaseBody(b);
                bodies.erase(it);
                return;
            }
        }
        b->m_character.setPosition(posi);
    }
}

void body::removeBody(const std::string & uuid){
    auto it=bodies.find(uuid);
    if(it!=bodies.end()){
        releaseBody(it->second);
        bodies.erase(it);
        myBodies.erase(it->first);
        selectedBodies.erase(it->second);
    }
}

void body::bodyItem::updateFromWorld(){
    if(node==NULL)
        return;
    btTransform transform;
    m_character.getTransform(transform);

    //从world复制到scene
    btVector3 btPos;
    btVector3 btRot;
    vec3 irrPos;
    vec3 irrRot;

    btPos = transform.getOrigin();
    irrPos.set(btPos.x(), btPos.y(), btPos.z());

    const btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    irrRot.X = irr::core::radToDeg(btRot.x());
    irrRot.Y = irr::core::radToDeg(btRot.y());
    irrRot.Z = irr::core::radToDeg(btRot.z());

    node->animateJoints(true,&animationBlend);

    bool needUpdateStatus = false;
    if(owner==parent->myUUID){//拥有的物体
        //尝试上传
        if(fabs(lastPosition.X-irrPos.X)>0.01 || fabs(lastPosition.Y!=irrPos.Y)>0.01 || fabs(lastPosition.Z!=irrPos.Z)>0.01){
            node->setPosition(irrPos);
            lastPosition = irrPos;
            parent->cmd_setPosition(uuid,irrPos.X,irrPos.Y,irrPos.Z);
        }
        if(fabs(lastRotation.X-irrRot.X)>0.01 || fabs(lastRotation.Y!=irrRot.Y)>0.01 || fabs(lastRotation.Z!=irrRot.Z)>0.01){
            node->setRotation(irrRot);
            lastRotation = irrRot;
            parent->cmd_setRotation(uuid,irrRot.X,irrRot.Y,irrRot.Z);
        }
        if(fabs(lastLookAt.X-lookAt.X)>0.01 || fabs(lastLookAt.Y!=lookAt.Y)>0.01 || fabs(lastLookAt.Z!=lookAt.Z)>0.01){
            lastLookAt = lookAt;
            parent->cmd_setLookAt(uuid,lookAt.X,lookAt.Y,lookAt.Z);
        }
        if(lastStatus!=status_mask){
            lastStatus = status_mask;
            parent->cmd_setStatus(uuid , status_mask);
            needUpdateStatus = true;
        }
    }else{
        node->setRotation(irrRot);
        node->setPosition(irrPos);
        if(lastStatus!=status_mask){
            lastStatus = status_mask;
            needUpdateStatus = true;
        }
    }
    int pitchAngle = getPitchAngle();
    if(pitchAngle!=lastPitchAngle){
        needUpdateStatus = true;
        lastPitchAngle   = pitchAngle;
    }

    if(needUpdateStatus)
        updateStatus();

    //setPositionByTransform(node,t);

    //如果是视角物体，更新摄像机
    if(uuid==parent->mainControl){
        parent->camera->setPosition(irrPos+vec3(0,config->deltaY,0));
    }

    coll_rigidBody->getMotionState()->setWorldTransform(transform);
}

void body::bodyItem::doAnimation(float speed, int start, int end,float frame, bool loop){
    if(node==NULL)
        return;
    node->setAnimationSpeed(speed);
    node->setLoopMode(loop);
    node->setFrameLoop(start,end);
    node->setCurrentFrame(frame);
}

void body::bodyItem::loadBag(const char * str){
    auto json = cJSON_Parse(str);

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
                        resources[id]=num;
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
                        tools.insert(uuid);
                        parent->cmd_getBagTool(uuid);
                    }catch(...){

                    }
                }
                line = line->next;
            }
        }

        cJSON_Delete(json);
    }
}

void body::bodyItem::doFire(){
    auto ntm = parent->timer->getRealTime();
    auto deltaFireTime = ntm - lastFireTime;
    if(fireDelta>0 && fireDelta < deltaFireTime){
        lastFireTime = ntm;
        auto wnode = wearing.find(firingWearingId);
        if(wnode!=wearing.end()){
            irr::scene::ISceneNode * n = wnode->second;
            node->updateAbsolutePosition();
            n->updateAbsolutePosition();
            auto mat = n->getAbsoluteTransformation();//变换矩阵
            auto cit = parent->wearingConfig.find(firingWearingId);//找到配置
            if(cit!=parent->wearingConfig.end()){
                wearingConf * c = cit->second;
                vec3 p = c->skillFrom;//技能释放点
                mat.transformVect(p);//变换到角色的手上
                //消耗弹药
                auto it = parent->fire_costs.find(fireId);
                if(it!=parent->fire_costs.end()){

                    int ndur=0,npwr=0;

                    if(!usingTool.empty()){
                        auto t = parent->tools.find(usingTool);
                        if(t!=parent->tools.end()){
                            ndur = t->second.dur;
                            npwr = t->second.pwr;
                            if(it->second.dur_cost!=0){//需要耐久
                                if(t->second.dur >= it->second.dur_cost){
                                    ndur = t->second.dur-it->second.dur_cost;
                                    parent->needUpdateUI = true;
                                }else{
                                    return;
                                }
                            }
                            if(it->second.pwr_cost!=0){//需要能量
                                if(t->second.pwr >= it->second.pwr_cost){
                                    npwr = t->second.pwr-it->second.pwr_cost;
                                    parent->needUpdateUI = true;
                                }else{
                                    return;
                                }
                            }

                            if(it->second.cost_num!=0){//需要弹药
                                auto bgr = resources.find(it->second.cost_id);//定位资源
                                if(bgr!=resources.end()){
                                    int nnm = bgr->second + it->second.cost_num;
                                    if(nnm>=0)
                                        bgr->second = nnm;
                                    else
                                        return;
                                    parent->needUpdateUI = true;
                                }else{
                                    return;
                                }
                            }

                            t->second.dur = ndur;
                            t->second.pwr = npwr;

                        }else{
                            return;
                        }
                    }else{
                        return;
                    }

                }else{
                    return;
                }
                parent->fireTo(uuid,fireId,p,lookAt);//执行开火
            }
        }
    }
}

void body::bodyItem::reloadTool(){
    if(!usingTool.empty()){
        parent->cmd_reloadBagTool(uuid.c_str(),usingTool.c_str());
    }
}

void body::bodyItem::reloadStart(){
    if(reloading)
        return;
    if(usingTool.empty())
        return;
    if(reloadNeedTime<=0)
        return;
    reloading = true;
    reloadStartTime = parent->timer->getTime();
}

void body::bodyItem::reloadEnd(){
    if(!reloading)
        return;
    reloading = false;
    auto ntm = parent->timer->getTime();
    auto delta = ntm - reloadStartTime;
    if(delta>reloadNeedTime)
        reloadTool();
}

void body::bodyItem::ghostTest(const btTransform & t, std::function<void (physical::bodyInfo *)> callback){
    if(parent==NULL)
        return;
    btPairCachingGhostObject ghost;//ghost对象
    ghost.setCollisionShape(m_character.getShape());
    ghost.setWorldTransform(t);

    parent->dynamicsWorld->addCollisionObject(&ghost);//加入世界

    for (int i = 0; i < ghost.getNumOverlappingObjects(); i++){
        btCollisionObject *btco = ghost.getOverlappingObject(i);

        auto info = (bodyInfo*)(btco->getUserPointer());

        if(info)
            callback(info);

    }

    parent->dynamicsWorld->removeCollisionObject(&ghost);
}

void body::bodyItem::setFollow(body::bodyItem * p){
    if(follow){
        follow->followers.erase(this);
        follow = NULL;
    }
    if(p){
        p->followers.insert(this);
        follow = p;
    }
    for(auto it:followers){
        it->follow=NULL;
    }
    followers.clear();
}

float body::bodyItem::getPitchAngle(){
    auto dir = lookAt;
    dir.normalize();
    auto hor = dir;
    hor.Y = 0;//水平方向向量

    if(hor.getLengthSQ()==0)//垂直
        return 90;

    hor.normalize();

    auto dot = dir.dotProduct(hor);

    auto dig = irr::core::radToDeg(acos(dot));//得到角度

    if(dir.Y>0)
        return dig;
    else
        return -dig;
}

body::bodyItem::bodyItem(btScalar w,btScalar h,const btVector3 & position,bool wis,bool jis):
    m_character(w,h,position,wis,jis){
    firing          = false;
    firingWearingId = 0;
    fireDelta       = 0;
}

void body::bodyAmCallback::OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode *){
    parent->updateStatus(true);
}
void body::bodyItem::updateStatus(bool finish){
    if(!config->haveAniCallback)
        return;
    auto nowFrame = node->getFrameNr();
    lua_settop(parent->L , 0);
    lua_rawgeti(parent->L,LUA_REGISTRYINDEX,config->aniCallback);
    if(lua_isfunction(parent->L,-1)){

        lua_newtable(parent->L);//创建主数组

        lua_pushstring(parent->L,"status");
        lua_newtable(parent->L);
        {

            lua_pushstring(parent->L,"pitchAngle");
            lua_pushinteger(parent->L, lastPitchAngle);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"walk");
            lua_newtable(parent->L);
            {
                lua_pushstring(parent->L,"forward");
                lua_pushinteger(parent->L, status.walk_forward);
                lua_settable(parent->L, -3);

                lua_pushstring(parent->L,"leftOrRight");
                lua_pushinteger(parent->L, status.walk_leftOrRight);
                lua_settable(parent->L, -3);
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"useLeft");
            lua_pushboolean(parent->L, status.useLeft);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"useRight");
            lua_pushboolean(parent->L, status.useRight);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"posture");
            if(status.bodyPosture==bodyStatus::BS_BODY_STAND){
                lua_pushstring(parent->L, "stand");
            }else if(status.bodyPosture==bodyStatus::BS_BODY_SQUAT){
                lua_pushstring(parent->L, "squat");
            }else if(status.bodyPosture==bodyStatus::BS_BODY_SIT){
                lua_pushstring(parent->L, "sit");
            }else if(status.bodyPosture==bodyStatus::BS_BODY_RIDE){
                lua_pushstring(parent->L, "ride");
            }else if(status.bodyPosture==bodyStatus::BS_BODY_LIE){
                lua_pushstring(parent->L, "lie");
            }else if(status.bodyPosture==bodyStatus::BS_BODY_LIEP){
                lua_pushstring(parent->L, "liep");
            }else{
                lua_pushstring(parent->L, "");
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"hand");
            if(status.handPosture==bodyStatus::BS_HAND_NONE){
                lua_pushstring(parent->L, "none");
            }else if(status.handPosture==bodyStatus::BS_HAND_AIM){
                lua_pushstring(parent->L, "aim");
            }else if(status.handPosture==bodyStatus::BS_HAND_THROW){
                lua_pushstring(parent->L, "throw");
            }else if(status.handPosture==bodyStatus::BS_HAND_BUILD){
                lua_pushstring(parent->L, "build");
            }else if(status.handPosture==bodyStatus::BS_HAND_BUILDP){
                lua_pushstring(parent->L, "buildp");
            }else if(status.handPosture==bodyStatus::BS_HAND_OPERATE){
                lua_pushstring(parent->L, "operate");
            }else if(status.handPosture==bodyStatus::BS_HAND_LIFT){
                lua_pushstring(parent->L, "lift");
            }else{
                lua_pushstring(parent->L, "");
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"mask");
            lua_pushinteger(parent->L, status_mask);
            lua_settable(parent->L, -3);

        }
        lua_settable(parent->L, -3);

        lua_pushstring(parent->L,"finish");
        lua_pushboolean(parent->L,finish);
        lua_settable(parent->L, -3);

        lua_pushstring(parent->L,"nowFrame");
        lua_pushnumber(parent->L,nowFrame);
        lua_settable(parent->L, -3);

        // do the call (1 arguments, 1 result)
        if (lua_pcall(parent->L, 1, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(parent->L, -1));
        else{
            if(lua_istable(parent->L,-1)){

                float speed = 1;
                int start = 1;
                int end = 1;
                float frame = 1;
                bool loop = false;

                lua_pushstring(parent->L, "speed");
                lua_gettable(parent->L, -2);
                if(lua_isnumber(parent->L,-1)){
                    speed = lua_tonumber(parent->L,-1);
                }
                lua_pop(parent->L,1);

                lua_pushstring(parent->L, "frame");
                lua_gettable(parent->L, -2);
                if(lua_isnumber(parent->L,-1)){
                    frame = lua_tonumber(parent->L,-1);
                }
                lua_pop(parent->L,1);

                lua_pushstring(parent->L, "start");
                lua_gettable(parent->L, -2);
                if(lua_isinteger(parent->L,-1)){
                    start = lua_tointeger(parent->L,-1);
                }
                lua_pop(parent->L,1);

                lua_pushstring(parent->L, "end");
                lua_gettable(parent->L, -2);
                if(lua_isinteger(parent->L,-1)){
                    end = lua_tointeger(parent->L,-1);
                }
                lua_pop(parent->L,1);

                lua_pushstring(parent->L, "loop");
                lua_gettable(parent->L, -2);
                if(lua_isboolean(parent->L,-1)){
                    loop = lua_toboolean(parent->L,-1);
                }
                lua_pop(parent->L,1);

                lua_pushstring(parent->L, "blend");
                lua_gettable(parent->L, -2);
                if(lua_istable(parent->L,-1)){
                    //blend
                    animationBlend.clear();
                    int len = luaL_len(parent->L,-1);
                    for (int i = 1; i <= len; ++i) {
                        lua_rawgeti(parent->L, -1, i);
                        if(lua_istable(parent->L,-1) && luaL_len(parent->L,-1)>=2){

                            bool getArrOk = true;

                            int amid;
                            lua_rawgeti(parent->L, -1, 1);
                            if(lua_isinteger(parent->L,-1)){
                                amid = lua_tointeger(parent->L,-1);
                                if(amid<0 || amid >= config->animation.size()){
                                    getArrOk = false;
                                }
                            }else
                                getArrOk = false;
                            lua_pop(parent->L,1);

                            float blend;
                            lua_rawgeti(parent->L, -1, 2);
                            if(lua_isnumber(parent->L,-1)){
                                blend = lua_tonumber(parent->L,-1);
                            }else
                                getArrOk = false;
                            lua_pop(parent->L,1);

                            try{
                                if(getArrOk){
                                    animationBlend.push_back(
                                                irr::scene::IAnimatedMeshSceneNode::IAnimationBlend(
                                                    config->animation.at(amid),blend
                                                    )
                                                );
                                }
                            }catch(...){}

                        }else if(lua_isnumber(parent->L,-1)){
                            //自己的动画
                            animationBlend.push_back(
                                        irr::scene::IAnimatedMeshSceneNode::IAnimationBlend(
                                                (irr::scene::ISkinnedMesh*)config->mesh,
                                                lua_tonumber(parent->L,-1)
                                            )
                                        );
                        }
                        lua_pop(parent->L, 1);
                    }
                    if(animationBlend.empty())
                        setBlenderAsDefault();
                }else{
                    setBlenderAsDefault();
                }
                lua_pop(parent->L,1);

                doAnimation(speed,start,end,frame,loop);
            }
        }

    }
    lua_settop(parent->L , 0);
}

void body::bodyItem::interactive(const char * t){
    parent->setInteractiveNode(this , t);
}

void body::addBody(const std::string & uuid,int id,int hp,int32_t sta_mask,const std::string & owner,const vec3 & posi,const vec3 & r,const vec3 & l){
    auto it = bodyConfig.find(id);
    if(it==bodyConfig.end())
        return;

    if(bodies.find(uuid)!=bodies.end())
        return;

    bodyConf * c = it->second;

    int cx = floor(posi.X/32);
    int cy = floor(posi.Z/32);
    if(owner==myUUID && (!myUUID.empty())){//是自己拥有的
        setCharacterChunk(uuid,cx,cy);
    }else{
        //if(!chunkLoaded(cx,cy)){//在chunk外，删除物体
        //    return;
        //}
    }

    bodyItem * p = new bodyItem(c->width , c->height , btVector3(posi.X,posi.Y,posi.Z) , c->walkInSky , c->jumpInSky);
    p->config = c;
    p->follow = NULL;
    p->fireDelta = 0;
    p->m_character.world = dynamicsWorld;
    p->m_character.addIntoWorld();
    p->m_character.setRotation(r);
    p->m_character.setDir(l);
    p->m_character.setUserPointer(&(p->info));

    p->info.ptr  = p;
    p->info.type = BODY_BODY;

    p->lookAt = l;

    p->lastLookAt   = l;
    p->lastPosition = posi;
    p->lastRotation = r;
    p->lastPitchAngle = p->getPitchAngle();

    p->uuid   = uuid;
    p->owner  = owner;
    p->id     = id;
    p->hp     = hp;
    p->reloading = false;
    p->reloadStartTime = 0;
    p->status = sta_mask;
    p->status_mask = sta_mask;
    p->lastStatus = sta_mask;
    p->node   = scene->addAnimatedMeshSceneNode(c->mesh);
    p->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    p->node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );

    irr::scene::ISkinnedMesh* skinnedMesh = reinterpret_cast<irr::scene::ISkinnedMesh*>(p->node->getMesh());
    skinnedMesh->animationOverrideCallback = &p->jointCallback;

    p->setBlenderAsDefault();

    p->node->setJointMode(irr::scene::EJUOR_CONTROL);//设为控制模式，因为要混合

    if(c->texture){
        p->node->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        p->node->setMaterialTexture( 0 , c->texture);
    }
    p->parent = this;

    //设置回调函数
    bodyAmCallback * cb = new bodyAmCallback;
    cb->parent = p;
    p->node->setAnimationEndCallback(cb);
    cb->drop();

    //设置碰撞刚体
    p->coll_bodyState=setMotionState(p->node->getAbsoluteTransformation().pointer());
    p->coll_rigidBody =createBody(p->m_character.getShape(),p->coll_bodyState);
    p->coll_rigidBody->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    p->coll_rigidBody->setActivationState(DISABLE_DEACTIVATION);
    p->coll_rigidBody->setFriction(0.7);
    p->coll_rigidBody->setRestitution(0.1);
    p->coll_rigidBody->setUserPointer(&(p->coll_rigidBodyInfo));
    dynamicsWorld->addRigidBody(p->coll_rigidBody);
    p->coll_rigidBodyInfo.type = BODY_BODY_PART;
    p->coll_rigidBodyInfo.ptr  = &p->m_character;

    p->updateFromWorld();
    p->updateStatus();

    if(chunkCreated(cx,cy)){
        p->m_character.setGravity(gravity);
        p->uncreatedChunk = false;
    }else{
        p->m_character.setGravity(btVector3(0,0,0));
        p->uncreatedChunk = true;
    }

    bodies[uuid] = p;
    if(owner==myUUID && (!myUUID.empty()))
        myBodies[uuid] = p;

    if(uuid==mainControl){
        mainControlBody = p;
    }
}
void body::setWearing(bodyItem * n, const std::set<int> & wearing){
    std::set<int> rm;
    for(auto it:n->wearing){
        rm.insert(it.first);
    }
    for(auto it:wearing){
        rm.erase(it);
        addWearing(n , it);
    }
    for(auto it:rm){
        removeWearing(n , it);
    }
}
void body::addWearing(bodyItem * n, int wearing){
    if(n->wearing.find(wearing)!=n->wearing.end())
        return;
    if(!addWearingNode(n , wearing))
        return;
    auto it = wearingToBullet.find(wearing);
    if(it!=wearingToBullet.end()){
        n->firingWearingId  = wearing;
        n->fireId           = it->second.id;
        n->fireDelta        = it->second.deltaTime;
        n->reloadEnd();
        n->reloadNeedTime   = it->second.reloadTime;
    }
}
void body::removeWearing(bodyItem * n, int wearing){
    auto it = n->wearing.find(wearing);
    if(it==n->wearing.end())
        return;
    it->second->remove();
    n->wearing.erase(it);
    auto nit = wearingToBullet.find(wearing);
    if(nit!=wearingToBullet.end()){
        n->firingWearingId  = 0;
        n->fireId           = 0;
        n->fireDelta        = 0;
        n->reloadEnd();
        n->reloadNeedTime   = 0;
    }
}
bool body::addWearingNode(bodyItem * n, int wearing){
    auto it = wearingConfig.find(wearing);
    if(it==wearingConfig.end())
        return false;
    if(it->second->attach.empty())
        return false;

    auto attachit = it->second->attach.find(n->id);
    if(attachit==it->second->attach.end())
        return false;

    auto attach = attachit->second;

    auto pn = n->node->getJointNode(attach.c_str());
    if(pn==NULL){
        return false;
    }
    auto wn = scene->addAnimatedMeshSceneNode(it->second->mesh , pn);
    if(it->second->texture){
        wn->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        wn->setMaterialTexture( 0 , it->second->texture);
    }
    n->wearing[wearing] = wn;
    return true;
}
body::body():gravity(0,-10,0){
    mainControlBody = NULL;
    L = luaL_newstate();
    luaL_openlibs(L);
    if(luaL_dofile(L, "../script/body.lua")){
        printf("[body]%s \n",lua_tostring(L, -1));
    }
    loadBodyConfig();
    loadWearingConfig();
    loadFireCost();
    selecting = false;

    int cx = width/2;
    int cy = height/2;
    bag_icons = gui->addEmptySpriteBank("bag_icons");
    loadBagIcons();

    body_bag_resource = gui->addListBox(irr::core::rect<irr::s32>(cx-200,cy-128,cx+200,cy+144),0,-1,true);
    body_bag_resource->setSpriteBank(bag_icons);
    bagPage = 0;
    body_bag_resource->setItemHeight(32);
    body_bag_resource->setVisible(false);
    gui->addImage(driver->getTexture("../../res/icon/bag_footer.png"),irr::core::vector2di(0,256),true,body_bag_resource);
    body_bag_page = NULL;

    body_bag_using = gui->addListBox(irr::core::rect<irr::s32>(20,20,200,52),0,-1,false);
    body_bag_using->setSpriteBank(bag_icons);
    body_bag_using->setItemHeight(32);
    body_bag_using->setVisible(false);

    needUpdateUI = false;
    bag_selectId = -1;
}

body::~body(){
    lua_close(L);
    releaseBodyConfig();
    releaseWearingConfig();
}

void body::msg_setVisualRange(int v){
    setVisualRange(v);
}

void body::setPositionByTransform(irr::scene::ISceneNode * node, const btTransform & transform){
    btVector3 btPos;
    btVector3 btRot;
    vec3 irrPos;
    vec3 irrRot;

    btPos = transform.getOrigin();
    irrPos.set(btPos.x(), btPos.y(), btPos.z());

    const btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    irrRot.X = irr::core::radToDeg(btRot.x());
    irrRot.Y = irr::core::radToDeg(btRot.y());
    irrRot.Z = irr::core::radToDeg(btRot.z());

    node->setPosition(irrPos);
    node->setRotation(irrRot);
}

int32_t body::bodyStatus::toMask()const{
    int32_t res = 0;

    if(walk_forward==1)
        res |= BM_WALK_F;
    else if (walk_forward==-1) {
        res |= BM_WALK_B;
    }

    if(walk_leftOrRight==1){
        res |= BM_WALK_R;
    }else if (walk_leftOrRight==-1) {
        res |= BM_WALK_L;
    }

    res |= bodyPosture;
    res |= handPosture;
    if(useLeft)
        res |= BM_HAND_LEFT;
    if(useRight)
        res |= BM_HAND_RIGHT;
    if(shotLeft)
        res |= BM_ACT_SHOT_L;
    if(shotRight)
        res |= BM_ACT_SHOT_R;
    if(throwing)
        res |= BM_ACT_THROW;
    if(chop)
        res |= BM_ACT_CHOP;
    return res;
}
void body::bodyStatus::loadMask(int32_t m){
    if(m & BM_HAND_LEFT)
        useLeft = true;
    else
        useLeft = false;

    if(m & BM_HAND_RIGHT)
        useRight = true;
    else
        useRight = false;

    handPosture = (handPosture_t)(m & BS_HAND_LIFT);
    bodyPosture = (bodyPosture_t)(m & (BS_BODY_SQUAT | BS_BODY_SIT | BS_BODY_RIDE));

    if(m & BM_WALK_F){
        walk_forward = 1;
    }else if (m & BM_WALK_B) {
        walk_forward = -1;
    }else{
        walk_forward = 0;
    }

    if(m & BM_WALK_L){
        walk_leftOrRight = -1;
    }else if (m & BM_WALK_R) {
        walk_leftOrRight = 1;
    }else{
        walk_leftOrRight = 0;
    }

    if(m & BM_ACT_SHOT_L)
        shotLeft = true;
    else
        shotLeft = false;

    if(m & BM_ACT_SHOT_R)
        shotRight = true;
    else
        shotRight = false;

    if(m & BM_ACT_CHOP)
        chop = true;
    else
        chop = false;

    if(m & BM_ACT_THROW)
        throwing = true;
    else
        throwing = false;
}
body::bodyStatus::bodyStatus(){
    bodyPosture = BS_BODY_STAND;
    handPosture = BS_HAND_NONE;
    useLeft  = false;
    useRight = false;
    walk_leftOrRight = 0;
    walk_forward     = 0;
    shotLeft = false;
    shotRight= false;
    throwing = false;
    chop     = false;
}
body::bodyStatus::bodyStatus(const bodyStatus & i){
    handPosture = i.handPosture;
    bodyPosture = i.bodyPosture;
    useLeft     = i.useLeft;
    useRight    = i.useRight;
    walk_leftOrRight = i.walk_leftOrRight;
    walk_forward     = i.walk_forward;
    shotLeft    = i.shotLeft;
    shotRight   = i.shotRight;
    throwing    = i.throwing;
    chop        = i.chop;
}
body::bodyStatus::bodyStatus(int32_t m){
    loadMask(m);
}
const body::bodyStatus & body::bodyStatus::operator=(const bodyStatus & i){
    handPosture = i.handPosture;
    bodyPosture = i.bodyPosture;
    useLeft     = i.useLeft;
    useRight    = i.useRight;
    walk_leftOrRight = i.walk_leftOrRight;
    walk_forward     = i.walk_forward;
    shotLeft    = i.shotLeft;
    shotRight   = i.shotRight;
    throwing    = i.throwing;
    chop        = i.chop;
    return *this;
}
const body::bodyStatus & body::bodyStatus::operator=(int32_t m){
    loadMask(m);
    return *this;
}

void body::releaseBody(bodyItem * b){
    if(b->coll_rigidBody)
        delete b->coll_rigidBody;
    if(b->coll_bodyState)
        delete b->coll_bodyState;
    b->m_character.removeFromWorld();
    b->node->remove();

    //清除跟随
    for(auto it:b->followers){
        it->follow = NULL;
    }
    b->followers.clear();
    if(b->follow){
        b->follow->followers.erase(b);
        b->follow=NULL;
    }

    if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的
        removeCharacterChunk(b->uuid);
    }

    if(!mainControl.empty() && mainControl==b->uuid){//是视角物体
        mainControl.clear();
        mainControlBody=NULL;
    }
    delete b;
}

body::bodyItem * body::seekBody(const std::string & u){
    auto it = bodies.find(u);
    if(it==bodies.end())
        return NULL;
    else
        return it->second;
}
body::bodyItem * body::seekMyBody(const std::string & u){
    auto it = myBodies.find(u);
    if(it==myBodies.end())
        return NULL;
    else
        return it->second;
}

body::commond::commond():uuid(),data_str(),data_vec(){
}

body::commond::commond(const body::commond & t){
    uuid        = t.uuid;
    cmd         = t.cmd;
    data_int    = t.data_int;
    data_str    = t.data_str;
    data_vec    = t.data_vec;
}
void body::selectBodyByScreenPoint(const irr::core::vector2d<s32> & sp, int range){
    selectedBodies.clear();
    for(auto it:myBodies){
        bodyItem * bd = it.second;
        auto p = bd->screenPosition;
        if(bd==mainControlBody)
            continue;
        if(selectAllBodies || ((p.X>0 && p.Y>0 && p.X<width && p.Y<height) && (p-sp).getLengthSQ()<range*range)){
            selectedBodies.insert(bd);
            //printf("select body:%s\n",bd->uuid.c_str());
        }
    }
}

void body::selectBodyStart(){
    if(selecting)
        return;
    selectBodyStartTime = timer->getTime();
    selecting = true;
    selectAllBodies = false;
    selectBodyRange = 0;
}

void body::selectBodyEnd(){
    if(!selecting)
        return;
    selecting = false;
    selectBodyByScreenPoint(screenCenter,selectBodyRange);
}

void body::selectBodyUpdate(){
    if(!selecting)
        return;
    if((selectBodyRange+selectBodyRange)<height)
        selectBodyRange = (timer->getTime()-selectBodyStartTime)*0.1;
    else{
        selectAllBodies = true;
    }
}

bool body::bodyItem::JointCallback::getFrameData(f32 frame, scene::ISkinnedMesh::SJoint * joint, core::vector3df & position, s32 & positionHint, core::vector3df & scale, s32 & scaleHint, core::quaternion & rotation, s32 & rotationHint){
    return false;
}

void body::updateBagUI(){
    body_bag_resource->clear();
    body_bag_using->clear();
    body_bag_using->setVisible(false);
    int bagStartAt = bagPage*8;
    int index=0;
    int keyIndex=0;
    wchar_t buf[256];
    usingToolsTable.clear();
    if(mainControlBody){
        for(auto it:mainControlBody->tools){
            if(index>=bagStartAt && index<=(bagStartAt+8)){
                auto t = tools.find(it);
                if(t!=tools.end()){
                    if(keyIndex>9)
                        break;
                    usingToolsTable.push_back(it);
                    swprintf(buf,256,L"\nUUID:%s\nDP:%d\n",it.c_str(),t->second.dur);
                    ++keyIndex;
                    int id;
                    auto icit = bag_tool_icons_mapping.find(t->second.id);
                    if(icit==bag_tool_icons_mapping.end()){
                        id = body_bag_resource->addItem(buf);
                    }else{
                        id = body_bag_resource->addItem(buf,icit->second);
                    }
                    if(mainControlBody->usingTool==it){//正在使用
                        body_bag_resource->setSelected(id);
                    }
                    body_bag_resource->setItemOverrideColor(id,irr::video::SColor(255,255,255,255));
                }
            }
            ++index;
        }
        if(!mainControlBody->usingTool.empty()){
            int bullets = 0;

            auto cit = fire_costs.find(mainControlBody->fireId);
            if(cit!=fire_costs.end()){
                if(cit->second.cost_num!=0){//需要弹药
                    auto bgr = mainControlBody->resources.find(cit->second.cost_id);//定位资源
                    if(bgr!=mainControlBody->resources.end()){
                        bullets = bgr->second;
                    }
                }
            }

            auto t = tools.find(mainControlBody->usingTool);
            if(t!=tools.end()){

                swprintf(buf,256,L"\n%d/%d %d\n",t->second.pwr,bullets,t->second.dur);

                int usingId;

                auto icit = bag_tool_icons_mapping.find(t->second.id);
                if(icit==bag_tool_icons_mapping.end()){
                    usingId = body_bag_using->addItem(buf);
                }else{
                    usingId = body_bag_using->addItem(buf,icit->second);
                }
                body_bag_using->setItemOverrideColor(usingId,irr::video::SColor(128,105,218,213));
                body_bag_using->setVisible(true);
            }
        }
        std::vector<int> rms;
        for(auto it:mainControlBody->resources){
            if(it.second<=0){
                rms.push_back(it.first);
                continue;
            }
            if(index>=bagStartAt && index<=(bagStartAt+8)){
                swprintf(buf,256,L"\n×%d\n",it.second);
                int id;
                auto icit = bag_res_icons_mapping.find(it.first);
                if(icit==bag_res_icons_mapping.end()){
                    id = body_bag_resource->addItem(buf);
                }else{
                    id = body_bag_resource->addItem(buf,icit->second);
                }
                body_bag_resource->setItemOverrideColor(id,irr::video::SColor(255,255,255,255));
            }
            ++index;
        }
        for(auto it:rms){
            mainControlBody->resources.erase(it);
        }
        if(body_bag_page)
            body_bag_page->remove();
        swprintf(buf,256,L"%d\n",bagPage+1);
        body_bag_page = gui->addStaticText(buf,irr::core::rect<irr::s32>(380,256,400,272),false,true,body_bag_resource);
        body_bag_page->setOverrideColor(irr::video::SColor(255,255,255,255));
        body_bag_page->setOverrideFont(font);
    }
    needUpdateUI = false;
}

void body::useTool(int id){
    if(mainControlBody==NULL)
        return;
    try{
        cmd_useBagTool(mainControl.c_str(),usingToolsTable.at(id).c_str());
    }catch(...){
        cmd_useBagTool(mainControl.c_str(),"");
    }
}

void body::bag_selectLast(){
    --bag_selectId;
    if(bag_selectId<-1)
        bag_selectId = -1;
    useTool(bag_selectId);
}

void body::bag_selectNext(){
    ++bag_selectId;
    auto max = usingToolsTable.size();
    if(bag_selectId>max){
        bag_selectId = max;
    }
    useTool(bag_selectId);
}

void body::tool::loadStr(const char * str){
    auto json = cJSON_Parse(str);
    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"id")==0){
                    id = c->valueint;
                }else if(strcmp(c->string,"durability")==0){
                    dur = c->valueint;
                }else if(strcmp(c->string,"power")==0){
                    pwr = c->valueint;
                }
            }
            c=c->next;
        }

        cJSON_Delete(json);
    }
}

}
