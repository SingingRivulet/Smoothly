#ifndef SMOOTHLY_TERRAIN_ITEM
#define SMOOTHLY_TERRAIN_ITEM
#include "terrain.h"
#include "../utils/cJSON.h"
#include <map>
namespace smoothly{
    typedef client::mapItem mapItem;
    class terrain_item:public terrain{
        public:
            terrain_item();
            ~terrain_item();
            void setRemoveTable(int x,int y,const std::set<mapItem> & rmt);
            void removeTerrainItem(int x , int y ,int index,int id);
            void releaseTerrainItems(int x , int y);
            struct mapId{
                int x,y;
                mapItem id;
            };
            virtual void msg_addRemovedItem(int x,int y,int,int);
            virtual void msg_setRemovedItem(int x,int y,const std::set<mapItem> &);
            virtual void releaseChunk(int x,int y);
        private:
            struct chunk;
            struct item{
                irr::scene::ISceneNode  * node[4];
                mapId id;
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                bodyInfo info;
                chunk * parent;
                inline item(){
                    rigidBody = NULL;
                    bodyState = NULL;
                    for(int i = 0;i<4;++i)
                        node[i] = NULL;
                }
            };
            struct chunk{
                int x,y;
                std::map<mapItem,item*> children;
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
            struct conf{
                bool                 haveBody;
                shapeGroup           shape;
                irr::scene::IMesh *  mesh[4];
                float                deltaHeight;
                vec3 scale;
                irr::s32             shader;
                bool                 useShader;
            };
            std::map<int,conf*> config;
            
            void loadConfig();
            void loadJSON(cJSON * json);
            void releaseConfig();
            irr::scene::ISceneNode * genTree(int seed);

            irr::video::ITexture * texture_treeTrunk,* texture_treeGrass;
            irr::u32 shader_tree;
    };
}
#endif
