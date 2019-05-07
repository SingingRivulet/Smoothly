#ifndef SMOOTHLY_PHYSICAL
#define SMOOTHLY_PHYSICAL
#include "utils.h"
#include <sstream>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
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
                BODY_SUBSTANCE,
                BODY_CHARACTER
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
            
            class bodyBase{
                public:
                    virtual void setPosition(const irr::core::vector3df & );
                    virtual void setRotation(const irr::core::vector3df & );
                    virtual void setDir(const irr::core::vector3df & );
                    virtual void getStatus(irr::core::vector3df & ,irr::core::vector3df & );
                    virtual void setTransform(btTransform & )=0;
                    virtual void getTransform(btTransform & )=0;
                    
                    virtual void setAngularFactor(const btVector3&)=0;
                    
                    virtual void teleport(const irr::core::vector3df &);
                    
                    //character只能使用walk，rigid body只能使用force
                    //能被推动的物体不能被控制，能被控制的物体不能被推动
                    //真不知道bullet作者怎么想的
                    virtual void setWalkDirection(const btVector3& walkDirection){}
                    virtual void jump(const btVector3& dir){}
                    
                    virtual void applyImpulse(const btVector3& impluse,const btVector3& point){}
                    virtual void applyForce(const btVector3& force,const btVector3& point){}
                    virtual void clearForces(){}
                    virtual void setLinearVelocity(const btVector3& lin_vel){}
                    virtual void setAngularVelocity(const btVector3& ang_vel){}
                    virtual void setFriction(btScalar){}
                    virtual void setRestitution(btScalar){}
                    virtual btVector3 getAngularVelocity(){}
                    virtual btVector3 getLinearVelocity(){}
                    
                    virtual void addIntoWorld()=0;
                    virtual void removeFromWorld()=0;
                    
                    virtual void setUserPointer(void *)=0;
                    
                    virtual void loop(){}
                    
                    btDynamicsWorld * world;
                    
                    virtual void destruct(){}
            };
            
            class character:public bodyBase{
                public:
                    btKinematicCharacterController * controller;
                    btPairCachingGhostObject * m_ghostObject;
                    btConvexShape* shape;
                    character(btScalar w,btScalar h,const btVector3 & position,btScalar stepHeight = btScalar(0.35));
                    virtual void destruct();
                    virtual void addIntoWorld();
                    virtual void removeFromWorld();
                    
                    virtual void setWalkDirection(const btVector3& walkDirection);
                    virtual void jump(const btVector3& dir);
                    
                    virtual void setAngularFactor(const btVector3&);
                    
                    virtual void setUserPointer(void *);
                    
                    virtual void setTransform(btTransform & );
                    virtual void getTransform(btTransform & );
            };
            
            class rigidBody:public bodyBase{
                public:
                    btRigidBody             * body;
                    btDefaultMotionState    * bodyState;
                    
                    rigidBody(
                        btCollisionShape *,
                        btTransform & ,
                        btScalar mass=0.0f , 
                        const btVector3& localInertia=btVector3(0,0,0)
                    );
                    virtual void destruct();
                    
                    virtual void addIntoWorld();
                    virtual void removeFromWorld();
                    
                    virtual void applyImpulse(const btVector3& impluse,const btVector3& point);
                    virtual void applyForce(const btVector3& force,const btVector3& point);
                    virtual void clearForces();
                    
                    virtual void setLinearVelocity(const btVector3& lin_vel);
                    virtual void setAngularVelocity(const btVector3& ang_vel);
                    virtual void setFriction(btScalar);
                    virtual void setRestitution(btScalar);
                    virtual btVector3 getAngularVelocity();
                    virtual btVector3 getLinearVelocity();
                    
                    virtual void setAngularFactor(const btVector3&);
                    
                    virtual void setUserPointer(void *);
                    
                    virtual void setTransform(btTransform & );
                    virtual void getTransform(btTransform & );
            };
            
    };
}
#endif