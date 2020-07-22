#include "body.h"

namespace smoothly{

void body::bodyItem::walk(int forward,int leftOrRight/*-1 left,1 right*/,float speed){

    irr::core::vector2df delta(0,0);
    vec3 direct=lookAt;
    irr::core::vector2df direct2d(direct.X , direct.Z);
    irr::core::vector2df p2d;
    direct2d.normalize();

    if(forward==1){
        delta+=direct2d;
    }else
    if(forward==-1){
        delta-=direct2d;
    }

    if(leftOrRight==-1){
        p2d=direct2d;
        rotate2d(p2d, 3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }else
    if(leftOrRight==1){
        p2d=direct2d;
        rotate2d(p2d, -3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }
    delta.normalize();
    delta*=speed;
    m_character.setWalkDirection(btVector3(delta.X , 0 , delta.Y));
}

void body::bodyItem::setLookAt(const vec3 & l){
    m_character.setDir(l);
    lookAt = l;
}

void body::bodyItem::interactive_c(const std::string & s){
    interactive(s.c_str());
}
void body::bodyItem::wearing_add_c(int d){
    auto it = parent->wearingToBullet.find(d);
    if(it!=parent->wearingToBullet.end()){
        if(firingWearingId!=0){
            wearing_remove_c(firingWearingId);
        }
    }
    parent->addWearing(this , d);
    parent->cmd_wearing_add(uuid , d);
}
void body::bodyItem::wearing_remove_c(int d){
    parent->removeWearing(this , d);
    parent->cmd_wearing_remove(uuid , d);
}
void body::bodyItem::wearing_clear_c(){
    for(auto it:wearing)
        parent->cmd_wearing_remove(uuid,it.first);
    parent->setWearing(this , std::set<int>());
}
void body::bodyItem::HP_inc_c(int d){
    hp = d;
    parent->cmd_HPInc(uuid , d);
}

void body::doCommond(const commond & c){
    bodyItem * p;
    if(c.uuid.empty()){//没有uuid，使用main control
        p = mainControlBody;
    }else{
        p = seekMyBody(c.uuid);
    }
    if(p==NULL)
        return;

    switch (c.cmd) {

        case CMD_SET_LOOKAT:
            p->setLookAt(c.data_vec);
            break;

        case CMD_SET_POSITION:
            p->m_character.setPosition(c.data_vec);
            break;

        case CMD_SET_ROTATION:
            p->m_character.setRotation(c.data_vec);
            break;

        case CMD_JUMP:
        {
            if(!p->uncreatedChunk){
                auto v = c.data_vec;
                v.normalize();
                v*=p->config->jump;
                p->m_character.jump(btVector3(v.X , v.Y , v.Z));
            }
        }
            break;

        case CMD_STATUS_SET:
            p->status_mask   = c.data_int;
            p->status        = c.data_int;
            //cmd_setStatus(p->uuid , p->status_mask);
            break;

        case CMD_STATUS_ADD:
            p->status_mask   = p->status.toMask();
            p->status_mask  |= c.data_int;
            p->status        = p->status_mask;
            //cmd_setStatus(p->uuid , p->status_mask);
            break;

        case CMD_STATUS_REMOVE:
            p->status_mask   = p->status.toMask();
            p->status_mask  &= ~c.data_int;
            p->status        = p->status_mask;
            //cmd_setStatus(p->uuid , p->status_mask);
            break;

        case CMD_INTERACTIVE:
            p->interactive_c(c.data_str);
            break;

        case CMD_WEARING_ADD:
            p->wearing_add_c(c.data_int);
            break;

        case CMD_WEARING_REMOVE:
            p->wearing_remove_c(c.data_int);
            break;

        case CMD_WEARING_CLEAR:
            p->wearing_clear_c();
            break;

        case CMD_FIRE_BEGIN:
            p->firing = true;
            break;

        case CMD_FIRE_END:
            p->firing = false;
            break;

        case CMD_TOOL_RELOAD_START:
            p->reloadStart();
            break;

        case CMD_TOOL_RELOAD_END:
            p->reloadEnd();
            break;

        case CMD_TOOL_USE_ID:
            p->useToolById(c.data_int);
            break;

        case CMD_TOOL_USE:
            cmd_useBagTool(p->uuid.c_str() , c.data_str.c_str());
            break;

        default:
            break;
    }
}
void body::pushCommond(const commond & c){
    cmdQueue_locker.lock();
    cmdQueue.push(c);
    cmdQueue_locker.unlock();
}
void body::doCommonds(){
    cmdQueue_locker.lock();
    while(!cmdQueue.empty()){
        auto c = cmdQueue.front();
        doCommond(c);
        cmdQueue.pop();
    }
    cmdQueue_locker.unlock();
}

void body::attackPositionStart(){
    if(attackingTarget)
        return;
    attackingTarget = true;
    auto ori    = camera->getPosition();
    auto dir    = camera->getTarget()-ori;
    dir.normalize();
    auto start  = ori;
    auto end    = ori+dir*200;

    fetchByRay(start , end,[&](const vec3 & p,bodyInfo * ){
        attackPositionStart(p);
        auto tg = scene->addBillboardSceneNode(0,core::dimension2d<f32>(5,5),p+vec3(0,1,0));
        tg->setMaterialTexture(0,texture_attackTarget);
        tg->setMaterialFlag(irr::video::EMF_LIGHTING, false );
        tg->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        auto am = scene->createDeleteAnimator(1000);
        tg->addAnimator(am);
        am->drop();
    });
}

void body::attackPositionStart(const vec3 & posi){
    for(auto it:selectedBodies){
        it->behaviorStatus.attackTarget = posi;
        if(!it->behaviorStatus.haveAttackTarget){
            it->behaviorStatus.attackTargetFlag = 1;
            it->behaviorStatus.haveAttackTarget = true;
        }
    }
}

void body::attackPositionEnd(){
    attackingTarget = false;
    for(auto it:myBodies){
        if(it.second->behaviorStatus.haveAttackTarget){
            it.second->behaviorStatus.haveAttackTarget = false;
            it.second->behaviorStatus.attackTargetFlag = -1;
        }
    }
}

}
