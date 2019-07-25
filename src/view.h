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
            
            float lastTime;
            bool deltaTimeUpdateFirst;
            void deltaTimeUpdate();
            
            irr::scene::ICameraSceneNode * camera;
            
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;
            irr::video::IVideoDriver * driver;
            
            view();
            ~view();
    };
}
#endif
