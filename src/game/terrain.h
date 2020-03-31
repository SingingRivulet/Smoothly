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
        private:
            float ** mapBuf;
            static irr::scene::IMesh * createTerrainMesh(
                irr::video::ITexture* texture,
                float ** heightmap,
                irr::u32 hx,irr::u32 hy,
                const irr::core::dimension2d<irr::f32>& stretchSize,
                const irr::core::dimension2d<irr::u32>& maxVtxBlockSize, //网眼大小。官方文档没写
                bool debugBorders);
            //irrlicht自带的太差，所以自己实现一个
            float genTerrain(float ** img,int x , int y ,int pointNum);
            
            struct chunk{//各组件维护各自的chunk
                irr::scene::IMeshSceneNode  * node;
                
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                btCollisionShape * bodyShape;
                btTriangleMesh   * bodyMesh;
                
                bodyInfo info;
                
                inline void show(){
                    node->setVisible(true);
                }
                inline void hide(){
                    node->setVisible(false);
                }
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
                                                    it->second->node->getTriangleSelector(),
                                                    outCollisionPoint,
                                                    outTriangle,
                                                    outNode
                                                    );
                }
            }
            inline irr::s32 getShader(int x,int y,int & lv){
                int len = std::max(abs(x-cm_cx) , abs(y-cm_cy));
                if(len<2){
                    lv = 1;
                    return shaderv1;
                }else
                if(len<4){
                    lv = 2;
                    return shaderv2;
                }else
                if(len<8){
                    lv = 3;
                    return shaderv3;
                }else{
                    lv = 4;
                    return shaderv4;
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
            int lastUCT;
    };
}
#endif
