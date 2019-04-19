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
            void loadConfig();
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
                btTriangleMesh   * bodyMesh;
                irr::scene::IMesh * meshv2;
                irr::scene::IMesh * meshv3;
                irr::scene::IMesh * meshv4;
                irr::video::ITexture* texture;
                bool                useAlpha;
                itemConfig(){
                    mesh=NULL;
                    meshv2=NULL;
                    meshv3=NULL;
                    meshv4=NULL;
                    texture=NULL;
                    bodyMesh=NULL;
                    bodyShape=NULL;
                    useAlpha=false;
                }
                inline void destroy(){
                    //if(mesh)     mesh->drop();
                    //if(meshv2)   meshv2->drop();
                    //if(meshv3)   meshv3->drop();
                    //if(meshv4)   meshv4->drop();
                    //irr可以自动回收
                    if(bodyMesh) delete bodyMesh;
                    if(bodyShape)delete bodyShape;
                }
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
                    bool                useAlpha;
                    float               friction;
                    float               restitution;
                    irr::core::aabbox3d<float> BB;
            };
            std::map<long,building*> buildings;
            
            irr::scene::ISceneManager * scene;
            
            long defaultBuildingList[7];
            //////////////////////////////////////////////////////////////////////////////
            enum subsType{
                SUBS_LASTING,
                SUBS_BRIEF
            };
            class subsConf{
                public:
                    long id;
                    float mass;
                    btVector3 localInertia;
                    int life;
                    subsType type;
                    irr::scene::IMesh  *  mesh;
                    btCollisionShape   *  bodyShape;
                    irr::video::ITexture* texture;
                    shapeGroup            shape;
                    bool                  useAlpha;
                    int                   hp;
                    float                 friction;
                    float                 restitution;
                    
                    bool haveHitSubsCallback;
                    int hitSubsCallback;
                    
                    bool haveHitBuildingCallback;
                    int hitBuildingCallback;
                    
                    bool haveHitTerrainItemCallback;
                    int hitTerrainItemCallback;
                    
                    bool haveHitTerrainCallback;
                    int hitTerrainCallback;
                    
                    inline subsConf(){
                        hitSubsCallback=0;
                        hitBuildingCallback=0;
                        hitTerrainItemCallback=0;
                        hitTerrainCallback=0;
                        
                        haveHitSubsCallback=false;
                        haveHitBuildingCallback=false;
                        haveHitTerrainItemCallback=false;
                        haveHitTerrainCallback=false;
                        
                    }
                    
                    inline ~subsConf(){
                        
                    }
            };
            std::map<long,subsConf *> subsConfs;
            //////////////////////////////////////////////////////////////////////////////
            void init(const char * path);
            void loadMesh();
            void destroy();
    };
}
#endif
