#ifndef SMOOTHLY_ENGINE
#define SMOOTHLY_ENGINE
#include "physical.h"
#include "../libclient/terrainGen.h"
namespace smoothly{
    class engine:public physical,public smoothly::world::terrain::terrainGen{
        public:
            irr::IrrlichtDevice       * device;
            irr::scene::ISceneManager * scene;//场景
            irr::gui::IGUIEnvironment * gui;
            irr::ITimer               * timer;
            irr::video::IVideoDriver * driver;
            irr::scene::ICameraSceneNode * camera;
            
            btDiscreteDynamicsWorld   * dynamicsWorld;
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;
            
            engine();
            ~engine();
            virtual void sceneLoop();
            virtual void worldLoop();
            void deltaTimeUpdate();
            
            inline int getDeltaTime(){
                return deltaTime;
            }
            
            virtual void onCollision(btPersistentManifold * contactManifold)=0;
            
        private:
            int deltaTime,lastTime;
            bool deltaTimeUpdateFirst;
    };
}
#endif
