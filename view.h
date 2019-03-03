#ifndef SMOOTHLY_CLIENT_VIEW
#define SMOOTHLY_CLIENT_VIEW
#include "clientNetwork.h"
namespace smoothly{
    class view:public clientNetwork{
        public:
            void loadScene();
            void destroyScene();
            void loadWorld();
            void destroyWorld();
            void worldLoop();
            void sceneLoop();
            
            float deltaTime,lastTime;
            bool deltaTimeUpdateFirst;
            void deltaTimeUpdate();
            
            irr::scene::ICameraSceneNode * camera;
            
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;
            btDiscreteDynamicsWorld* dynamicsWorld;
            irr::ITimer * timer;
            irr::IrrlichtDevice * device;
            irr::video::IVideoDriver * driver;
            
            view();
            ~view();
    };
}
#endif
