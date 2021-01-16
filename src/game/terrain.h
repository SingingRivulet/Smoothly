#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include <map>
#include <set>
#include "building.h"
namespace smoothly{
    class terrain:public building{
        public:
            terrain();
            ~terrain();
            
            virtual void createChunk(int x,int y);
            virtual void showChunk(int x,int y);
            virtual void hideChunk(int x,int y);
            virtual void releaseChunk(int x,int y);
            virtual bool chunkShowing(int x,int y);
            virtual bool chunkLoaded(int x,int y)=0;
            void loop()override;
            virtual bool chunkCreated(int x,int y);
            bool selectByRay(//射线拾取
                    const irr::core::line3d<irr::f32>& ray,
                    vec3& outCollisionPoint,
                    irr::core::triangle3df& outTriangle,
                    irr::scene::ISceneNode*& outNode);
            int cm_cx,cm_cy;
            int getLodLevel(int x,int y);
            virtual int getVisualRange()=0;

            int getCollHeight(int x,int y);
        private:
            static irr::scene::IMesh * createTerrainMesh(irr::video::ITexture* texture,
                float * heightmap,
                int16_t * digmap,
                irr::u32 hx, irr::u32 hy,
                const irr::core::dimension2d<irr::f32>& stretchSize,
                const irr::core::dimension2d<irr::u32>& maxVtxBlockSize,  //网眼大小。官方文档没写
                u32 lod,
                bool debugBorders);
            //irrlicht自带的太差，所以自己实现一个
            float genTerrain(float * img,int x , int y ,int pointNum);
        public:
            float * genTerrain(int x , int y , float * mb = NULL){
                auto mapBuf = mb;
                if(mapBuf==NULL)
                    mapBuf = new float[33*33];
                genTerrain(mapBuf , x , y , 33);
                return mapBuf;
            }
        private:
            struct chunk{//各组件维护各自的chunk
                irr::scene::IMeshSceneNode  * node[4] , * shadowNode;
                
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                btCollisionShape * bodyShape;
                btTriangleMesh   * bodyMesh;

                float * mapBuf;
                int16_t * digMap;
                
                bodyInfo info;
                
                inline void show(){
                    for(int i=0;i<4;++i){
                        if(i+1 == lodLevel){
                            node[i]->setVisible(true);
                        }else{
                            node[i]->setVisible(false);
                        }
                    }
                }
                inline void hide(){
                    for(int i=0;i<4;++i){
                        node[i]->setVisible(false);
                    }
                }
                short lodLevel;
                inline void updateLOD(int x,int y,int cm_cx,int cm_cy){
                    int len = std::max(abs(x-cm_cx) , abs(y-cm_cy));
                    if(len<2){
                        lodLevel = 1;
                    }else
                    if(len<4){
                        lodLevel = 2;
                    }else
                    if(len<8){
                        lodLevel = 3;
                    }else{
                        lodLevel = 4;
                    }
                }

                int collMap[16][16];

                int x,y;

            };
            std::map<ipair,chunk*> chunks;
            chunk * genChunk(int x,int y);
            void freeChunk(chunk * );

            irr::s32 shaderv1;
            irr::s32 shaderv2;
            irr::s32 shaderv3;
            irr::s32 shaderv4;

            inline bool selectPointInChunk(//在chunk内进行拾取
                    int x,int y,
                    const irr::core::line3d<irr::f32>& ray,
                    vec3& outCollisionPoint,
                    irr::core::triangle3df& outTriangle,
                    irr::scene::ISceneNode*& outNode
                    ){
                auto it=chunks.find(ipair(x,y));
                if(it==chunks.end()){
                    return false;
                }else{
                    return scene->getSceneCollisionManager()->getCollisionPoint(
                                                    ray,
                                                    it->second->node[0]->getTriangleSelector(),
                                                    outCollisionPoint,
                                                    outTriangle,
                                                    outNode
                                                    );
                }
            }
            bool first;
            inline bool updateCamChunk(){
                if(camera==NULL)
                    return false;
                auto p = camera->getPosition();
                int cx = floor(p.X/32);
                int cy = floor(p.Z/32);
                if(cx!=cm_cx || cy!=cm_cy){
                    cm_cx = cx;
                    cm_cy = cy;
                    return true;
                }
                auto res = first;
                first = false;
                return res;
            }
            void initChunk(chunk * res);
            void updateChunk(chunk * p);
            int lastUCT;

            class TerrainShaderCallback:public irr::video::IShaderConstantSetCallBack{//shader回调
                public:
                    terrain * parent;
                    void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
            }terrainShaderCallback;

    };
}
#endif
