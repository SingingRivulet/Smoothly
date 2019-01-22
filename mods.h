#ifndef SMOOTHLY_MODS
#define SMOOTHLY_MODS
#include <list>
#include <map>
#include "utils.h"
#include "perlin.h"
namespace smoothly{
    class mods{
        public:
            class mapGenerator;
            struct itemBase;
            //预先声明，防止报错
            
            typedef void(*mapGeneratorFunc)(int x,int y,float temp,float humi,float alti,mapGenerator * gen);
            
            struct itemBase{
                irr::scene::IMesh * mesh;
                
            };
            std::map<long,itemBase*> items;
            class mapGenerator{
                public:
                    irr::IrrlichtDevice * device;
                    irr::scene::ISceneManager * scene;//场景
                    irr::scene::IMeshSceneNode * node;
                    perlin3d * generator;
                    virtual void add(
                        int id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    )=0;
                    virtual float getRealHight(float x,float y)=0;
            };
            std::list<mapGeneratorFunc> mapGenFuncs;
    };
}
#endif
