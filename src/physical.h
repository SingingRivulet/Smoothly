#ifndef SMOOTHLY_PHYSICAL
#define SMOOTHLY_PHYSICAL
#include "utils.h"
#include <sstream>
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
            static void               createMeshByTerrain();
            static void               addQuadIntoMesh (btTriangleMesh * mesh,
                const btVector3 & A,
                const btVector3 & B,
                const btVector3 & C,
                const btVector3 & D
            );
            
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
            
            class shapeGroup{
                public:
                
                    btCompoundShape * compound;
                    std::list<btCollisionShape*> children;
                    btScalar mass;
                    btVector3 localInertia;
                    
                    virtual void init(const std::string & conf);
                    virtual void release();
                
                private:
                    
                    void parseLine(const char * str);
                    void add(btCollisionShape * obj,const btVector3& position,const btQuaternion& rotation);
                    virtual void setFric(float f);
                    virtual void setResti(float r);
            };
            class bodyGroup:public shapeGroup{
                public:
                    btDefaultMotionState* motion;
                    btRigidBody * body;
                    float f,e;
                    virtual void init(const std::string & conf);
                    virtual void release();
                
                private:
                    virtual void setFric(float f);
                    virtual void setResti(float r);
                
            };
            
    };
}
#endif