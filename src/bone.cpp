#include "subsani.h"
namespace smoothly{

void subsaniChar::pushAttackInfo(int boneId,tool & t,attackOri & ori){
    attackEvent ev;
    ev.activity =t.amc->attackActivity;
    ev.node     =t.node;
    ev.boneId   =boneId;
    ori.push_back(ev);
}

void subsaniChar::getAttackNode(const std::list<mods::attackLaunchConf::activity> & mapping,attackOri & ori){
    for(auto it:mapping){
        
        auto itb=conf->boneMapping.find(it.handIndex);//handIndex即起始节点
        if(itb!=conf->boneMapping.end()){
            if(itb->second.first){//on body
                auto itn=items.find(itb->second.second);
                if(itn!=items.end()){
                    pushAttackInfo(it.handIndex,itn->second,ori);
                }
            }else{
                auto itn=parts.find(itb->second.second);
                if(itn!=parts.end()){
                    pushAttackInfo(it.handIndex,itn->second,ori);
                }
            }
        }
        
    }
}

void subsaniChar::doAniItem(irr::u32 id,int speed,int start,int end ,bool loop){
    auto it=items.find(id);
    if(it!=items.end()){
        doAni(it->second.node,speed,start,end,loop);
    }
}
void subsaniChar::doAniPart(irr::u32 id,int speed,int start,int end ,bool loop){
    auto it=parts.find(id);
    if(it!=parts.end()){
        doAni(it->second.node,speed,start,end,loop);
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
void subsaniChar::setItem(irr::u32 id,mods::animationConf * am,int objId,const std::string & uuid){
    if(body==NULL)
        return;
    auto it=items.find(id);
    if(it!=items.end()){
        it->second.node->removeAll();
        it->second.node->remove();
    }
    auto joint = body->getJointNode(id);
    auto p = scene->addAnimatedMeshSceneNode(am->mesh,joint);
    
    if(am->texture){
        p->setMaterialTexture( 0 , am->texture);
        if(am->useAlpha){
            p->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        }
    }
    
    tool t;
    t.node=p;
    t.uuid=uuid;
    t.objId=objId;
    t.amc=am;
    items[id]=t;
}
void subsaniChar::setPart(irr::u32 id,mods::animationConf * am,int objId,const std::string & uuid){
    auto it=parts.find(id);
    if(it!=parts.end()){
        it->second.node->removeAll();
        it->second.node->remove();
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
    
    tool t;
    t.node=p;
    t.uuid=uuid;
    t.objId=objId;
    t.amc=am;
    parts[id]=t;
}

void subsaniChar::removeItem(irr::u32 id){
    auto it=items.find(id);
    if(it!=items.end()){
        it->second.node->removeAll();
        it->second.node->remove();
        items.erase(it);
    }
}
void subsaniChar::removePart(irr::u32 id){
    auto it=parts.find(id);
    if(it!=parts.end()){
        it->second.node->removeAll();
        it->second.node->remove();
        parts.erase(it);
    }
}

void subsaniChar::doAni(irr::scene::IAnimatedMeshSceneNode * n,int speed,int start,int end ,bool loop){
    n->setAnimationSpeed(speed); 
    n->setLoopMode(loop);
    n->setFrameLoop(start,end); 
}

void subsaniChar::mount(const attachingStatus & p){
    auto it=conf->boneMapping.find(p.attOn);
    if(it!=conf->boneMapping.end()){
        
        auto mit=m->animations.find(p.objId);
        if(mit==m->animations.end())
            return;
        
        if(it->second.first){//on body
            setItem(it->second.second , mit->second , p.objId , p.uuid);
        }else{
            setPart(it->second.second , mit->second , p.objId , p.uuid);
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
irr::scene::IAnimatedMeshSceneNode * subsaniChar::getNode(int p){
    auto it=conf->boneMapping.find(p);
    if(it!=conf->boneMapping.end()){
        if(it->second.first){//on body
            auto it2=items.find(it->second.second);
            if(it2!=items.end()){
                return it2->second.node;
            }
        }else{
            auto it2=parts.find(it->second.second);
            if(it2!=parts.end()){
                return it2->second.node;
            }
        }
    }
    return NULL;
}
void subsaniChar::removeAll(){
    for(auto it:parts){
        it.second.node->removeAll();
        it.second.node->remove();
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
        it.second.node->setPosition(p);
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
        it.second.node->setRotation(r);
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
    /*
     * 行走由status设置
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
     */
}
int subsaniChar::setStatusPair(int key,int sta){
    switch(key){
        case 1:
            foot=sta;
        break;
        case 2:
            hand=sta;
        break;
        case 3:
            range=sta;
        break;
        case 4:
            move=sta;
        break;
    }
    status = getCharAnimationId(foot,hand,range,move);
    updateAnimation();
    return status;
}
void subsaniChar::setStatus(int id){
    
    status=id;
    
    updateAnimation();
}

void subsaniChar::updateAnimation(){
    //int aniId=status*8;
    
    //if(walking)
    //    aniId+=1;
    
    doAni(status);
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
    
    foot=0;
    hand=0;
    range=0;
    move=0;
    
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

void subsaniChar::doAttaching(const std::list<attachingStatus> & added,const std::list<attachingStatus> & removed){
    for(auto it:removed){
        umount(it.attOn);
    }//先执行umount，因为可能有不同uuid的绑定过来
    for(auto it:added){
        mount(it);
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