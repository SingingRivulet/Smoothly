#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <functional>
#include <math.h>

#include "weather.h"
#include "../libclient/terrain.h"
#include "../libclient/terrainDispather.h"
#include "../utils/cJSON.h"
#include "../utils/dbvt3d.h"

namespace smoothly{

typedef irr::core::vector3df vec3;
typedef smoothly::world::terrain::ipair ipair;

class building:public weather{
    public:
        building();
        ~building();
        bool buildingChunkCreated(int x,int y)const;    //检查chunk被创建
        void buildingChunkStart(int x,int y);           //启动chunk，添加chunk被添加的标记，由服务器端调用
        void buildingChunkFetch(int x,int y);           //下载chunk，由子类调用
        void buildingAdd(const vec3 & p,const vec3 & r,int id,const std::string & uuid);
        void buildingChunkRelease(int x, int y);

        virtual void loop();

        void msg_addBuilding(const char * uuid, int id, float px, float py, float pz, float rx, float ry, float rz)override;
        void msg_removeBuilding(const char * uuid)override;
        void msg_startChunk(int x, int y)override;

        virtual void updateLOD(int x,int y,int lv);

        void onDraw()override;

        struct ghostSearch{
                btBoxShape shape;
                btPairCachingGhostObject ghost;
                building * parent;
                inline ghostSearch(const btVector3 & v,building * p):shape(v){
                    ghost.setCollisionShape(&shape);
                    parent = p;
                }
                void search(std::function<void (physical::bodyInfo *)> callback);
        };
        void ghostTest(const btVector3 & pos, float len,std::function<void (physical::bodyInfo *)> callback);

        bool getCollMap(int x,int y,int z);

        struct buildingBody;
    private:
        void addDefaultBuilding();
        lua_State * L;
        struct buildingChunk{
            bool started;
            ipair position;
            std::set<buildingBody*> bodies;
            buildingChunk *nearx0,*nearx1,*neary0,*neary1;//所有chunk构成一张二维链表，方便查询
            inline buildingChunk(int x,int y):position(x,y){
                started = false;
            }
            void unlink();
        };
        void linkChunk(buildingChunk * , int x,int y);
        struct conf;
    public:
        struct blockPosition{//点在chunk中的位置
            int x,y,z;
            inline blockPosition() {
                x=0;
                y=0;
                z=0;
            }
            inline blockPosition(const blockPosition & i) {
                x=i.x;
                y=i.y;
                z=i.z;
            }
            inline blockPosition(int ix,int iy,int iz) {
                x=ix;
                y=iy;
                z=iz;
            }
            inline const blockPosition & operator=(const blockPosition & i) {
                x=i.x;
                y=i.y;
                z=i.z;
                return * this;
            }
            inline bool operator<(const blockPosition & i)const{
                if(x<i.x)
                    return true;
                else
                if(x==i.x){
                    if(y<i.y)
                        return true;
                    else
                    if(y==i.y){
                        if(z<i.z){
                            return true;
                        }
                    }
                }
                    return false;
            }
            inline bool operator==(const blockPosition & i)const{
                return (x==i.x) && (y==i.y) && (z==i.z);
            }
        };
    private:
        std::map<blockPosition,bool> collTable;//用于寻路的碰撞表
        std::queue<blockPosition>    checkingRemoveList;
        void processCheckingRemove();
    public:
        struct buildingBody{
            std::string uuid;
            buildingChunk * inchunk;
            irr::scene::IMeshSceneNode * node[4];
            btRigidBody      * rigidBody;
            btMotionState    * bodyState;
            conf             * config;
            bodyInfo info;
            dbvt3d::AABB     * bb;
            building         * parent;
            inline buildingBody(){
                for(int i = 0;i<4;++i)
                    node[i] = NULL;
            }
            void initCollTable(bool removeMode = false);
            void buildAffectArea(std::queue<blockPosition> & t);
        };
    private:
        std::map<ipair,buildingChunk*> buildingChunks;
        std::unordered_map<std::string,buildingBody*> bodies;

        void releaseBuilding(buildingBody*);
        void removeBuilding(buildingBody*);

        buildingBody * createBuilding(const vec3 & p,const vec3 & r,int id,const std::string & uuid);

        buildingChunk * seekChunk(int x,int y);

        struct conf{
            int                     id;

            irr::s32                shader;
            bool                    haveShader;

            irr::scene::IMesh     * mesh[4];

            irr::video::ITexture  * texture;

            irr::video::ITexture  * desTexture;

            shapeGroup              bodyShape;
            bool                    haveBody;

            btVector3               pathTarget;//寻路时作为目标点
            bool                    canWalk;

            struct{
                float deltaHei;
                float deltaHor;
                bool allowed[4];
            }autoAttach;
            bool useAutoAttach;

            bool                          canBuildOn;
            irr::core::aabbox3d<irr::f32> fetchBB;//放置建筑及搜索建筑时用于拾取周围建筑的包围盒

            int                           attachHandler;
            bool                          useAttachHandler;

            conf(){
                haveBody             = false;
                haveShader           = false;
                for(int i = 0;i<4;++i)
                    mesh[i] = NULL;
                useAutoAttach        = false;
                canBuildOn           = false;
                useAttachHandler     = false;
                attachHandler        = 0;
                autoAttach.deltaHei  = 0;
                autoAttach.deltaHor  = 1;
                texture              = NULL;
                desTexture           = NULL;
                for(int i=0;i<4;++i){
                    autoAttach.allowed[i]=true;
                }
            }
        };
        std::map<int,conf*> config;

        void loadConfig();
        void releaseConfig();

        dbvt3d dbvt;

    public:
        void fetchByRay(
                const vec3 & from ,
                const vec3 & to ,
                std::function<void(const vec3 &,bodyInfo*)> callback ,
                std::function<bool(physical::bodyInfo *)> filter=[](physical::bodyInfo * b){
            //默认忽略角色控制器
            return b->type!=BODY_BODY;
        });
        void buildingStart();           //开始建筑模式
        void buildingUpdate();          //更新预览建筑
        void buildingApply();           //应用被预览的建筑
        void buildingEnd(bool apply=true);             //建筑模式结束
        void switchBuilding();
        std::vector<int>          availableBuilding;
        virtual void cancle();
    private:
        irr::scene::ISceneNode  * buildingPrev;
        conf                    * buildingPrevConf;
        int                       buildingPrevId;
        bool                      buildingAllowBuild;
        buildingBody            * buildingTarget;

        clock_t                   showningDes;

        irr::u32 def_shader;

        int buildingSelect;

};

}//smoothly

#endif // SMOOTHLY_BUILDING
