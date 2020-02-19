#include "body.h"
namespace smoothly{

void body::loop(){
    for(auto it:bodies){
        bodyItem * b = it.second;
        if(b->uncreatedChunk){
            int cx = b->lastPosition.X/32;
            int cy = b->lastPosition.Z/32;
            if(chunkCreated(cx,cy)){
                b->m_character.setGravity(gravity);
                b->uncreatedChunk = false;
            }else{
                //b->m_character.setPosition(b->lastPosition);
                continue;
            }
        }
        b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
        b->updateFromWorld();
        auto p = b->node->getPosition();
        auto h = getRealHight(p.X,p.Z);
        if(p.Y<h){
            p.Y = (b->config->height+b->config->width)*0.5+h;//防止掉出地图
            b->m_character.setPosition(p);
        }
        if((p.X+p.Z)>32*4){
            b->node->setVisible(false);
        }else{
            b->node->setVisible(true);
        }
    }
    for(auto it:myBodies){
        bodyItem * b = it.second;
        if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的

            //设置行走
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
            //转移到上面了
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity*timer->getRealTime());

            //设置dispather
            btTransform transform;
            b->m_character.getTransform(transform);

            btVector3 btPos;
            btPos = transform.getOrigin();
            setCharacterChunk(b->uuid,btPos.x()/32,btPos.z()/32);

            if(b->firing)
                b->doFire();
        }
    }
    doCommonds();
}

}
