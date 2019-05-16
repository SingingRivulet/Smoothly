#include "subani.h"
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
void subsaniChar::doAni(int speed,int start,int end ,bool loop){
    if(body)
        doAni(body,speed,start,end,loop);
}
void subsaniChar::setItem(irr::u32 id,irr::scene::IAnimatedMesh * mesh){
    if(body==NULL)
        return;
    auto it=items.find(id);
    if(it!=items.end()){
        it->second->removeAll();
        it->second->remove();
    }
    auto joint = body->getJointNode(id);
    auto p = scene->addAnimatedMeshSceneNode(mesh,joint);
    items[id]=p;
}
void subsaniChar::setPart(irr::u32 id,irr::scene::IAnimatedMesh * mesh){
    auto it=parts.find(id);
    if(it!=parts.end()){
        it->second->removeAll();
        it->second->remove();
    }
    auto p=scene->addAnimatedMeshSceneNode(mesh);
    parts[id]=p;
}
void subsaniChar::setBody(irr::scene::IAnimatedMesh * mesh){
    if(body==NULL)
        return;//body不允许现场修改
    body=scene->addAnimatedMeshSceneNode(mesh);
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

void subsaniChar::doAni(irr::scene::IAnimatedMesh * n,int speed,int start,int end ,bool loop){
    n->setAnimationSpeed(speed); 
    n->setLoopMode(loop); 
    n->setFrameLoop(start,end); 
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
void subsaniChar::setRotation(const irr::core::vector3df & r){
    for(auto it:parts){
        it.second->setRotation(r);
    }
    if(body){
        body->setRotation(r);
    }
}
void subsaniChar::setDirection(const irr::core::vector3df & d){
    direction=d;
    irr::core::vector3df rotate=d.getHorizontalAngle();
    rotate.X=0;
    rotate.Z=0;
    setRotation(rotate);
}
void subsaniChar::setStatus(int id){
    
}

void subsaniChar::destruct(){
    removeAll();
    delete this;
}

subsaniChar::subsaniChar(irr::scene::ISceneManager * scene){
    
}

}