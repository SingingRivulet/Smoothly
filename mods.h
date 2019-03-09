#ifndef SMOOTHLY_MODS
#define SMOOTHLY_MODS
#include <list>
#include <map>
#include "utils.h"
#include "perlin.h"
#include "physical.h"
namespace smoothly{
    class mods:public physical{
        public:
            lua_State * L;
            void scriptInit(const char * path);
            void scriptDestroy();
            /////////////////////////////////////////////////////////////////////////////
            class mapGenerator;
            struct itemBase;
            //预先声明，防止报错
            enum phyObjMethod{
                USE_IRRMOD,
                USE_BULLET,
                NONE
            };
            typedef void(*mapGeneratorFunc)(int x,int y,float temp,float humi,float alti,mapGenerator * gen);
            /////////////////////////////////////////////////////////////////////////////
            struct itemBase{
                irr::scene::IMesh * mesh;
                
            };
            struct itemConfig:itemBase{
                bool haveBB;
                irr::core::aabbox3d<float> BB;
                btCollisionShape * bodyShape;
            };
            std::map<long,itemConfig*> items;
            /////////////////////////////////////////////////////////////////////////////
            class mapGenerator{
                public:
                    irr::IrrlichtDevice * device;
                    irr::scene::ISceneManager * scene;//场景
                    irr::scene::IMeshSceneNode * node;
                    perlin3d * generator;
                    virtual int add(
                        long id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    )=0;
                    virtual float getRealHight(float x,float y)=0;
                    void autoGen(int x,int y,int tem,int hu,float h,mods * mod);
            };
            std::list<mapGeneratorFunc> mapGenFuncs;
            
            std::map<long,int> terrainItemNum;
            
            void getGenList(int x,int y,int tem,int hu,float h,std::map<long,float> & pl);
            struct autoMapGen{
                int luaFunc;
                void getGenList(lua_State * L,int x,int y,int tem,int hu,float h,std::map<long,float> & pl);
            };
            std::list<autoMapGen> autoMapGenList;
            /////////////////////////////////////////////////////////////////////////////
            class buildingBase{
                public:
                    long type;
                    int linkNum;
            };
            class building:public buildingBase{
                public:
                    irr::scene::IMesh * mesh;
                    btCollisionShape  * bodyShape;
                    btTriangleMesh    * bodyMesh;
                    irr::core::aabbox3d<float> BB;
            };
            std::map<long,building*> buildings;
            
            irr::scene::ISceneManager * scene;
            
            long defaultBuildingList[7];
            
            void init(const char * path);
            void loadMesh();
            void destroy();
    };
}
#endif
