#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include <map>
#include <set>
#include "engine.h"
#include "../libclient/terrain.h"
#include "../libclient/terrainDispather.h"
namespace smoothly{
    typedef smoothly::world::terrain::ipair ipair;
    class terrain:public engine{
        public:
            terrain();
            ~terrain();
            
            void createChunk(int x,int y);
            void showChunk(int x,int y);
            void hideChunk(int x,int y);
            void releaseChunk(int x,int y);
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
    };
}
#endif