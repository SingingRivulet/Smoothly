#include "fire.h"

namespace smoothly{

/*
void fire::processCollision(){
    auto disp=dynamicsWorld->getDispatcher();
    int numManifolds = disp->getNumManifolds();
    for(int i=0;i<numManifolds;i++){
        btPersistentManifold * contactManifold = disp->getManifoldByIndexInternal(i);
        int numContacts= contactManifold->getNumContacts();
        if(numContacts>0){
            const btCollisionObject * obA = contactManifold->getBody0();
            const btCollisionObject * obB = contactManifold->getBody1();

            auto pA=(bodyInfo*)obA->getUserPointer();
            auto pB=(bodyInfo*)obB->getUserPointer();

            if(pA==NULL || pB==NULL)
                continue;

            int numContacts=contactManifold->getNumContacts();
            for(int j=0;j<numContacts;j++){

                auto point=contactManifold->getContactPoint(j);

                //float impulse=point.getAppliedImpulse();

                onCollision(pA,pB,point);

            }
        }
    }
}
*/
void fire::onCollision(btPersistentManifold *contactManifold){
    int numContacts= contactManifold->getNumContacts();
    if(numContacts>0){
        const btCollisionObject * obA = contactManifold->getBody0();
        const btCollisionObject * obB = contactManifold->getBody1();

        auto pA=(bodyInfo*)obA->getUserPointer();
        auto pB=(bodyInfo*)obB->getUserPointer();

        if(pA==NULL || pB==NULL)
            return;

        int numContacts=contactManifold->getNumContacts();
        for(int j=0;j<numContacts;j++){

            auto point=contactManifold->getContactPoint(j);

            //float impulse=point.getAppliedImpulse();

            onCollision(pA,pB,point);

        }
    }
}
void fire::onCollision(bodyInfo *A, bodyInfo *B, const btManifoldPoint &point){
    if(A->type==BODY_BULLET){
        if(B->type!=BODY_BULLET){
            auto p = (bullet*)A->ptr;
            attackBody(p->owner , p->config , B , point.getAppliedImpulse());
        }
    }else
    if(B->type==BODY_BULLET){
        if(A->type!=BODY_BULLET){
            auto p = (bullet*)B->ptr;
            attackBody(p->owner , p->config , A , point.getAppliedImpulse());
        }
    }
}

}
