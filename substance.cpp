#include "substance.h"
namespace smoothly{

typedef mempool<substance::subs> subsPool;

void substance::subs::setMotion(const irr::core::vector3df & p,const irr::core::vector3df & r){
    btTransform transform;
    
    node->setPosition(p);
    node->setRotation(r);
    node->updateAbsolutePosition();//update mtx
    
    transform.setFromOpenGLMatrix(node->getAbsoluteTransformation().pointer());//set transform
    
    bodyState->setWorldTransform(transform);//apply
}

void substance::pushSubsCommond(const subsCommond & cmd){
    commondLocker.lock();
    cmdQueue.push(cmd);
    commondLocker.unlock();
}
void substance::parseAllCommond(){
    commondLocker.lock();
    while(!cmdQueue.empty()){
        parseCommond(cmdQueue.front());
        cmdQueue.pop();
    }
    commondLocker.unlock();
}

void substance::parseCommond(const subsCommond & cmd){
    auto p=seekSubs(cmd.uuid);
    if(p){
        switch(cmd.method){
            case subsCommond::JUMP:
                p->jump(cmd.vec);
            break;
            case subsCommond::WALK:
                p->walkingForward      =cmd.walkForward;
                p->walkingLeftOrRight  =cmd.walkLeftOrRight;
            break;
            case subsCommond::WALK_STOP:
                p->walkingForward=0;
                p->walkingLeftOrRight=0;
            break;
            case subsCommond::FLY:
                p->flyUpdate(cmd.flying , cmd.lifting);
            break;
            case subsCommond::FLY_STOP:
                p->flyUpdate(false,false);
            break;
            case subsCommond::DIRECT:
                p->setDirection(cmd.vec);
            break;
        }
    }
}
void substance::subs::moveUpdate(){
    if(walkingForward!=0 || walkingLeftOrRight!=0){
        walk(walkingForward,walkingLeftOrRight,walkSpeed*parent->deltaTime);
    }
}
void substance::subs::flyUpdate(bool flying,bool lifting){
    if(flying){
        if(lifting){
            fly(liftForce,pushForce);
        }else{
            fly(0,pushForce);
        }
    }else
        rigidBody->clearForces();
}

void substance::subs::fly(float lift,float push){
    irr::core::vector3df direct=getDirection();
    direct.normalize();
    direct*=push;
    fly(lift,direct);
}
void substance::subs::fly(float lift,const irr::core::vector3df & push){
    btVector3 F(push.X , push.Y+lift , push.Z);
    rigidBody->applyForce(F , btVector3(0,0,0));
}
void substance::subs::jump(const irr::core::vector3df & d){
    rigidBody->applyImpulse(btVector3(d.X , d.Y , d.Z) , btVector3(0,0,0));
}

static void rotate2d(irr::core::vector2df & v,float a){
    auto cosa=cos(a);
    auto sina=sin(a);
    auto x=v.X*cosa - v.Y*sina;
    auto y=v.X*sina + v.Y*cosa;
    v.X=x;
    v.Y=y;
    //v.normalize();
}
void substance::subs::setDirection(const irr::core::vector3df & d){
    irr::core::vector3df rotate=d.getHorizontalAngle();
    setRotation(rotate);
}
irr::core::vector3df substance::subs::getDirection(){
    btTransform transform;
    btVector3 btRot;
    irr::core::vector3df rotate;
    bodyState->getWorldTransform(transform);
    btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    rotate.X = irr::core::radToDeg(btRot.x());
    rotate.Y = irr::core::radToDeg(btRot.y());
    rotate.Z = irr::core::radToDeg(btRot.z());
    return rotate.rotationToDirection();
}
void substance::subs::walk(int forward,int leftOrRight/*-1 left,1 right*/,float length){
    if(!contacted)
        return;
    irr::core::vector2df delta(0,0);
    irr::core::vector3df direct=getDirection();
    irr::core::vector2df direct2d(direct.X , direct.Z);
    irr::core::vector2df p2d;
    direct2d.normalize();
    
    if(forward==1){
        delta+=direct2d;
    }else
    if(forward==-1){
        delta-=direct2d;
    }
    
    if(leftOrRight==1){
        p2d=direct2d;
        rotate2d(p2d, -3.1415926/2);
        p2d.normalize();
        delta+=p2d;
    }else
    if(leftOrRight==-1){
        p2d=direct2d;
        rotate2d(p2d, 3.1415926/2);
        p2d.normalize();
        delta+=p2d;
    }
    delta.normalize();
    delta*=length;
    walk(delta);
}

void substance::subs::walk(const irr::core::vector2df & d){
    walk(irr::core::vector3df(d.X,0,d.Y));
}
void substance::subs::walk(const irr::core::vector3df & d){
    if(contacted)
        move(d);
}
void substance::subs::move(const irr::core::vector3df & delta){
    btTransform transform;
    bodyState->getWorldTransform(transform);
    auto p=transform.getOrigin();
    irr::core::vector3df np(p.getX(),p.getY(),p.getZ());
    np+=delta;
    setPosition(np);
}
void substance::subs::teleport(const irr::core::vector3df & p){
    btTransform transform=rigidBody->getWorldTransform();
    transform.setOrigin(btVector3(p.X, p.Y, p.Z));
    rigidBody->setWorldTransform(transform);
}
void substance::subs::setPosition(const irr::core::vector3df & p){
    node->setPosition(p);
    
    btTransform transform;
    bodyState->getWorldTransform(transform);
    transform.setOrigin(btVector3(p.X, p.Y, p.Z));
    bodyState->setWorldTransform(transform);
    
    wake=true;
}

void substance::subs::setRotation(const irr::core::vector3df & r){
    node->setRotation(r);
    
    btQuaternion rq;
    euler2quaternion(r,rq);
    
    btTransform transform;
    bodyState->getWorldTransform(transform);
    transform.setRotation(rq);
    bodyState->setWorldTransform(transform);
    
    wake=true;
}

void substance::subs::updateByWorld(){
    if(!inWorld)
        return;
        
    btTransform transform;
    
    bodyState->getWorldTransform(transform);
    
    btVector3 btPos;
    btVector3 btRot;
    irr::core::vector3df irrPos;
    irr::core::vector3df irrRot;
    
    btPos = transform.getOrigin();
    irrPos.set(btPos.x(), btPos.y(), btPos.z());
    
    btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    irrRot.X = irr::core::radToDeg(btRot.x());
    irrRot.Y = irr::core::radToDeg(btRot.y());
    irrRot.Z = irr::core::radToDeg(btRot.z());
    
    node->setPosition(irrPos);
    node->setRotation(irrRot);
    
    x=irrPos.X/32;
    y=irrPos.Z/32;
}
void substance::subsUpdate(){
    clearDiedSubs();
    removeApply();
    
    {//update
        for(auto it:subses){
            //update collision
            it.second->update();//update scene
            it.second->moveUpdate();
        }
        parseAllCommond();
        for(auto it:subses){
            it.second->contacted=false;
        }
    }
    {//set camera
        if(!mainControlUUID.empty()){
            auto it=subses.find(mainControlUUID);
            if(it!=subses.end()){
                mainControlPosition=it->second->node->getPosition();
                
                int cx=mainControlPosition.X/32;
                int cy=mainControlPosition.Z/32;
                //clear out of range
                for(auto sit:subses){
                    auto p=sit.second->node->getPosition();
                    
                    int dx=p.X/32;
                    int dy=p.Z/32;
                    
                    if(fabs(cx-dx)>=14 || fabs(cy-dy)>=14){
                        removeLocalSubs(sit.second->uuid);
                    }
                }
                removeApply();
                
            }
        }
    }
}
void substance::removeSubs(substance::subs * p){
    if(p){
        if(p->type==mods::SUBS_LASTING){
            requestRemoveSubs(p->uuid);
        }else
        if(p->type==mods::SUBS_BRIEF){
            removeLocalSubs(p->uuid);
        }
    }
}
void substance::subs::update(){
    if(parent){
        updateByWorld();
        if(type==mods::SUBS_LASTING && (!rigidBody->wantsSleeping() || wake)){
            //upload to server
            parent->uploadBodyStatus(
                uuid,
                status,
                node->getPosition(),
                node->getRotation(),
                getLinearVelocity(),
                getAngularVelocity()
            );
        }
        wake=false;
    }
}
void substance::removeApply(){
    for(auto it:subsRMT){
        auto p=seekSubs(it);
        if(p){
            removeOwned(p->uuid,p->owner);
            p->release();
            delSubs(p);
        }
    }
    subsRMT.clear();
}

substance::subs * substance::addLastingSubs(//添加持久物体
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    //向服务器发送创建命令，服务器再向自己发送
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_LASTING)
        requestCreateLastingSubs(id,posi,rota,impulse,rel_pos);
}

substance::subs * substance::addBriefSubs(//添加非持久物体
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    //在本地创建，再发送到服务器。服务器不会再次对自己发送创建命令(smootly::watch::boardcastSubsCreate)
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_BRIEF){
        
        auto sp=createSubs();
        sp->setRandUUID();
        sp->owner=myUUID;
        sp->hp=p->hp;
        sp->status=0;
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_BRIEF;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
        
        sp->setAsBrief(p->life);
        
        requestCreateBriefSubs(id,posi,rota,impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（持久），由服务器调用
    const std::string & uuid ,
    const std::string & owner ,
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3 & impulse,
    const btVector3 & rel_pos
){
    auto op=seekSubs(uuid);
    if(op){
        return;
    }
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_LASTING){
        auto sp=createSubs();
        sp->uuid=uuid;
        sp->owner=owner;
        setOwner(uuid,owner);
        sp->hp=p->hp;
        sp->status=0;
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_LASTING;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（非持久）
    long id , 
    const std::string & owner ,
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_BRIEF){
        
        auto sp=createSubs();
        sp->setRandUUID();
        sp->owner=owner;
        sp->hp=p->hp;
        sp->status=0;
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_BRIEF;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
        sp->setAsBrief(p->life);
    }
}
void substance::updateSubs(//更新物体状态，由服务器调用
    long id,
    const std::string & uuid , 
    const std::string & owner ,
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& lin_vel ,
    const btVector3& ang_vel ,
    int hp,int status,
    const std::string & conf
){
    auto sp=seekSubs(uuid);
    if(sp){
        sp->setMotion(posi,rota);
        sp->owner=owner;
        //setOwner(uuid,owner);
        sp->hp=hp;
        sp->status=status;
        sp->rigidBody->setLinearVelocity(lin_vel);
        sp->rigidBody->setAngularVelocity(ang_vel);
    }else{
        auto p=seekSubsConf(id);
        if(p && p->type==mods::SUBS_LASTING){
            auto sp=createSubs();
            sp->uuid=uuid;
            sp->owner=owner;
            setOwner(uuid,owner);
            sp->hp=hp;
            sp->status=status;
            sp->init(p,posi,rota);
            sp->type=mods::SUBS_LASTING;
            sp->rigidBody->setLinearVelocity(lin_vel);
            sp->rigidBody->setAngularVelocity(ang_vel);
        }
    }
}
void substance::subs::init(mods::subsConf * conf,const irr::core::vector3df & p,const irr::core::vector3df & r){
    if(uuid.empty() || parent==NULL)
        return;
    
    subsConf=conf;
    contacted=false;
    wake=true;
    id=conf->id;
    
    node=parent->scene->addMeshSceneNode(conf->mesh);
    node->setPosition(p);
    node->setRotation(r);
    node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    node->updateAbsolutePosition();
    
    if(conf->texture){
        node->setMaterialTexture( 0 , conf->texture);
        if(conf->useAlpha){
            node->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    info.type=BODY_SUBSTANCE;
    info.ptr=this;
    
    bodyState=setMotionState(node->getAbsoluteTransformation().pointer());
    rigidBody=createBody(conf->bodyShape , bodyState , conf->mass , conf->localInertia);
    
    rigidBody->setUserPointer(&info);
    
    rigidBody->setFriction(conf->friction);
    rigidBody->setRestitution(conf->restitution);
    
    x=p.X/32;
    y=p.Z/32;
    this->addIntoWorld();
    
    parent->subses[uuid]=this;
}

void substance::lockChunk(int x,int y){
    chunkLocker[ipair(x,y)];
}

void substance::unlockChunk(int x,int y){
    auto it=chunkLocker.find(ipair(x,y));
    
    if(it!=chunkLocker.end()){
        
        for(auto s:it->second){
            dynamicsWorld->addRigidBody(s->rigidBody);
            s->inWorld=true;
        }
        chunkLocker.erase(it);
        
    }
}

bool substance::subs::addIntoWorld(){
    auto it=parent->chunkLocker.find(ipair(x,y));
    
    if(it==parent->chunkLocker.end()){
        //chunnk has not been locked
        parent->dynamicsWorld->addRigidBody(rigidBody);
        inWorld=true;
        return true;
    }else{
        //chunk has been locked
        it->second.insert(this);
        inWorld=false;
        return false;
    }
}
void substance::onGenChunk(terrain::chunk * c){
    buildings::onGenChunk(c);
    
    int cx=mainControlPosition.X/32;
    int cy=mainControlPosition.Z/32;
    
    if(fabs(cx-(c->x))>=14 || fabs(cy-(c->y))>=14)
        return;
    
    lockChunk(c->x,c->y);
    requestDownloadSubstanceChunk(c->x,c->y);
}
void substance::onFreeChunk(terrain::chunk * c){
    buildings::onFreeChunk(c);
}
void substance::subs::setRandUUID(){
    getUUID(uuid);
}

void substance::subs::release(){
    if(uuid.empty() || parent==NULL)
        return;
    
    node->remove();
    
    parent->subses.erase(uuid);
    
    if(inWorld)
        parent->dynamicsWorld->removeRigidBody(rigidBody);
    else{
        //chunk has been save in x,y
        auto it=parent->chunkLocker.find(ipair(x,y));
    
        if(it!=parent->chunkLocker.end()){
            it->second.erase(this);
        }
        
    }
    
    delete rigidBody;
    delete bodyState;
    
    rigidBody=NULL;
    bodyState=NULL;
    node=NULL;
}

void substance::subs::setAsBrief(int life){
    if(uuid.empty() || parent==NULL)
        return;
    
    parent->briefSubs.push_back(briefTime(this->uuid , parent->timer->getTime(),life));
    type==mods::SUBS_BRIEF;
}
struct isSubsExpire{
    substance * self;
    bool operator() (const substance::briefTime & value){
        if(fabs(self->timer->getTime()-value.createTime)>value.life){
            //expire
            self->removeLocalSubs(value.uuid);
            return true;
        }else
            return false;
    }
};
void substance::clearDiedSubs(){
    isSubsExpire s;
    s.self=this;
    briefSubs.remove_if(s);
}
substance::subs * substance::createSubs(){
    auto p=((subsPool*)sPool)->get();
    p->parent=this;
    p->wake=true;
    return p;
}
void substance::delSubs(substance::subs * p){
    ((subsPool*)sPool)->del(p);
}
void substance::subsPoolInit(){
    sPool = new subsPool;
}
void substance::subsPoolDestroy(){
    delete (subsPool*)sPool;
}

void substance::onCollision(bodyInfo * A,bodyInfo * B,const btManifoldPoint & point){
    //碰撞
    float impulse=point.getAppliedImpulse();//冲量
    {
        if(A->type==BODY_SUBSTANCE){
        
            auto ptrA=(subs*)A->ptr;
            auto scA=ptrA->subsConf;
            
            ptrA->contacted=true;
            
            if(B->type==BODY_TERRAIN){
                if(scA->haveHitTerrainCallback && (!(scA->hitTerrainCallbackOnlyForMe) || (ptrA->owner==myUUID))){
                    scA->onHitTerrainCallback(m->L , this , ptrA->uuid , ptrA->owner , impulse);
                }
            }else
            if(B->type==BODY_TERRAIN_ITEM){
                if(scA->haveHitTerrainItemCallback && (!(scA->hitTerrainItemCallbackOnlyForMe) || (ptrA->owner==myUUID))){
                    
                    auto i=(terrain::item*)B->ptr;
                    scA->onHitTerrainItemCallback(
                        m->L , 
                        this , 
                        ptrA->uuid , 
                        ptrA->owner ,
                        mapid(i->inChunk->x , i->inChunk->y , i->id , i->mapId) , 
                        impulse
                    );
                    
                }
            }else
            if(B->type==BODY_BUILDING){
                if(scA->haveHitBuildingCallback && (!(scA->hitBuildingCallbackOnlyForMe) || (ptrA->owner==myUUID))){
                    
                    auto b=(remoteGraph::item*)B->ptr;
                    scA->onHitBuildingCallback(m->L , this , ptrA->uuid , ptrA->owner , b->uuid , impulse);
                    
                }
            }else
            if(B->type==BODY_SUBSTANCE){
                if(scA->haveHitSubsCallback && (!(scA->hitSubsCallbackOnlyForMe) || (ptrA->owner==myUUID))){
                    
                    auto p=(subs*)B->ptr;
                    scA->onHitSubs(m->L , this , ptrA->uuid , ptrA->owner , p->uuid , p->owner , impulse);
                    
                }
            }
        }
    }
    {
        if(B->type==BODY_SUBSTANCE){
            
            auto ptrB=(subs*)B->ptr;
            auto scB=ptrB->subsConf;
            
            ptrB->contacted=true;
            
            if(A->type==BODY_TERRAIN){
                if(scB->haveHitTerrainCallback && (!(scB->hitTerrainCallbackOnlyForMe) || (ptrB->owner==myUUID))){
                    scB->onHitTerrainCallback(m->L , this , ptrB->uuid , ptrB->owner , impulse);
                }
            }else
            if(A->type==BODY_TERRAIN_ITEM){
                if(scB->haveHitTerrainItemCallback && (!(scB->hitTerrainItemCallbackOnlyForMe) || (ptrB->owner==myUUID))){
                    
                    auto i=(terrain::item*)A->ptr;
                    scB->onHitTerrainItemCallback(
                        m->L , 
                        this , 
                        ptrB->uuid , 
                        ptrB->owner ,
                        mapid(i->inChunk->x , i->inChunk->y , i->id , i->mapId) , 
                        impulse
                    );
                    
                }
            }else
            if(A->type==BODY_BUILDING){
                if(scB->haveHitBuildingCallback && (!(scB->hitBuildingCallbackOnlyForMe) || (ptrB->owner==myUUID))){
                    
                    auto b=(remoteGraph::item*)A->ptr;
                    scB->onHitBuildingCallback(m->L , this , ptrB->uuid , ptrB->owner , b->uuid , impulse);
                    
                }
            }else
            if(A->type==BODY_SUBSTANCE){
                if(scB->haveHitSubsCallback && (!(scB->hitSubsCallbackOnlyForMe) || (ptrB->owner==myUUID))){
                    
                    auto p=(subs*)A->ptr;
                    scB->onHitSubs(m->L , this , ptrB->uuid , ptrB->owner , p->uuid , p->owner , impulse);
                    
                }
            }
        }
    }
}

}//namespace smoothly
