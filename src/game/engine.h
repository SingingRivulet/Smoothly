#ifndef SMOOTHLY_ENGINE
#define SMOOTHLY_ENGINE
#include "physical.h"
#include "../libclient/terrainGen.h"
#include "RealisticWater.h"
namespace smoothly{
    inline void rotate2d(irr::core::vector2df & v,double a){
        auto cosa=cos(a);
        auto sina=sin(a);
        auto x=v.X*cosa - v.Y*sina;
        auto y=v.X*sina + v.Y*cosa;
        v.X=x;
        v.Y=y;
        //v.normalize();
    }
    inline float mhtDist(const irr::core::vector3df & A,const irr::core::vector3df & B){
        irr::core::vector3df d=A-B;
        return fabs(d.X)+fabs(d.Y)+fabs(d.Z);
    }
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
            
            inline float getDeltaTime(){
                return deltaTime;
            }
            
            virtual void onCollision(btPersistentManifold * contactManifold)=0;
            
            float waterLevel;

            float width,height;

            bool running;

        private:
            float deltaTime,lastTime;
            bool deltaTimeUpdateFirst;
            RealisticWaterSceneNode * water;
            int lastFPS;
            virtual void renderSky()=0;

    };
}
#endif
