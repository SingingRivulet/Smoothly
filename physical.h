#ifndef SMOOTHLY_PHYSICAL
#define SMOOTHLY_PHYSICAL
#include "utils.h"
namespace smoothly{
    class physical{
        public:
            static btTriangleMesh   * createBtMesh    (irr::scene::IMesh * irrm);
            static void               setMotionState  (btMotionState * motionState,const float * mtx);
            static void               getMotionState  (btMotionState * motionState,float * mtx);
            static btMotionState    * setMotionState  (const float * mtx);
            static btCollisionShape * createShape     (btTriangleMesh * mesh);
            static btRigidBody      * createBody      (
                btCollisionShape * shape , 
                btMotionState * motionState , 
                btScalar mass=0.0f , 
                const btVector3& localInertia=btVector3(0,0,0)
            );
            static float              getKEnergy      (btRigidBody *);
            enum bodyType{
                BODY_TERRAIN,
                BODY_TERRAIN_ITEM,
                BODY_BUILDING,
                BODY_SUBSTANCE
            };
            struct bodyInfo{
                void * ptr;
                bodyType type;
            };
    };
}
#endif