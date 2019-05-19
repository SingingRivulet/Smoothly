#include "subsani.h"
namespace smoothly{

void subsaniStatic::setLOD(float len){
    
}

void subsaniStatic::playAnimation(float dtm,const irr::core::vector3df & dl){
    //nothing
}

const irr::core::vector3df & subsaniStatic::getPosition(){
    return node->getPosition();
}
const irr::core::vector3df & subsaniStatic::getRotation(){
    return node->getRotation();
}

void subsaniStatic::setPosition(const irr::core::vector3df & p){
    node->setPosition(p);
}
void subsaniStatic::setRotation(const irr::core::vector3df & r){
    node->setRotation(r);
}
void subsaniStatic::setDirection(const irr::core::vector3df & d){
    irr::core::vector3df direction=d;
    irr::core::vector3df rotate=d.getHorizontalAngle();
    setRotation(rotate);
}

subsaniStatic::subsaniStatic(
    irr::scene::ISceneManager * sc,
    mods::subsConf * sconf,
    mods * gconf,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r,
    const irr::core::vector3df & d
){
    
    this->conf  =sconf;
    this->m     =gconf;
    this->scene =sc;
    
    node=scene->addAnimatedMeshSceneNode(sconf->mesh);
    
    if(conf->texture){
        node->setMaterialTexture( 0 , conf->texture);
        if(conf->useAlpha){
            node->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    node->setPosition(p);
    node->setRotation(r);
    node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    node->updateAbsolutePosition();
}

void subsaniStatic::destruct(){
    node->remove();
    delete this;
}

void subsaniStatic::doAttaching(const std::list<ipair> & added,const std::list<ipair> & removed){
    //nothing
}

const irr::core::matrix4 & subsaniStatic::getAbsoluteTransformation(){
    return node->getAbsoluteTransformation();
}

void subsaniStatic::updateAbsolutePosition(){
    node->updateAbsolutePosition();
}

subsani * subsaniFactory(
    irr::scene::ISceneManager * scene,
    mods::subsConf * conf,
    mods * m,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r,
    const irr::core::vector3df & d
){
    subsani * res;
    if(conf->animationType=="character"){
        res=new subsaniChar(scene,conf,m,p,r,d);
    }else{
        res=new subsaniStatic(scene,conf,m,p,r,d);
    }
    return res;
}

}