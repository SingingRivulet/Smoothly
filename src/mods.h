#ifndef SMOOTHLY_MODS
#define SMOOTHLY_MODS
#include <list>
#include <map>
#include "utils.h"
#include "perlin.h"
#include "physical.h"
namespace smoothly{
    int getCharAnimationId(int foot,int hand,int range,int move);
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
                irr::core::vector3df deltaPosition;
                irr::core::vector3df scale;
                
                bool                 haveBody;
                shapeGroup           shape;
                
                irr::scene::IMesh * meshv2;
                irr::scene::IMesh * meshv3;
                irr::scene::IMesh * meshv4;
                irr::video::ITexture* texture;
                bool                useAlpha;
                itemConfig():deltaPosition(0,0,0),scale(1,1,1){
                    mesh=NULL;
                    meshv2=NULL;
                    meshv3=NULL;
                    meshv4=NULL;
                    texture=NULL;
                    haveBody=false;
                    useAlpha=false;
                }
                inline ~itemConfig(){
                    //if(mesh)     mesh->drop();
                    //if(meshv2)   meshv2->drop();
                    //if(meshv3)   meshv3->drop();
                    //if(meshv4)   meshv4->drop();
                    //irr可以自动回收
                    if(haveBody){
                        shape.release();
                        haveBody=false;
                    }
                }
            };
            std::map<long,itemConfig*> items;
            /////////////////////////////////////////////////////////////////////////////
            class mapGenerator{
                public:
                    irr::IrrlichtDevice         * device;
                    irr::scene::ISceneManager   * scene;//场景
                    irr::scene::IMeshSceneNode  * node;
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
                    irr::scene::IMesh    * mesh;
                    irr::video::ITexture * texture;
                    
                    btCollisionShape  * bodyShape;
                    //btTriangleMesh    * bodyMesh;
                    shapeGroup           shape;
                    
                    bool                useAlpha;
                    float               friction;
                    float               restitution;
                    
                    /*
                    table callback({
                      x,y,z,//瞄准坐标
                      x,y,z,//瞄准物体的坐标
                      x,y,z,//瞄准物体的旋转角度
                      x,y,z,//摄像头旋转角度
                      aimId   //瞄准物体的id
                    })
                     * return:
                     *   nil:不放置物体
                     * 
                     *   {
                     *     x,y,z,     //放置位置
                     *     ax,ay,az   //放置角度
                     *   }
                     */
                    int         onAimAtBuilding;
                    bool        haveOnAimAtBuilding;
                    bool (*onAimAtBuildingCFunc)(
                        const irr::core::vector3df & aimAt,
                        const irr::core::vector3df & trg,
                        const irr::core::vector3df & trgRot,
                        const irr::core::vector3df & camRot,
                        int aimId,
                        irr::core::vector3df & outPosi,
                        irr::core::vector3df & outRot
                    );
                    
                    bool callOnAimAtBuilding(
                        lua_State * L,
                        const irr::core::vector3df & aimAt,
                        const irr::core::vector3df & trg,
                        const irr::core::vector3df & trgRot,
                        const irr::core::vector3df & camRot,
                        int aimId,
                        irr::core::vector3df & outPosi,
                        irr::core::vector3df & outRot
                    ){
                        if(!haveOnAimAtBuilding)
                            return false;
                        
                        if(onAimAtBuildingCFunc){
                            return onAimAtBuildingCFunc(aimAt,trg,trgRot,camRot,aimId,outPosi,outRot);
                        }else{
                            
                            lua_settop(L,0);
                            lua_rawgeti(L,LUA_REGISTRYINDEX,onAimAtBuilding);
                            if(!lua_isfunction(L,-1)){
                                lua_pop(L,1);
                                return false;
                            }
                            
                            lua_newtable(L);
                            
                                lua_newtable(L);
                                lua_pushnumber(L,aimAt.X);
                                lua_rawseti(L, -2, 1);
                                lua_pushnumber(L,aimAt.Y);
                                lua_rawseti(L, -2, 2);
                                lua_pushnumber(L,aimAt.Z);
                                lua_rawseti(L, -2, 3);
                            
                            lua_rawseti(L, -2, 1);
                            
                                lua_newtable(L);
                                lua_pushnumber(L,trg.X);
                                lua_rawseti(L, -2, 1);
                                lua_pushnumber(L,trg.Y);
                                lua_rawseti(L, -2, 2);
                                lua_pushnumber(L,trg.Z);
                                lua_rawseti(L, -2, 3);
                            
                            lua_rawseti(L, -2, 2);
                            
                                lua_newtable(L);
                                lua_pushnumber(L,trgRot.X);
                                lua_rawseti(L, -2, 1);
                                lua_pushnumber(L,trgRot.Y);
                                lua_rawseti(L, -2, 2);
                                lua_pushnumber(L,trgRot.Z);
                                lua_rawseti(L, -2, 3);
                            
                            lua_rawseti(L, -2, 3);
                            
                                lua_newtable(L);
                                lua_pushnumber(L,camRot.X);
                                lua_rawseti(L, -2, 1);
                                lua_pushnumber(L,camRot.Y);
                                lua_rawseti(L, -2, 2);
                                lua_pushnumber(L,camRot.Z);
                                lua_rawseti(L, -2, 3);
                            
                            lua_rawseti(L, -2, 4);
                            
                            lua_pushinteger(L,aimId);
                            lua_rawseti(L, -2, 5);
                            
                            if(lua_pcall(L, 1, 1, 0) != 0){
                                printf("[callOnAimAtBuilding]error %s\n", lua_tostring(L,-1));
                                return false;
                            }else{
                                if(lua_istable(L,-1)){
                                    
                                    if(luaL_len(L,-1)<6)
                                        return false;
                                    
                                    lua_rawgeti(L,-1,1);
                                    outPosi.X=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    lua_rawgeti(L,-1,2);
                                    outPosi.Y=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    lua_rawgeti(L,-1,3);
                                    outPosi.Z=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    lua_rawgeti(L,-1,4);
                                    outRot.X=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    lua_rawgeti(L,-1,5);
                                    outRot.Y=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    lua_rawgeti(L,-1,6);
                                    outRot.Z=lua_tonumber(L,-1);
                                    lua_pop(L,1);
                                    
                                    return true;
                                }else
                                    return false;
                            }
                            
                        }
                        return false;
                    }
                    
                    irr::core::aabbox3d<float> BB;
                    
                    inline building(
                        irr::scene::IMesh * m,
                        irr::video::ITexture* tu,
                        const std::string & shape,
                        bool ualpha,
                        float f,
                        float r
                    ){
                        init(m,tu,shape,ualpha,f,r);
                    }
                    
                    inline building(
                        irr::scene::ISceneManager * scene,
                        const char * mpath,
                        const char * tpath,
                        const std::string & shape,
                        bool ualpha,
                        float f,
                        float r
                    ){
                        init(
                            scene->getMesh(mpath),
                            scene->getVideoDriver()->getTexture(tpath),
                            shape,
                            ualpha,
                            f,r
                        );
                    }
                    
                    inline void init(
                        irr::scene::IMesh * m,
                        irr::video::ITexture* tu,
                        const std::string & shape,
                        bool ualpha,
                        float f,
                        float r
                    ){
                        this->mesh      = m;
                        BB              = this->mesh->getBoundingBox();
                        this->texture   = tu;
                        this->shape.init(shape);
                        this->bodyShape = this->shape.compound;
                        useAlpha        = ualpha;
                        friction        = f;
                        restitution     = r;
                        haveOnAimAtBuilding = false;
                        onAimAtBuildingCFunc=NULL;
                    }
                    inline ~building(){
                        if(mesh)        mesh->drop();
                        //if(it.second->texture)   it.second->texture->drop();
                        //if(bodyShape)   delete bodyShape;
                        //if(bodyMesh)    delete bodyMesh;
                        
                        //if(texture)     texture->drop();
                        
                        shape.release();
                        this->bodyShape=NULL;
                    }
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
                    
                    int rideMode;//0:不允许，1:允许乘坐，2:骑跨
                    bool walkInSky;
                    bool jumpInSky;
                    
                    std::string         bodyType;//rigid,character,vehicle , default is rigid
                    std::string         animationType;//static,character,vehicle
                    std::map<int,boneAnimateGroup> boneAnimation;
                    
                    std::map<int,std::pair<bool,int> > boneMapping;
                    
                    float characterWidth,characterHeight;
                    
                    //std::vector<int> hand;//持武器的骨骼id
                    //已移除。直接用boneMapping定义
                    
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
                        if(mesh)    mesh->drop();
                        if(texture) texture->drop();
                        shape.release();
                    }
            };
            std::map<long,subsConf *> subsConfs;
            //////////////////////////////////////////////////////////////////////////////
            struct animationConf{
                irr::scene::IAnimatedMesh   * mesh;
                irr::video::ITexture        * texture;
                bool useAlpha;
                int  attackActivity;
                ~animationConf(){
                    if(mesh)
                        mesh->drop();
                    if(texture)
                        texture->drop();
                }
                inline irr::scene::IAnimatedMeshSceneNode * create(irr::scene::ISceneManager * scene,irr::scene::ISceneNode * par=NULL){
                    auto p=scene->addAnimatedMeshSceneNode(mesh,par);
                    if(texture){
                        p->setMaterialTexture( 0 , texture);
                        if(useAlpha){
                            p->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
                        }
                    }
                    return p;
                }
            };
            
            std::map<int,animationConf*> animations;
            //////////////////////////////////////////////////////////////////////////////
            struct attackConf{
                WeaponType  type;
                
                //激光 近战 爆炸使用range，子弹使用lifeTime
                float       range;
                int         lifeTime;
                
                const btConvexShape * castShape;
                
                int         bulletNum;//子弹数量
                float       scatter;//发散
                float       impulse;//动量
                btVector3   rel_pos;//动量施加点
                int         bulletSubsId;//子弹的substance id
                
                int  onAttackTerrainItemLuaFunc;
                bool haveOnAttackTerrainItem;
                
                int  onAttackBuildingLuaFunc;
                bool haveOnAttackBuilding;
                
                int  onAttackSubstanceLuaFunc;
                bool haveOnAttackSubstance;
                
                void onAttackTerrainItem(void * substance , const std::string & uuid,const mapid & mid);
                void onAttackBuilding(void * substance , const std::string & uuid , const std::string & tuuid);
                void onAttackSubstance(void * substance , const std::string & uuid , const std::string & tuuid);
            };
            std::map<int,attackConf*> attackings;
            //////////////////////////////////////////////////////////////////////////////
            class attackLaunchConf{
                public:
                    struct activity{
                        int handIndex;
                        int attackingId;
                    };
                    std::list<activity> mapping;//对应subsConf::hand的下标
            };
            std::map<int,attackLaunchConf*> attackLaunchMapping;
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
