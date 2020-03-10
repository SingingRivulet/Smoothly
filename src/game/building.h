#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING
#include <unordered_map>
#include <set>

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

        void msg_addBuilding(const char * uuid, int id, float px, float py, float pz, float rx, float ry, float rz)override;
        void msg_removeBuilding(const char * uuid)override;
        void msg_startChunk(int x, int y)override;

    private:
        struct buildingBody;
        struct buildingChunk{
            bool started;
            ipair position;
            std::set<buildingBody*> bodies;
            inline buildingChunk(int x,int y):position(x,y){
                started = false;
            }
        };
        struct buildingBody{
            std::string uuid;
            buildingChunk * inchunk;
            irr::scene::IAnimatedMeshSceneNode * node;
            btRigidBody      * rigidBody;
            btMotionState    * bodyState;
            bodyInfo info;
        };
        std::map<ipair,buildingChunk*> buildingChunks;
        std::unordered_map<std::string,buildingBody*> bodies;

        void releaseBuilding(buildingBody*);
        void removeBuilding(buildingBody*);

        buildingBody * createBuilding(const vec3 & p,const vec3 & r,int id,const std::string & uuid);

        buildingChunk * seekChunk(int x,int y);

        struct conf{

            irr::s32                shader;
            bool                    haveShader;

            irr::scene::IAnimatedMesh *   mesh;

            shapeGroup              bodyShape;
            bool                    haveBody;
            conf(){
                haveBody = false;
                haveShader = false;
                mesh = NULL;
            }
        };
        std::map<int,conf*> config;

        void loadConfig();
        void releaseConfig();
};

}//smoothly

#endif // SMOOTHLY_BUILDING
