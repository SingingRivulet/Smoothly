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
            bool chunkCreated(int x,int y);
            bool selectByRay(//射线拾取
                    const irr::core::line3d<irr::f32>& ray,
                    irr::core::vector3df& outCollisionPoint,
                    irr::core::triangle3df& outTriangle,
                    irr::scene::ISceneNode*& outNode);
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
            
            std::set<ipair> showing;

            irr::s32 shader;

            inline bool selectPointInChunk(//在chunk内进行拾取
                    int x,int y,
                    const irr::core::line3d<irr::f32>& ray,
                    irr::core::vector3df& outCollisionPoint,
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
    };
}
#endif
