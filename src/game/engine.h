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
            void renderSky();
            struct skyBox{
                irr::IrrlichtDevice       * device;
                irr::ITimer               * timer;
                irr::video::IVideoDriver  * driver;
                irr::scene::ISceneManager * scene;
                irr::video::SMaterial cloudMaterial;
                irr::video::ITexture * cloudTop;
                irr::video::ITexture * cloudFront;
                irr::video::ITexture * cloudBack;
                irr::video::ITexture * cloudLeft;
                irr::video::ITexture * cloudRight;
                irr::scene::ISceneNode * box;
                void(*callback[80])(skyBox *);
                int count;
                bool first;
                void init(const std::string &name, s32 cloud, s32 sky);
                bool process();
            };
            skyBox * sky_p , * sky_pb;
            skyBox sky_1,sky_2;
            class CloudShaderCallback:public irr::video::IShaderConstantSetCallBack{//shader回调
                public:
                    engine * parent;
                    void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
            }cloudShaderCallback;
            irr::s32 cloudTime;
    };
}
#endif
