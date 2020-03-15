﻿#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include <math.h>

#include "engine.h"
#include "../libclient/terrain.h"
#include "../libclient/terrainDispather.h"
#include "../utils/cJSON.h"
#include "../utils/dbvt3d.h"

namespace smoothly{

typedef irr::core::vector3df vec3;
typedef smoothly::world::terrain::ipair ipair;

class building:public engine{
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

    private:
        lua_State * L;
        struct buildingBody;
        struct buildingChunk{
            bool started;
            ipair position;
            std::set<buildingBody*> bodies;
            inline buildingChunk(int x,int y):position(x,y){
                started = false;
            }
        };
        struct conf;
        struct buildingBody{
            std::string uuid;
            buildingChunk * inchunk;
            irr::scene::IAnimatedMeshSceneNode * node;
            btRigidBody      * rigidBody;
            btMotionState    * bodyState;
            conf             * config;
            bodyInfo info;
            dbvt3d::AABB     * bb;
        };
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

            irr::scene::IAnimatedMesh *   mesh;

            shapeGroup              bodyShape;
            bool                    haveBody;

            struct{
                float deltaHei;
                float deltaHor;
            }autoAttach;
            bool useAutoAttach;

            bool                          canBuildOn;
            irr::core::aabbox3d<irr::f32> fetchBB;//放置建筑时用于拾取周围建筑的包围盒

            int                           attachHandler;
            bool                          useAttachHandler;

            conf(){
                haveBody             = false;
                haveShader           = false;
                mesh = NULL;
                useAutoAttach        = false;
                canBuildOn           = false;
                useAttachHandler     = false;
                attachHandler        = 0;
                autoAttach.deltaHei  = 0;
                autoAttach.deltaHor  = 1;
            }
        };
        std::map<int,conf*> config;

        void loadConfig();
        void releaseConfig();

        void fetchByRay(
                const vec3 & from ,
                const vec3 & to ,
                std::function<void(const vec3 &,bodyInfo*)> callback ,
                std::function<bool(physical::bodyInfo *)> filter=[](physical::bodyInfo * b){
            //默认忽略角色控制器
            return b->type!=BODY_BODY;
        });
        dbvt3d dbvt;

    public:
        void buildingStart();           //开始建筑模式
        void buildingUpdate();          //更新预览建筑
        void buildingApply();           //应用被预览的建筑
        void buildingEnd();             //建筑模式结束
    private:
        irr::scene::ISceneNode  * buildingPrev;
        conf                    * buildingPrevConf;
        int                       buildingPrevId;
        bool                      buildingAllowBuild;
        buildingBody            * buildingTarget;
};

}//smoothly

#endif // SMOOTHLY_BUILDING