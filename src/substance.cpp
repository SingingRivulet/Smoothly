#include "substance.h"
namespace smoothly{

typedef mempool<substance::subs> subsPool;

class closeCombCallback:public btCollisionWorld::ConvexResultCallback{
    public:
        substance::subs * parent;
        mods::attackConf * attconf;
        virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult & convexResult,bool normalInWorldSpace){
            parent->attackBody(attconf , convexResult.m_hitCollisionObject);
        }
};
void substance::subs::shoot(
    mods::attackConf * att,
    const irr::core::vector3df & from,
    const irr::core::vector3df & dir
){//发射
    
    irr::core::vector3df rd=dir;
    rd.normalize();
    rd*=10;
    
    auto rgen = parent->device->getRandomizer();
    
    for(int i=0;i < att->bulletNum ;i++){
    
        rd.X+=rgen->frand() * att->scatter;
        rd.Y+=rgen->frand() * att->scatter;
        rd.Z+=rgen->frand() * att->scatter;
        
        parent->addBriefSubs(
            att->bulletSubsId,
            from,
            irr::core::vector3df(0,0,0),
            rd,
            btVector3(rd.X , rd.Y , rd.Z),
            att->rel_pos
        );
        
    }
}
void substance::subs::attackBody(mods::attackConf * att,const btCollisionObject * body){//处理攻击结果
    auto ptr=(bodyInfo*)body->getUserPointer();
    if(ptr){
        if(ptr->type==BODY_TERRAIN){
            auto target=(terrain::chunk*)ptr->ptr;
            
        }else
        if(ptr->type==BODY_TERRAIN_ITEM){
            auto target=(terrain::item*)ptr->ptr;
            auto mid=mapid(target->inChunk->x , target->inChunk->y , target->id , target->mapId);
            att->onAttackTerrainItem(parent,uuid,mid);
        }else
        if(ptr->type==BODY_BUILDING){
            auto target=(remoteGraph::item*)ptr->ptr;
            auto tuuid=target->uuid;
            att->onAttackBuilding(parent,uuid,tuuid);
        }else
        if(ptr->type==BODY_SUBSTANCE || ptr->type==BODY_CHARACTER){
            auto target=(substance::subs*)ptr->ptr;
            auto tuuid=target->uuid;
            att->onAttackSubstance(parent,uuid,tuuid);
        }
    }
}

bool substance::subs::doAttackActive(
    int wpid,
    const irr::core::vector3df & from,
    irr::core::vector3df & dir
){
    auto it=parent->m->attackings.find(wpid);
    if(it==parent->m->attackings.end())
        return false;
    //获取类型为bodyInfo
    
    if(dir.X==0 && dir.Y==0 && dir.Z==0)
        return false;
    
    dir.normalize();
    dir*=it->second->range;//取得射程
    
    if(it->second->type==WEAPON_LASER){
        
        auto to=from+dir;
        
        btVector3 bfrom(from.X,from.Y,from.Z),bto(to.X,to.Y,to.Z);//转换为bullet向量
        
        btCollisionWorld::ClosestRayResultCallback rayCallback(bfrom,bto);
        parent->dynamicsWorld->rayTest(bfrom, bto, rayCallback);//使用bullet的rayTest接口
        
        if (rayCallback.hasHit()){
            //攻击到物体
            //被攻击的物体是m_collisionObject
            //激光终点是m_hitPointWorld
            irr::core::vector3df targ(
                rayCallback.m_hitPointWorld.getX(),
                rayCallback.m_hitPointWorld.getY(),
                rayCallback.m_hitPointWorld.getZ()
            );
            dir=targ-from;
            attackBody(it->second , rayCallback.m_collisionObject);
        }
        
    }else
    if(it->second->type==WEAPON_CLOSE || it->second->type==WEAPON_EXPLODE){
        
        if(it->second->castShape){
            
            btTransform bfrom,bto;
            
            auto rotate=dir.getHorizontalAngle();//旋转角度
            
            irr::core::matrix4 matF,matT;//从矩阵F到矩阵T
            
            matF.setRotationDegrees(rotate);
            matT=matF;
            
            matF.setTranslation(from);
            matT.setTranslation(from+dir);
            
            bfrom.setFromOpenGLMatrix(matF.pointer());
            bto.setFromOpenGLMatrix(matT.pointer());
            
            closeCombCallback callback;
            callback.parent=this;
            callback.attconf=it->second;
            //使用convexSweepTest实现
            parent->dynamicsWorld->convexSweepTest(
                it->second->castShape,
                bfrom,
                bto,
                callback
            );
        }
        
    }else
    if(it->second->type==WEAPON_SHOT){
        shoot(it->second,from,dir);//发射
    }
    return true;
}

void substance::subs::setAttaching(const bodyAttaching & att){
    std::list<attachingStatus> added;
    std::list<attachingStatus> removed;
    attaching.diff(att,added,removed);
    attaching=att;
    
    node->doAttaching(added,removed);
}

void substance::setAttaching(const std::string & subsuuid,const bodyAttaching & att){
    auto p=seekSubs(subsuuid);
    if(p){
        p->setAttaching(att);
    }
}

void substance::subs::setMotion(const irr::core::vector3df & p,const irr::core::vector3df & r){
    btTransform transform;
    
    node->setPosition(p);
    node->setRotation(r);
    node->updateAbsolutePosition();//update mtx
    
    transform.setFromOpenGLMatrix(node->getAbsoluteTransformation().pointer());//set transform
    
    body->setTransform(transform);//apply
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
    //if(p)printf("[control]parseCommond begin %s\n",p->owner.c_str());
    if(p && p->owner==myUUID){
        //printf("[control]parseCommond\n");
        irr::core::vector3df tvec;
        switch(cmd.method){
            case subsCommond::JUMP:
                
                tvec=cmd.vec;
                if(tvec.getLengthSQ()!=0){
                    tvec.normalize();
                    p->jump(tvec*p->jumpImp);
                }
                
            break;
            case subsCommond::WALK:
                p->moveUpdate(cmd.walkForward,cmd.walkLeftOrRight);
            break;
            case subsCommond::WALK_STOP:
                p->moveUpdate(0,0);
            break;
            case subsCommond::FLY:
                p->flyUpdate(cmd.flying , cmd.lifting);
            break;
            case subsCommond::FLY_STOP:
                p->flyUpdate(false,false);
            break;
            case subsCommond::DIRECT:
                //break;
                p->setDirection(cmd.vec);
            break;
        }
    }
}
void substance::subs::setPowerAsDefault(){
    walkSpeed   =subsConf->defaultSpeed;
    liftForce   =subsConf->defaultLiftForce;
    pushForce   =subsConf->defaultPushForce;
    jumpImp     =subsConf->defaultJumpImp;
    deltaCamera =subsConf->deltaCamera;
}
void substance::subs::moveUpdate(int forward,int leftOrRight){
    //printf("[control]move update\n");
    /*
    if(!body->onGround()){
        
    }else{
        
    }
    */
    if(forward==1){
        setStatusPair(4,1);//4号是move
    }else
    if(forward==-1){
        setStatusPair(4,3);
    }else
    if(forward==0){
        setStatusPair(4,0);
    }
    walk(forward,leftOrRight,walkSpeed);
}
void substance::subs::flyUpdate(bool flying,bool lifting){
    body->clearForces();//清除之前的力，设置新的力
    if(flying){
        if(lifting){
            fly(liftForce,pushForce);
        }else{
            fly(0,pushForce);
        }
    }
}

void substance::subs::fly(float lift,float push){
    irr::core::vector3df direct=getDirection();
    direct.normalize();
    direct*=push;
    fly(lift,direct);
}
void substance::subs::fly(float lift,const irr::core::vector3df & push){
    btVector3 F(push.X , push.Y+lift , push.Z);
    body->applyForce(F , btVector3(0,0,0));
}
void substance::subs::jump(const irr::core::vector3df & d){
    body->jump(btVector3(d.X , d.Y , d.Z));
}

void substance::subs::setDirection(const irr::core::vector3df & dir){
    body->setDir(dir);
    direction=dir;
}
irr::core::vector3df substance::subs::getDirection(){
    //btTransform transform;
    //btVector3 btRot;
    //irr::core::vector3df rotate;
    //body->getTransform(transform);
    //btMatrix3x3 & btM = transform.getBasis();
    //btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    //rotate.X = irr::core::radToDeg(btRot.x());
    //rotate.Y = irr::core::radToDeg(btRot.y());
    //rotate.Z = irr::core::radToDeg(btRot.z());
    //return rotate.rotationToDirection();
    return direction;
}
void substance::subs::walk(int forward,int leftOrRight/*-1 left,1 right*/,float speed){
    //printf("[control]walk\n");
    if(!inWorld)
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
        rotate2d(p2d, 3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }else
    if(leftOrRight==-1){
        p2d=direct2d;
        rotate2d(p2d, -3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }
    delta.normalize();
    delta*=speed;
    walk(delta);
}

void substance::subs::walk(const irr::core::vector2df & d){
    walk(irr::core::vector3df(d.X,0,d.Y));
}
void substance::subs::walk(const irr::core::vector3df & d){
    move(d);
}
void substance::subs::move(const irr::core::vector3df & delta){
    body->setWalkDirection(btVector3(delta.X , delta.Y , delta.Z));
}
void substance::subs::teleport(const irr::core::vector3df & p){
    //printf("[substance]teleport substance %s to (%f,%f,%f)\n",uuid.c_str(),p.X,p.Y,p.Z);
    body->teleport(p);
}
void substance::subs::setPosition(const irr::core::vector3df & p){
    body->setPosition(p);
    
    //btTransform transform;
    //bodyState->getWorldTransform(transform);
    //transform.setOrigin(btVector3(p.X, p.Y, p.Z));
    //bodyState->setWorldTransform(transform);
    
    //node->updateAbsolutePosition();
    
    //btTransform transform;
    //transform.setFromOpenGLMatrix(node->getAbsoluteTransformation().pointer());
    //body->setTransform(transform);
    
}

void substance::subs::setRotation(const irr::core::vector3df & r){
    //node->setRotation(r);
    
    //btQuaternion rq;
    //euler2quaternion(r,rq);
    
    //btTransform transform;
    //bodyState->getWorldTransform(transform);
    //transform.setRotation(rq);
    //bodyState->setWorldTransform(transform);
    
    //node->updateAbsolutePosition();
    
    //btTransform transform;
    //transform.setFromOpenGLMatrix(node->getAbsoluteTransformation().pointer());
    body->setRotation(r);
    
}

void substance::subs::updateByWorld(){
    if(!inWorld)
        return;
    
    btTransform transform;
    
    body->getTransform(transform);
    
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
    
    //printf("[updateByWorld]irrRot (%f,%f,%f)\n",irrRot.X,irrRot.Y,irrRot.Z);
    
    node->setPosition(irrPos);
    node->setRotation(irrRot);
    
    //printf("[subtance]%s update by world (%f,%f,%f)\n",uuid.c_str(),btPos.x(), btPos.y(), btPos.z());
    
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
        }
        parseAllCommond();
    }
    {//set camera
        if(!mainControlUUID.empty()){
            auto it=subses.find(mainControlUUID);
            if(it!=subses.end()){
                mainControlPosition=it->second->node->getPosition();
                deltaCamera=it->second->deltaCamera;
                
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
void substance::subs::playAnimation(float dtm,const irr::core::vector3df & dl){
    node->playAnimation(dtm,dl);
}
bool substance::subs::haveChanging(){
    return changing.diff(
        node->getPosition(),
        node->getRotation(),
        body->getDir(),
        getLinearVelocity(),
        getAngularVelocity(),
        status
    );
}
void substance::subs::update(){
    if(parent){
        updateByWorld();
        if(inWorld){
            body->loop(parent->deltaTime);
            irr::core::vector3df p;
            body->getDeltaL(p);
            playAnimation(parent->deltaTime,p);
        }
        if(type==mods::SUBS_LASTING && node!=NULL && haveChanging()){
            //upload to server
            //printf("[substance]upload body:%s\n",uuid.c_str());
            parent->uploadBodyStatus(
                uuid,
                status,
                node->getPosition(),
                node->getRotation(),
                body->getDir(),
                getLinearVelocity(),
                getAngularVelocity()
            );
        }
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
    const irr::core::vector3df & dire, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    //向服务器发送创建命令，服务器再向自己发送
    auto p=seekSubsConf(id);
    if(p && p->type==mods::SUBS_LASTING)
        requestCreateLastingSubs(id,posi,rota,dire,impulse,rel_pos);
}

substance::subs * substance::addBriefSubs(//添加非持久物体
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
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
        sp->init(p,posi,rota,dire);
        sp->type=mods::SUBS_BRIEF;
        sp->body->applyImpulse(impulse,rel_pos);
        
        sp->setAsBrief(p->life);
        
        requestCreateBriefSubs(id,posi,rota,dire,impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（持久），由服务器调用
    const std::string & uuid ,
    const std::string & owner ,
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
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
        sp->init(p,posi,rota,dire);
        sp->type=mods::SUBS_LASTING;
        sp->body->applyImpulse(impulse,rel_pos);
    }
}
void substance::genSubs(//添加物体（非持久）
    long id , 
    const std::string & owner ,
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
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
        sp->init(p,posi,rota,dire);
        sp->type=mods::SUBS_BRIEF;
        sp->body->applyImpulse(impulse,rel_pos);
        sp->setAsBrief(p->life);
    }
}
void substance::updateSubs(//更新物体状态，由服务器调用
    long id,
    const std::string & uuid , 
    const std::string & owner ,
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
    const btVector3& lin_vel ,
    const btVector3& ang_vel ,
    int hp,int status,
    const std::string & conf
){
    //printf("[substance]updateSubs\n");
    //printf("[substance]set position:(%f %f %f)\n",posi.X , posi.Y , posi.Z);
    auto sp=seekSubs(uuid);
    if(sp){
        sp->setMotion(posi,rota);
        sp->owner=owner;
        //setOwner(uuid,owner);
        sp->hp=hp;
        sp->status=status;
        sp->setStatus(status);
        sp->setDirection(dire);
        sp->body->setLinearVelocity(lin_vel);
        sp->body->setAngularVelocity(ang_vel);
    }else{
        //printf("[substance]updateSubs create\n");
        auto p=seekSubsConf(id);
        if(p && p->type==mods::SUBS_LASTING){
            //lua文件那边必须将subs设为SUBS_LASTING，物体才能被控制
            auto sp=createSubs();
            sp->uuid=uuid;
            sp->owner=owner;
            //printf("[updateSubs]owner:%s\n",owner.c_str());
            setOwner(uuid,owner);
            sp->hp=hp;
            sp->status=status;
            sp->init(p,posi,rota,dire);
            sp->type=mods::SUBS_LASTING;
            sp->body->setLinearVelocity(lin_vel);
            sp->body->setAngularVelocity(ang_vel);
            sp->setStatus(status);
            printf("[substance]create substance:%s id=%ld\n",uuid.c_str(),id);
        }else{
            printf("[substance]substance:%s id=%ld \e[0;31mno found\e[0m\n",uuid.c_str(),id);
        }
    }
}
void substance::subs::init(
    mods::subsConf * conf,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r,
    const irr::core::vector3df & d
){
    //printf("[substance]init\n");
    if(uuid.empty() || parent==NULL)
        return;
    
    subsConf=conf;
    id=conf->id;
    setPowerAsDefault();
    
    node=subsaniFactory(parent->scene,conf,parent->m,p,r,d);
    //node->m=m;//设置全局配置信息
    
    if(node==NULL)
        return;
    //node=parent->scene->addMeshSceneNode(conf->mesh);
    //node->setPosition(p);
    //node->setRotation(r);
    //node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    //node->updateAbsolutePosition();
    
    attaching.clear();
    
    changing.init();
    
    //if(conf->texture){
    //    node->setMaterialTexture( 0 , conf->texture);
    //    if(conf->useAlpha){
    //        node->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    //    }
    //}
    
    
    info.type=BODY_SUBSTANCE;
    info.ptr=this;
    
    btTransform transform;
    transform.setFromOpenGLMatrix(node->getAbsoluteTransformation().pointer());
    
    if(conf->bodyType=="character"){
        printf("[substance]create character:%s\n",uuid.c_str());
        body=new character(
            conf->characterWidth , 
            conf->characterHeight , 
            btVector3(p.X , p.Y , p.Z),
            conf->walkInSky , 
            conf->jumpInSky 
        );
    }else{
        printf("[substance]create rigid body:%s\n",uuid.c_str());
        body=new rigidBody(conf->bodyShape , transform , conf->shape.mass , conf->shape.localInertia);
    }
    
    body->world=parent->dynamicsWorld;
    
    body->setUserPointer(&info);
    
    body->setFriction(conf->friction);
    body->setRestitution(conf->restitution);
    
    body->setDir(d);
    
    if(conf->noFallDown){
        setNoFallDown();
    }else
        noFallDown=false;
    
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
        //printf("[subtance]unlock(%d,%d)\n",x,y);
        for(auto s:it->second){
            s->body->addIntoWorld();
            s->inWorld=true;
        }
        chunkLocker.erase(it);
        
    }
}

bool substance::subs::addIntoWorld(){
    auto it=parent->chunkLocker.find(ipair(x,y));
    
    if(it==parent->chunkLocker.end()){
        //chunnk has not been locked
        body->addIntoWorld();
        inWorld=true;
        //printf("[subtance]%s has been added into world\n",uuid.c_str());
        return true;
    }else{
        //chunk has been locked
        it->second.insert(this);
        //printf("[subtance]%s has been locked\n",uuid.c_str());
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
    
    node->destruct();
    
    parent->subses.erase(uuid);
    
    if(inWorld)
        body->removeFromWorld();
    else{
        //chunk has been save in x,y
        auto it=parent->chunkLocker.find(ipair(x,y));
    
        if(it!=parent->chunkLocker.end()){
            it->second.erase(this);
        }
        
    }
    
    body->destruct();
    
    body=NULL;
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
