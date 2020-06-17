#ifndef SMOOTHLY_ENGINE
#define SMOOTHLY_ENGINE
#include "physical.h"
#include "../libclient/terrainGen.h"
#include "RealisticWater.h"
#include <vector>
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
            irr::scene::ISceneCollisionManager * collisionManager;
            irr::core::stringc          vendor;
            
            irr::core::vector2di screenCenter;

            btDiscreteDynamicsWorld   * dynamicsWorld;
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;
            
            engine();
            ~engine();
            virtual void sceneLoop();
            virtual void worldLoop();
            virtual void onDraw();
            void deltaTimeUpdate();
            
            inline float getDeltaTime(){
                return deltaTime;
            }
            
            virtual void onCollision(btPersistentManifold * contactManifold)=0;
            
            float waterLevel;

            int width,height;

            bool running;

            irr::gui::IGUIFont * font;

            void drawArcProgressBar(core::position2d<s32> center,
                f32 radius, video::SColor color, s32 count,f32 progress);

        private:
            float deltaTime,lastTime;
            bool deltaTimeUpdateFirst;
            RealisticWaterSceneNode * water;
            int lastFPS;
            virtual void renderSky()=0;
            virtual void renderShadow()=0;

    };
}
#endif
