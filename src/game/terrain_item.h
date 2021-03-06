﻿#ifndef SMOOTHLY_TERRAIN_ITEM
#define SMOOTHLY_TERRAIN_ITEM
#include "terrain.h"
#include "../utils/cJSON.h"
#include <map>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
namespace smoothly{
    typedef client::mapItem mapItem;
    class terrain_item:public terrain{
        public:
            terrain_item();
            ~terrain_item();
            bool setRemoveTable(int x,int y,const std::set<mapItem> & rmt,bool cache=true);
            void pushRemoveTable(int x,int y,const std::set<mapItem> & rmt);
            void removeTerrainItem(int x , int y ,int index,int id);
            void releaseTerrainItems(int x , int y);
            struct mapId{
                int cx,cy;
                float x,y;
                mapItem id;
            };
            void msg_addRemovedItem(int x,int y,int,int)override;
            void msg_setRemovedItem(int x,int y,const std::set<mapItem> &)override;
            void msg_chunkACL(int32_t x,int32_t y,bool b,bool c,bool t,const char * owner)override;
            virtual void releaseChunk(int x,int y);

            void loop()override;
            void onDraw()override;
            bool chunkCreated(int x,int y)override;
            void occupy(){
                auto posi = camera->getPosition();
                int cx = floor(posi.X / 32);
                int cy = floor(posi.Z / 32);
                occupy(cx,cy);
            }
            void occupy(int x,int y);
        private:
            std::map<ipair,std::set<mapItem> > rmtCache;
            float * mapBuffer;
            struct chunk;
            struct item{
                irr::scene::ISceneNode  * node[4];
                int lodLevel;
                irr::scene::ISceneNode  * shadowNode;
                mapId id;
                vec3 position;
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                btTriangleMesh   * bodyMesh;
                btCollisionShape * bodyShape;
                bodyInfo info;
                chunk * parent;
                int hideLodLevel;
                bool useHiZ;
                inline item(){
                    rigidBody = NULL;
                    bodyState = NULL;
                    bodyMesh  = NULL;
                    bodyShape = NULL;
                    hideLodLevel = 5;
                    shadowNode = NULL;
                    for(int i = 0;i<4;++i)
                        node[i] = NULL;
                    lodLevel = -1;
                    useHiZ = false;
                }
                void updateVisible();
            };
            struct chunk{
                int x,y;
                terrain_item * parent;
                chunk *nearx0,*nearx1,*neary0,*neary1;//所有chunk构成一张二维链表，方便查询
                irr::scene::ISceneNode * minimap_element;
                std::string owner;
                bool allowBuilding,allowColl,allowAttack;
                std::map<mapItem,item*> children;
                int lodLevel;
                void updateVisible();
                void unlink();
            };
            std::map<ipair,chunk*> chunks;
            void releaseChunk(chunk*);
            void releaseChildren(chunk*);
            void releaseTerrainItem(item*);
            void releaseAllChunk();
            void removeTerrainItem(chunk * ,int index,int id);
            item * makeTerrainItem(int, int, float, float, float);

            virtual void updateLOD(int x,int y,int lv)override;
        private:
            void linkChunk(chunk * , int x,int y);
            struct conf{
                bool                 haveBody;
                shapeGroup           shape;
                irr::scene::IMesh *  mesh[4];
                float                deltaHeight;
                vec3 scale;
                irr::s32             shader;
                bool                 useShader;
                bool                 flat;
            };
            std::map<int,conf*> config;
            
            void loadConfig();
            void loadJSON(cJSON * json);
            void releaseConfig();
            irr::scene::ISceneNode * genTree(int seed, scene::SMesh *& shadowMesh1,scene::SMesh *& shadowMesh2, btTriangleMesh *& bodyMesh);
            irr::scene::ISceneNode * genGrass(int seed, int & lodLevel,irr::video::ITexture *& tex);
            void genGrassMesh();

            irr::video::ITexture * texture_treeTrunk,* texture_treeLeaves;
            irr::u32 shader_tree;
            std::vector<irr::video::ITexture*> texture_grass;
            irr::scene::IMesh * mesh_grass;

            struct rmt{
                int x,y;
                std::set<mapItem> r;
                rmt():r(){
                    x=0;
                    y=0;
                }
                rmt(const rmt & i){
                    x = i.x;
                    y = i.y;
                    r = i.r;
                }
            };
            std::queue<rmt> rmtQueue;

            int chunkLeft;
            irr::gui::IGUIStaticText * chunkLeft_text;

            irr::scene::IMesh * minimap_terrain_mesh[2][2][2] , * minimap_terrain_mesh_owned;
            irr::s32 minimap_terrain_shader;

            std::wstring showingText;
            time_t showText_time;

            std::vector<ipair> flatCells;

            virtual void occupyChunk()=0;
        public:
            virtual void createChunk(int x,int y)override;
            inline void setTerrainMapMessage(const std::wstring & w){
                showingText = w;
                showText_time = time(0);
            }
            void uploadChunkACL();
            void setFullMapMode(bool m)override;
            irr::gui::IGUICheckBox * terrmapacl_1,* terrmapacl_2,* terrmapacl_3;
            irr::gui::IGUIButton * terrmapacl_save;

            bool isMyChunk(int x,int y);

        private://hiz多线程

            std::mutex hiz_solve_mtx;
            std::condition_variable hiz_solve_cv;
            void hiz_solve_wait(){
                std::unique_lock<std::mutex> lck(hiz_solve_mtx);
                hiz_solve_cv.wait(lck);
            }
            void hiz_solve_wake(){
                std::unique_lock<std::mutex> lck(hiz_solve_mtx);
                hiz_solve_cv.notify_all();
            }

            std::atomic<int> hiz_num;

            std::mutex hiz_queue_lock;
            std::queue<item*> hiz_queue;

            void hiz_solve_mainThread();

    };
}
#endif
