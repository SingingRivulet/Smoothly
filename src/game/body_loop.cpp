#include "body.h"
namespace smoothly{

void body::loop(){
    for(auto it:myBodies){
        bodyItem * b = it.second;
        if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的

            //设置行走
            b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity*timer->getRealTime());

            //设置dispather
            btTransform transform;
            b->m_character.getTransform(transform);

            btVector3 btPos;
            btPos = transform.getOrigin();
            setCharacterChunk(b->uuid,btPos.x()/32,btPos.z()/32);
        }
    }
    for(auto it:bodies){
        bodyItem * b = it.second;
        b->updateFromWorld();
    }
}

}
