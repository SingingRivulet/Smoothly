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
}

void substance::subs::updateByWorld(){
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
}
void substance::subsUpdate(){
    clearDiedSubs();
    removeApply();
    for(auto it:subses){
        it.second->update();
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
                node->getPosition(),
                node->getRotation(),
                getLinearVelocity(),
                getAngularVelocity()
            );
        }
        wake=(!rigidBody->wantsSleeping());
    }
}
void substance::removeApply(){
    for(auto it:subsRMT){
        auto p=seekSubs(it);
        if(p){
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
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_BRIEF){
        
        auto sp=createSubs();
        sp->setRandUUID();
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_BRIEF;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
        
        sp->setAsBrief(p->life);
        
        requestCreateBriefSubs(id,posi,rota,impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（持久），由服务器调用
    const std::string & uuid ,
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    if(seekSubs(uuid))
        return;
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_LASTING){
        auto sp=createSubs();
        sp->uuid=uuid;
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_LASTING;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（非持久）
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_BRIEF){
        
        auto sp=createSubs();
        sp->setRandUUID();
        sp->init(p,posi,rota);
        sp->type=mods::SUBS_BRIEF;
        sp->rigidBody->applyImpulse(impulse,rel_pos);
        sp->setAsBrief(p->life);
    }
}
void substance::setSubs(//设置物体（持久），由服务器调用
    const std::string & uuid ,
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota,
    const btVector3& lin_vel ,
    const btVector3& ang_vel
){
    auto ptr=seekSubs(uuid);
    if(ptr){
        ptr->setMotion(posi,rota);
        ptr->rigidBody->setLinearVelocity(lin_vel);
        ptr->rigidBody->setAngularVelocity(ang_vel);
    }else{
        auto p=seekSubsConf(id);
        if(p && p->type==mods::SUBS_LASTING){
            auto sp=createSubs();
            sp->uuid=uuid;
            sp->init(p,posi,rota);
            sp->type=mods::SUBS_LASTING;
            sp->rigidBody->setLinearVelocity(lin_vel);
            sp->rigidBody->setAngularVelocity(ang_vel);
        }
    }
}
void substance::updateSubs(//更新物体状态，由服务器调用
    const std::string & uuid , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const btVector3& lin_vel ,
    const btVector3& ang_vel
){
    auto sp=seekSubs(uuid);
    if(sp){
        sp->setMotion(posi,rota);
        sp->rigidBody->setLinearVelocity(lin_vel);
        sp->rigidBody->setAngularVelocity(ang_vel);
    }
}
void substance::subs::init(mods::subsConf * conf,const irr::core::vector3df & p,const irr::core::vector3df & r){
    if(uuid.empty() || parent==NULL)
        return;
    
    id=conf->id;
    
    node=parent->scene->addMeshSceneNode(conf->mesh);
    node->setPosition(p);
    node->setRotation(r);
    node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    node->updateAbsolutePosition();
    
    info.type=BODY_SUBSTANCE;
    info.ptr=this;
    
    bodyState=setMotionState(node->getAbsoluteTransformation().pointer());
    rigidBody=createBody(conf->bodyShape , bodyState , conf->mass , conf->localInertia);
    
    rigidBody->setUserPointer(&info);
    
    parent->dynamicsWorld->addRigidBody(rigidBody);
    
    parent->subses[uuid]=this;
}
void substance::subs::setRandUUID(){
    getUUID(uuid);
}
void substance::subs::release(){
    if(uuid.empty() || parent==NULL)
        return;
    
    node->remove();
    
    parent->subses.erase(uuid);
    
    parent->dynamicsWorld->removeRigidBody(rigidBody);
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

}//namespace smoothly
