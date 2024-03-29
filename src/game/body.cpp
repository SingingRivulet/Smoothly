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

    if(node->isVisible() || this==parent->mainControlBody){
        //处理动画
        node->animateJoints(true,&animationBlend);
        if(!ik_effectors.empty() && config && config->ik_solver){//如果需要使用ik
            uploadIK();//上传数据至ik
            solveIK();//更新ik
            updateIK();//修正骨骼
        }
    }

    //更新光影
    if(this->shadow){
        this->shadow->setPosition(irrPos);
        this->shadow->setRotation(irrRot);
        if(node->isVisible() || this==parent->mainControlBody){
            u32 num = node->getJointCount();
            if(num == this->shadow->getJointCount()){
                for(u32 i=0;i<num;++i){
                    auto jt1 = this->shadow->getJointNode(i);
                    auto jt2 = node->getJointNode(i);
                    if(jt1 && jt2){
                        jt1->setPosition(jt2->getPosition());
                        jt1->setRotation(jt2->getRotation());
                    }
                }
            }
        }
    }

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
    if(audio){
        if(audio->isPlaying())
            audio->setPosition(irrPos);
        else{
            audio->drop();
            audio = NULL;
        }
    }
    int pitchAngle = getPitchAngle();
    if(pitchAngle!=lastPitchAngle){
        needUpdateStatus = true;
        lastPitchAngle   = pitchAngle;
    }

    if(needUpdateStatus)
        updateStatus();
    needUpdateStatus = false;

    //setPositionByTransform(node,t);

    //如果是视角物体，更新摄像机
    if(uuid==parent->mainControl){
        parent->camera->setPosition(irrPos+vec3(0,config->deltaY,0));
    }

    coll_rigidBody->getMotionState()->setWorldTransform(transform);

    irrPos.Y = 1;
    if(minimap_element){
        minimap_element->setPosition(irrPos);
    }
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
    resources.clear();
    tools.clear();
    usingTool.clear();
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

void body::bodyItem::clearFollowers(){
    for(auto it:followers){
        it->follow = NULL;
        it->setFollow(follow);
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

void body::bodyItem::make(int id){
    auto p = node->getPosition();
    parent->cmd_make(id,p.X,p.Y,p.Z,uuid.c_str());
}

void body::bodyItem::moveToEndOfFollow(){
    if(!followers.empty()){
        auto it = followers.begin();
        body::bodyItem * p = *it;
        auto tp = p;
        while(p){
            if(p==this){
                return;//构成环形
            }else if(p->followers.empty()){//插在此位置
                if(follow==NULL){
                    tp->follow = NULL;  //子节点成了队首
                    followers.erase(it);//断开链表
                    follow = p;         //跟随p
                    p->followers.insert(this);
                }else{
                    clearFollowers();
                    setFollow(p);
                }
                return;
            }else{//指针下移
                p = *(p->followers.begin());
            }
        }
    }
}

void body::bodyItem::useToolById(int id){
    for(auto it:tools){
        auto tit = parent->tools.find(it);
        if(tit!=parent->tools.end()){
            tool & t = tit->second;
            if(t.id==id && t.dur>0){
                parent->cmd_useBagTool(uuid.c_str() , it.c_str());
                return;
            }
        }
    }
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

        lua_createtable(parent->L,0,3);//创建主数组

        lua_pushstring(parent->L,"status");
        lua_createtable(parent->L,0,7);
        {

            lua_pushstring(parent->L,"pitchAngle");
            lua_pushinteger(parent->L, lastPitchAngle);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"walk");
            lua_createtable(parent->L,0,2);
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

            lua_pushstring(parent->L,"onGround");
            lua_pushboolean(parent->L,m_character.onGround());
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

                lua_pushstring(parent->L, "audio");
                lua_gettable(parent->L, -2);
                if(lua_istable(parent->L,-1)){

                    lua_pushstring(parent->L, "stop");
                    lua_gettable(parent->L, -2);
                    if(lua_isboolean(parent->L,-1) && lua_toboolean(parent->L,-1)){
                        //停止
                        if(audio){
                            audio->drop();
                            audio = NULL;
                        }
                        lua_pop(parent->L,1);
                    }else{
                        lua_pop(parent->L,1);

                        int audioId = 0;
                        bool audioLoop = false;

                        lua_pushstring(parent->L, "id");
                        lua_gettable(parent->L, -2);
                        if(lua_isinteger(parent->L,-1)){
                            audioId = lua_tointeger(parent->L,-1);
                        }
                        lua_pop(parent->L,1);

                        lua_pushstring(parent->L, "loop");
                        lua_gettable(parent->L, -2);
                        if(lua_isboolean(parent->L,-1)){
                            audioLoop = lua_toboolean(parent->L,-1);
                        }
                        lua_pop(parent->L,1);

                        playAudio(audioId,audioLoop);
                    }

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

    p->needUpdateStatus = true;

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

    p->audio = NULL;
    p->node   = scene->addAnimatedMeshSceneNode(c->mesh);
    p->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    p->node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    p->node->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;

    //光影
    p->shadow = shadowSpace->addAnimatedMeshSceneNode(c->mesh);
    p->shadow->setMaterialType((irr::video::E_MATERIAL_TYPE)shadowMapShader);
    p->shadow->setJointMode(irr::scene::EJUOR_CONTROL);

    irr::scene::ISkinnedMesh* skinnedMesh = reinterpret_cast<irr::scene::ISkinnedMesh*>(p->node->getMesh());
    skinnedMesh->animationOverrideCallback = &p->jointCallback;

    p->setBlenderAsDefault();

    p->node->setJointMode(irr::scene::EJUOR_CONTROL);//设为控制模式，因为要混合

    p->node->setMaterialType(bodyShader);
    //p->node->setMaterialTexture( 1 , shadowMapTexture);

    if(c->texture){
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

    p->minimap_element = NULL;
    if(owner==myUUID && (!myUUID.empty())){
        p->minimap_element = mapScene->addBillboardSceneNode();
        p->minimap_element->setMaterialFlag(irr::video::EMF_LIGHTING, false );
        //p->minimap_element->setMaterialType((video::E_MATERIAL_TYPE)minimap_body_shader);
        p->minimap_element->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
        p->minimap_element->setMaterialTexture(0,texture_minimap_body);
    }

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
    if(owner==myUUID && (!myUUID.empty())){
        myBodies[uuid] = p;
    }

    if(uuid==mainControl){
        mainControlBody = p;
    }

    createBodyAI(p);
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
    loadBodyLuaAPI(L);
    if(luaL_dofile(L, "../script/body.lua")){
        printf("[body]%s \n",lua_tostring(L, -1));
    }

    loadFastUseTool();
    loadBodyConfig();
    loadWearingConfig();
    loadFireCost();
    selecting = false;
    selectBodyStartTime = 0;

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

    commond_flags_list = gui->addListBox(irr::core::rect<irr::s32>(cx-200,cy-300,cx+200,cy+300),0,-1,true);
    commond_flags_list->setItemHeight(32);
    commond_flags_icons = gui->addEmptySpriteBank("commond_flags_icons");
    commond_flags_list->setSpriteBank(commond_flags_icons);
    commond_flags_list->setVisible(false);
    loadCommonds();

    body_bag_using = gui->addListBox(irr::core::rect<irr::s32>(20,20,200,52),0,-1,false);
    body_bag_using->setSpriteBank(bag_icons);
    body_bag_using->setItemHeight(32);
    body_bag_using->setVisible(false);

    needUpdateUI = false;
    usingResource = false;
    bag_selectId = -1;

    texture_hp = driver->getTexture("../../res/icon/hp.png");
    texture_minimap_body = driver->getTexture("../../res/icon/texture_minimap_body.png");
    texture_attackTarget = driver->getTexture("../../res/icon/attackTarget.png");
    attackingTarget = false;

    bodyShader = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                "../shader/body.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                "../shader/body.ps.glsl", "main", irr::video::EPST_PS_1_1,&defaultCallback);
}

body::~body(){
    for(auto it:bodies){
        releaseBody(it.second);
    }
    lua_close(L);
    releaseBodyConfig();
    releaseWearingConfig();
    saveFastUseTool();
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
    if(b->audio)
        b->audio->drop();

    //清除跟随
    b->clearFollowers();
    if(b->follow){
        b->follow->followers.erase(b);
        b->follow=NULL;
    }

    b->clearIKEffector();

    if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的
        removeCharacterChunk(b->uuid);
    }

    if(!mainControl.empty() && mainControl==b->uuid){//是视角物体
        mainControl.clear();
        mainControlBody=NULL;
    }

    if(b->minimap_element){
        b->minimap_element->remove();
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
    auto tm = timer->getTime();
    if(tm-selectBodyStartTime < 300){
        selectAllBodies = true;
    }else{
        selectAllBodies = false;
    }
    selectBodyStartTime = tm;
    selecting = true;
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

void body::getChunkMission(){
    if(mainControlBody){
        auto p = mainControlBody->node->getPosition();
        int cx = floor(p.X/32);
        int cy = floor(p.Z/32);

        cmd_getChunkMission(cx , cy);

        bool cxl = ((cx*32+16) > p.X);
        bool cyl = ((cy*32+16) > p.Y);

        if(cxl){
            cmd_getChunkMission(cx-1 , cy);
            if(cyl){
                cmd_getChunkMission(cx   , cy-1);
                cmd_getChunkMission(cx-1 , cy-1);
            }else{
                cmd_getChunkMission(cx   , cy+1);
                cmd_getChunkMission(cx-1 , cy+1);
            }
        }else{
            cmd_getChunkMission(cx+1 , cy);
            if(cyl){
                cmd_getChunkMission(cx   , cy-1);
                cmd_getChunkMission(cx+1 , cy-1);
            }else{
                cmd_getChunkMission(cx   , cy+1);
                cmd_getChunkMission(cx+1 , cy+1);
            }
        }

        scanAnimate();
    }
}

void body::saveFastUseTool(){
    auto fp = fopen("fastUseTool.txt","w");
    if(fp){
        fprintf(fp,"%d %d %d %d",fastUseTool[0],fastUseTool[1],fastUseTool[2],fastUseTool[3]);
        fclose(fp);
    }
}
void body::loadFastUseTool(){
    fastUseTool[0] = -1;
    fastUseTool[1] = -1;
    fastUseTool[2] = -1;
    fastUseTool[3] = -1;
    auto fp = fopen("fastUseTool.txt","r");
    if(fp){
        fscanf(fp,"%d %d %d %d",&fastUseTool[0],&fastUseTool[1],&fastUseTool[2],&fastUseTool[3]);
        fclose(fp);
    }
}

bool body::bodyItem::JointCallback::getFrameData(f32 frame, scene::ISkinnedMesh::SJoint * joint, core::vector3df & position, s32 & positionHint, core::vector3df & scale, s32 & scaleHint, core::quaternion & rotation, s32 & rotationHint){
    return false;
}

}
