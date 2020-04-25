#include "fire.h"

namespace smoothly{

fire::bullet::bullet():owner(){
    rigidBody   = NULL;
    bodyState   = NULL;
    parent      = NULL;
    node        = NULL;
    attack      = false;
    config      = NULL;
    expire      = 0;
}

void fire::bullet::update(){
    btTransform t;
    bodyState->getWorldTransform(t);
    setPositionByTransform(node , t);
}
void fire::bulletAttackBody(bullet * b , const btCollisionObject * body){
    auto ptr = (bodyInfo*)body->getUserPointer();
    attackBody(b->owner , b->config , ptr);
    if(b->config->breakWhenHit){
        releaseBullet(b);
    }
}
void fire::processBulletRemove(){
    for(auto b:bulletRemove){
        b->node->removeAll();
        b->node->remove();
        dynamicsWorld->removeRigidBody(b->rigidBody);
        delete b->rigidBody;
        delete b->bodyState;
        bullets.erase(b);
        delete b;
    }
    bulletRemove.clear();
}
void fire::releaseBullet(bullet * b){
    bulletRemove.insert(b);
}
void fire::shoot(const std::string &uuid, fireConfig * conf, const vec3 &from, const vec3 &dir, bool attack){
    auto b          = new bullet;
    b->config       = conf;
    b->info.type    = BODY_BULLET;
    b->info.ptr     = b;
    b->parent       = this;
    b->attack       = attack;
    b->owner        = uuid;

    b->expire       = timer->getRealTime() + conf->lifeTime;

    //create scene node
    b->node         = scene->addEmptySceneNode();
    if(conf->bulletConf.have){
        if(conf->bulletConf.billboardMode){
            if(conf->bulletConf.texture){

                //set bullet as billboard
                auto n = scene->addBillboardSceneNode(b->node,irr::core::dimension2df(conf->billboardSize,conf->billboardSize));
                n->setMaterialFlag(irr::video::EMF_LIGHTING, conf->bulletConf.light );
                n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
                n->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
                n->setMaterialTexture( 0 , conf->bulletConf.texture);

            }
        }else{
            if(conf->bulletConf.mesh){

                //mesh
                auto n = scene->addMeshSceneNode(conf->bulletConf.mesh , b->node);
                n->setMaterialFlag(irr::video::EMF_LIGHTING, conf->bulletConf.light );
                n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
                n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);

            }
        }
    }

    //create particle
    if(conf->particleConfig.have){
        auto ps = scene->addParticleSystemSceneNode(false , b->node);
        auto em = ps->createPointEmitter(
                    vec3(0.005,0,0),
                    conf->particleConfig.minParticlesPerSecond  ,   conf->particleConfig.maxParticlesPerSecond,
                    conf->particleConfig.minStartColor          ,   conf->particleConfig.maxStartColor,
                    conf->particleConfig.lifeTimeMin            ,   conf->particleConfig.lifeTimeMax,
                    conf->particleConfig.maxAngleDegrees,
                    conf->particleConfig.minStartSize           ,   conf->particleConfig.maxStartSize
                    );
        ps->setEmitter(em);//粒子系统设置发射器
        em->drop();
        ps->setParticlesAreGlobal(true);

        ps->setMaterialFlag(irr::video::EMF_LIGHTING, conf->particleConfig.light);
        ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true);
        if(conf->particleConfig.texture){
            ps->setMaterialTexture(0, conf->particleConfig.texture); // fireball
            //ps->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);
            ps->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
        }
        if(conf->particleConfig.gravity.have){
            auto af = ps->createGravityAffector(conf->particleConfig.gravity.gravity,conf->particleConfig.gravity.timeForceLost);
            ps->addAffector(af);
            af->drop();
        }
        auto fd = ps->createFadeOutParticleAffector(video::SColor(0,0,0,0),500);
        ps->addAffector(fd);
        fd->drop();
    }

    //set node status
    b->node->setPosition(from);
    b->node->setRotation(dir.getHorizontalAngle());
    b->node->updateAbsolutePosition();

    b->bodyState    = setMotionState(b->node->getAbsoluteTransformation().pointer());//创建状态
    b->rigidBody    = createBody(conf->shape.compound,b->bodyState);//创建物体
    b->rigidBody->setUserPointer(&(b->info));
    b->rigidBody->setMassProps(conf->mass,conf->inertia);

    dynamicsWorld->addRigidBody(b->rigidBody);

    vec3 imp = dir;
    imp.normalize();
    imp*=conf->impulse;
    b->rigidBody->applyCentralImpulse(btVector3(imp.X , imp.Y , imp.Z));

    bullets.insert(b);
}

bool fire::processEmitter(emitter *em){
    auto ntm = timer->getRealTime();
    if(em->leave<=0){
        return true;
    }

    auto dt         = ntm - em->lastProcess;
    int sendNum     = (dt * em->config->streamParticleVelocity)/1000.0f;;

    bool release    = false;
    if(sendNum>=em->leave){
        sendNum     = em->leave;
        release     = true;
    }

    if(sendNum>0){
        auto p = seekBody(em->uuid);
        if(p==NULL){

        }else{
            auto pos = p->node->getPosition();
            auto dir = p->lookAt;
            for(int i=0;i<sendNum;++i){
                shoot(em->uuid , em->config , pos , dir , em->attack);
            }
        }
        em->leave  -= sendNum;//没显示出射流不影响计数
        em->lastProcess = ntm;
    }

    return release;

}

void fire::releaseEmitter(emitter *em){
    emitters.erase(em);
    delete em;
}

void fire::processBullets(){
    auto tm = timer->getRealTime();
    std::list<bullet*> brm;
    for(auto it:bullets){//找出过期的
        if(it->expire<tm)
            brm.push_back(it);
    }
    for(auto it:brm){//删除过期的
        releaseBullet(it);
    }
    brm.clear();
    for(auto it:bullets){//更新节点
        it->update();
    }
    //执行发射器
    std::list<emitter*> erm;
    for(auto it:emitters){
        if(processEmitter(it))
            erm.push_back(it);
    }
    for(auto it:erm){
        releaseEmitter(it);
    }
    processBulletRemove();
}
void fire::worldLoop(){
    body::loop();
    engine::worldLoop();
    this->processBullets();
}

}
