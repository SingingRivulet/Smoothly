#include "subsani.h"
namespace smoothly{

void subsaniChar::doAniItem(irr::u32 id,int speed,int start,int end ,bool loop){
    auto it=items.find(id);
    if(it!=items.end()){
        doAni(it->second,speed,start,end,loop);
    }
}
void subsaniChar::doAniPart(irr::u32 id,int speed,int start,int end ,bool loop){
    auto it=parts.find(id);
    if(it!=parts.end()){
        doAni(it->second,speed,start,end,loop);
    }
}
void subsaniChar::doAni(int id){
    auto it=conf->boneAnimation.find(id);
    if(it!=conf->boneAnimation.end()){
        for(auto i:it->second.item){
            doAniItem(i.id , i.speed , i.start , i.end , i.loop);
        }
        for(auto i:it->second.pair){
            doAniPart(i.id , i.speed , i.start , i.end , i.loop);
        }
        doAni(it->second.body.speed , it->second.body.start , it->second.body.end , it->second.body.loop);
    }
}
void subsaniChar::doAni(int speed,int start,int end ,bool loop){
    if(body)
        doAni(body,speed,start,end,loop);
}
void subsaniChar::setItem(irr::u32 id,mods::animationConf * am){
    if(body==NULL)
        return;
    auto it=items.find(id);
    if(it!=items.end()){
        it->second->removeAll();
        it->second->remove();
    }
    auto joint = body->getJointNode(id);
    auto p = scene->addAnimatedMeshSceneNode(am->mesh,joint);
    
    if(am->texture){
        p->setMaterialTexture( 0 , am->texture);
        if(am->useAlpha){
            p->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    items[id]=p;
}
void subsaniChar::setPart(irr::u32 id,mods::animationConf * am){
    auto it=parts.find(id);
    if(it!=parts.end()){
        it->second->removeAll();
        it->second->remove();
    }
    auto p=scene->addAnimatedMeshSceneNode(am->mesh);
    
    if(am->texture){
        p->setMaterialTexture( 0 , am->texture);
        if(am->useAlpha){
            p->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    
    p->setPosition(body->getPosition());
    p->setRotation(body->getRotation());
    p->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    p->updateAbsolutePosition();
    
    parts[id]=p;
}

void subsaniChar::removeItem(irr::u32 id){
    auto it=items.find(id);
    if(it!=items.end()){
        it->second->removeAll();
        it->second->remove();
        items.erase(it);
    }
}
void subsaniChar::removePart(irr::u32 id){
    auto it=parts.find(id);
    if(it!=parts.end()){
        it->second->removeAll();
        it->second->remove();
        parts.erase(it);
    }
}

void subsaniChar::doAni(irr::scene::IAnimatedMeshSceneNode * n,int speed,int start,int end ,bool loop){
    n->setAnimationSpeed(speed); 
    n->setLoopMode(loop);
    n->setFrameLoop(start,end); 
}

void subsaniChar::mount(int p,int mesh){
    auto it=conf->boneMapping.find(p);
    if(it!=conf->boneMapping.end()){
        
        auto mit=m->animations.find(mesh);
        if(mit==m->animations.end())
            return;
        
        if(it->second.first){//on body
            setItem(it->second.second , mit->second);
        }else{
            setPart(it->second.second , mit->second);
        }
    }
}
void subsaniChar::umount(int p){
    auto it=conf->boneMapping.find(p);
    if(it!=conf->boneMapping.end()){
        if(it->second.first){//on body
            removeItem(it->second.second);
        }else{
            removePart(it->second.second);
        }
    }
}

void subsaniChar::removeAll(){
    for(auto it:parts){
        it.second->removeAll();
        it.second->remove();
    }
    if(body){
        body->removeAll();
        body->remove();
    }
    parts.clear();
    items.clear();//items不需要单独remove
}

void subsaniChar::setLOD(float len){
    
}

void subsaniChar::setPosition(const irr::core::vector3df & p){
    for(auto it:parts){
        it.second->setPosition(p);
    }
    if(body){
        body->setPosition(p);
    }
}
void subsaniChar::setRotation(const irr::core::vector3df & ir){
    irr::core::vector3df r=ir;
    r.X=0;
    r.Z=0;
    for(auto it:parts){
        it.second->setRotation(r);
    }
    if(body){
        body->setRotation(r);
    }
}
void subsaniChar::setDirection(const irr::core::vector3df & d){
    irr::core::vector3df direction=d;
    irr::core::vector3df rotate=d.getHorizontalAngle();
    rotate.X=0;
    rotate.Z=0;
    setRotation(rotate);
}
void subsaniChar::playAnimation(float dtm,const irr::core::vector3df & dl){
    if(dtm==0)//时间没变
        return;
    
    float sp=sqrt(dl.X*dl.X + dl.Z*dl.Z)/dtm;
    if(sp<0.01){//速度太小
        walking=false;
        return;
    }
    
    walking=true;
    
    this->speed=sp;
    
    updateAnimation();
}
void subsaniChar::setStatus(int id){
    //          站     蹲     趴    飞
    //          0      1     2     3
    //单手近战    4      5     6     7
    //双手近战    8      9     10   11
    //单手远战    12     13    14   15
    //双手远战    16     17    18   19
    status=id;
    
    updateAnimation();
}

void subsaniChar::updateAnimation(){
    int aniId=status*8;
    
    if(walking)
        aniId+=1;
    
    doAni(aniId);
}

void subsaniChar::destruct(){
    removeAll();
    delete this;
}

subsaniChar::subsaniChar(
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
    
    body=scene->addAnimatedMeshSceneNode(sconf->mesh);
    
    if(conf->texture){
        body->setMaterialTexture( 0 , conf->texture);
        if(conf->useAlpha){
            body->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    body->setPosition(p);
    body->setRotation(irr::core::vector3df(0 , r.Y , 0));
    body->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    body->updateAbsolutePosition();
    
}

void subsaniChar::doAttaching(const std::list<ipair> & added,const std::list<ipair> & removed){
    for(auto it:added){
        mount(it.x,it.y);
    }
    for(auto it:removed){
        umount(it.x);
    }
}

const irr::core::vector3df & subsaniChar::getPosition(){
    return body->getPosition();
}
const irr::core::vector3df & subsaniChar::getRotation(){
    return body->getRotation();
}

const irr::core::matrix4 & subsaniChar::getAbsoluteTransformation(){
    return body->getAbsoluteTransformation();
}
            
void subsaniChar::updateAbsolutePosition(){
    body->updateAbsolutePosition();
}

}