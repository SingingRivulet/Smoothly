#ifndef SMOOTHLY_MODS
#define SMOOTHLY_MODS
#include <list>
#include "utils.h"
namespace smoothly{
    class mods{
        public:
            struct item{
                
            };
            
            class mapGenerator{
                public:
                    irr::IrrlichtDevice * device;
                    irr::scene::ISceneManager * scene;//场景
                    irr::scene::IMeshSceneNode * node;
                    virtual void add(
                        int id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    )=0;
            };
            typedef void(*mapGeneratorFunc)(int x,int y,int temp,int humi,float alti,mapGenerator * gen);
            std::list<mapGeneratorFunc> mapGenFuncs;
    };
}
#endif
