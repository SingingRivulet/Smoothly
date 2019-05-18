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
            struct boneAnimate{
                int id;
                int speed;
                int start;
                int end;
                bool loop;
            };
            struct boneAnimateGroup{
                boneAnimate body;
                std::list<boneAnimate> item;
                std::list<boneAnimate> pair;
            };
            class subsConf{
                public:
                    long id;
                    int life;
                    subsType type;
                    irr::scene::IAnimatedMesh  *  mesh;
                    btCollisionShape   *  bodyShape;
                    irr::video::ITexture* texture;
                    shapeGroup            shape;
                    bool                  useAlpha;
                    int                   hp;
                    float                 friction;//摩擦力
                    float                 restitution;//反弹
                    
                    bool walkInSky;
                    bool jumpInSky;
                    
                    std::string         bodyType;//rigid,character,vehicle , default is rigid
                    std::string         animationType;//static,character,vehicle
                    std::map<int,boneAnimateGroup> boneAnimation;
                    
                    std::map<int,std::pair<bool,int> > boneMapping;
                    
                    float characterWidth,characterHeight;
                    
                    float defaultSpeed;     //默认速度
                    float defaultLiftForce; //默认升力
                    float defaultPushForce; //默认推力
                    float defaultJumpImp;   //跳跃冲量
                    float deltaCamera;      //相机平移量
                    
                    bool noFallDown;        //设置不会倒
                    
                    bool haveHitSubsCallback;
                    bool hitSubsCallbackOnlyForMe;
                    int hitSubsCallback;
                    void onHitSubs(
                        lua_State * L,
                        void * self,
                        const std::string & uuid,
                        const std::string & sowner,
                        const std::string & tuuid,
                        const std::string & tsowner,
                        int imp
                    );
                    
                    bool haveHitBuildingCallback;
                    bool hitBuildingCallbackOnlyForMe;
                    int hitBuildingCallback;
                    void onHitBuildingCallback(
                        lua_State * L,
                        void * self,
                        const std::string & uuid,
                        const std::string & sowner,
                        const std::string & tuuid,
                        int imp
                    );
                    
                    bool haveHitTerrainItemCallback;
                    bool hitTerrainItemCallbackOnlyForMe;
                    int hitTerrainItemCallback;
                    void onHitTerrainItemCallback(
                        lua_State * L,
                        void * self,
                        const std::string & uuid,
                        const std::string & sowner,
                        const mapid & mapid,
                        int imp
                    );
                    
                    bool haveHitTerrainCallback;
                    bool hitTerrainCallbackOnlyForMe;
                    int hitTerrainCallback;
                    void onHitTerrainCallback(
                        lua_State * L,
                        void * self,
                        const std::string & uuid,
                        const std::string & sowner,
                        int imp
                    );
                    
                    inline subsConf():bodyType("rigid"),animationType("static"){
                        texture=NULL;
                        
                        type=SUBS_LASTING;
                        
                        hitSubsCallback=0;
                        hitBuildingCallback=0;
                        hitTerrainItemCallback=0;
                        hitTerrainCallback=0;
                        
                        walkInSky=false;
                        jumpInSky=false;
                        
                        haveHitSubsCallback=false;
                        haveHitBuildingCallback=false;
                        haveHitTerrainItemCallback=false;
                        haveHitTerrainCallback=false;
                        
                        hitSubsCallbackOnlyForMe=false;
                        hitBuildingCallbackOnlyForMe=false;
                        hitTerrainItemCallbackOnlyForMe=false;
                        hitTerrainCallbackOnlyForMe=false;
                        
                        defaultSpeed=0.5;       //默认速度
                        defaultLiftForce=0;     //默认升力
                        defaultPushForce=0;     //默认推力
                        defaultJumpImp=1;       //跳跃冲量
                        deltaCamera=0;
                    
                        noFallDown=false;       //设置不会倒
                        
                        characterHeight=1;
                        characterWidth=1;
                    }
                    
                    inline ~subsConf(){
                        
                    }
            };
            std::map<long,subsConf *> subsConfs;
            //////////////////////////////////////////////////////////////////////////////
            std::map<int,irr::scene::IAnimatedMesh*> animations;
            //////////////////////////////////////////////////////////////////////////////
            void init();
            void loadMesh();
            void destroy();
            //////////////////////////////////////////////////////////////////////////////
            int windowWidth;
            int windowHeight;
            float booster;
            mods(){
                windowWidth=1024;
                windowHeight=768;
                booster=1;
            }
    };
}
#endif
