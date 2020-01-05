#include "fire.h"

namespace smoothly{

void fire::shoot(const std::string &uuid, fireConfig * conf, const vec3 &from, const vec3 &dir, bool attack){
    auto b          = new bullet;
    b->config       = conf;
    b->info.type    = BODY_BULLET;
    b->info.ptr     = b;
    b->parent       = this;
    b->attack       = attack;
    b->owner        = uuid;

    //create scene node
    b->node         = scene->addEmptySceneNode();
    if(conf->bulletConf.have){
        if(conf->bulletConf.billboardMode){
            if(conf->bulletConf.texture){

                //set bullet as billboard
                auto n = scene->addBillboardSceneNode(b->node);
                n->setMaterialFlag(irr::video::EMF_LIGHTING, conf->bulletConf.light );
                n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
                n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
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
                    vec3(0,0,-1),
                    conf->particleConfig.minParticlesPerSecond  ,   conf->particleConfig.maxParticlesPerSecond,
                    conf->particleConfig.minStartColor          ,   conf->particleConfig.maxStartColor,
                    conf->particleConfig.lifeTimeMin            ,   conf->particleConfig.lifeTimeMax,
                    conf->particleConfig.maxAngleDegrees,
                    conf->particleConfig.minStartSize           ,   conf->particleConfig.maxStartSize
                    );
        ps->setEmitter(em);//粒子系统设置发射器
        em->drop();

        ps->setParticlesAreGlobal(false);
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
    }

    //set node status
    b->node->setPosition(from);
    b->node->setRotation(dir.getHorizontalAngle());
    b->node->updateAbsolutePosition();

    b->bodyState    = setMotionState(b->node->getAbsoluteTransformation().pointer());//创建状态
    b->rigidBody    = createBody(conf->shape.compound,b->bodyState);//创建物体
    b->rigidBody->setUserPointer(&(b->info));

    dynamicsWorld->addRigidBody(b->rigidBody);
}

}
