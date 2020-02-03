#include "body.h"

namespace smoothly{

void body::setBodyPosition(const std::string & uuid , const vec3 & posi){
    int cx = posi.X/32;
    int cy = posi.Z/32;

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
        myBodies.erase(it);
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
    irr::core::vector3df irrPos;
    irr::core::vector3df irrRot;

    btPos = transform.getOrigin();
    irrPos.set(btPos.x(), btPos.y(), btPos.z());

    const btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    irrRot.X = irr::core::radToDeg(btRot.x());
    irrRot.Y = irr::core::radToDeg(btRot.y());
    irrRot.Z = irr::core::radToDeg(btRot.z());

    if(owner==parent->myUUID){//拥有的物体
        //尝试上传
        if(lastPosition!=irrPos){
            node->setPosition(irrPos);
            lastPosition = irrPos;
            parent->cmd_setPosition(uuid,irrPos.X,irrPos.Y,irrPos.Z);
        }
        if(lastRotation!=irrRot){
            node->setRotation(irrRot);
            lastRotation = irrRot;
            parent->cmd_setRotation(uuid,irrPos.X,irrPos.Y,irrPos.Z);
        }
        if(lastLookAt!=lookAt){
            lastLookAt = lookAt;
            parent->cmd_setLookAt(uuid,irrPos.X,irrPos.Y,irrPos.Z);
        }
    }else{
        node->setRotation(irrRot);
        node->setPosition(irrPos);
    }

    //setPositionByTransform(node,t);

    //如果是视角物体，更新摄像机
    if(uuid==parent->mainControl){
        parent->camera->setPosition(irrPos);
    }
}

void body::bodyItem::doAnimation(int speed, int start, int end, bool loop){
    if(node==NULL)
        return;
    node->setAnimationSpeed(speed);
    node->setLoopMode(loop);
    node->setFrameLoop(start,end);
}

void body::bodyItem::doFire(){
    auto ntm = parent->timer->getRealTime();
    if(fireDelta>0 && ntm>fireDelta+lastFireTime){
        auto wnode = wearing.find(firingWearingId);
        if(wnode!=wearing.end()){
            parent->fireTo(uuid,fireId,wnode->second->getPosition(),lookAt);
        }
        lastFireTime = ntm;
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
    auto it = parent->bodyConfig.find(id);
    if(it==parent->bodyConfig.end())
        return;
    if(it->second->aniCallback.empty())
        return;
    lua_getglobal(parent->L,it->second->aniCallback.c_str());
    if(lua_isfunction(parent->L,-1)){

        lua_pushboolean(parent->L, status.walk_forward);
        lua_pushboolean(parent->L, status.walk_leftOrRight);
        lua_pushboolean(parent->L, status.useLeft);
        lua_pushboolean(parent->L, status.useRight);

        if(status.bodyPosture==bodyStatus::BS_BODY_STAND){
            lua_pushstring(parent->L, "stand");
        }else
        if(status.bodyPosture==bodyStatus::BS_BODY_SQUAT){
            lua_pushstring(parent->L, "squat");
        }else
        if(status.bodyPosture==bodyStatus::BS_BODY_SIT){
            lua_pushstring(parent->L, "sit");
        }else
        if(status.bodyPosture==bodyStatus::BS_BODY_RIDE){
            lua_pushstring(parent->L, "ride");
        }else
        if(status.bodyPosture==bodyStatus::BS_BODY_LIE){
            lua_pushstring(parent->L, "lie");
        }else
        if(status.bodyPosture==bodyStatus::BS_BODY_LIEP){
            lua_pushstring(parent->L, "liep");
        }else{
            lua_pushstring(parent->L, "");
        }

        if(status.handPosture==bodyStatus::BS_HAND_NONE){
            lua_pushstring(parent->L, "none");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_AIM){
            lua_pushstring(parent->L, "aim");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_THROW){
            lua_pushstring(parent->L, "throw");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_BUILD){
            lua_pushstring(parent->L, "build");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_BUILDP){
            lua_pushstring(parent->L, "buildp");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_OPERATE){
            lua_pushstring(parent->L, "operate");
        }else
        if(status.handPosture==bodyStatus::BS_HAND_LIFT){
            lua_pushstring(parent->L, "lift");
        }else{
            lua_pushstring(parent->L, "");
        }

        lua_pushboolean(parent->L,finish);

        // do the call (2 arguments, 1 result)
        if (lua_pcall(parent->L, 7, 4, 0) != 0)
             printf("error running function : %s \n",lua_tostring(parent->L, -1));
        else{
            if(lua_isboolean(parent->L,-1) && lua_isinteger(parent->L,-2) && lua_isinteger(parent->L,-3) && lua_isinteger(parent->L,-4)){
                doAnimation(
                            lua_tointeger(parent->L,-4),
                            lua_tointeger(parent->L,-3),
                            lua_tointeger(parent->L,-2),
                            lua_toboolean(parent->L,-1));
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

    int cx = posi.X/32;
    int cy = posi.Z/32;
    if(owner==myUUID && (!myUUID.empty())){//是自己拥有的
        setCharacterChunk(uuid,cx,cy);
    }else{
        if(!chunkLoaded(cx,cy)){//在chunk外，删除物体
            return;
        }
    }

    bodyItem * p = new bodyItem(c->width , c->height , btVector3(posi.X,posi.Y,posi.Z) , c->walkInSky , c->jumpInSky);
    p->m_character.world = dynamicsWorld;
    p->m_character.addIntoWorld();
    p->m_character.setRotation(r);
    p->m_character.setUserPointer(&(p->info));

    p->info.ptr  = p;
    p->info.type = BODY_BODY;

    p->lookAt = l;

    p->lastLookAt   = l;
    p->lastPosition = posi;
    p->lastRotation = r;

    p->uuid   = uuid;
    p->owner  = owner;
    p->id     = id;
    p->hp     = hp;
    p->status = sta_mask;
    p->status_mask = sta_mask;
    p->config = c;
    p->node   = scene->addAnimatedMeshSceneNode(c->mesh);
    p->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    p->node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
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

    p->updateFromWorld();
    p->updateStatus();

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
    }
}
void body::removeWearing(bodyItem * n, int wearing){
    auto it = n->wearing.find(wearing);
    if(it!=n->wearing.end())
        return;
    it->second->remove();
    n->wearing.erase(it);
    auto nit = wearingToBullet.find(wearing);
    if(nit!=wearingToBullet.end()){
        n->firingWearingId  = 0;
        n->fireId           = 0;
        n->fireDelta        = 0;
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
body::body(){
    mainControlBody = NULL;
    loadBodyConfig();
    loadWearingConfig();
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "../script/body.lua");
}

body::~body(){
    lua_close(L);
    releaseBodyConfig();
    releaseWearingConfig();
}

void body::setPositionByTransform(irr::scene::ISceneNode * node, const btTransform & transform){
    btVector3 btPos;
    btVector3 btRot;
    irr::core::vector3df irrPos;
    irr::core::vector3df irrRot;

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
    }

    if(m & BM_WALK_L){
        walk_leftOrRight = -1;
    }else if (m & BM_WALK_R) {
        walk_leftOrRight = 1;
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
    b->m_character.removeFromWorld();
    b->node->remove();
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

}
