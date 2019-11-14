#ifndef SMOOTHLY_TERRAIN_ITEM
#define SMOOTHLY_TERRAIN_ITEM
#include "terrain.h"
#include "cJSON.h"
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
        private:
            struct chunk;
            struct item{
                irr::scene::IMeshSceneNode  * node;
                mapId id;
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                bodyInfo info;
                chunk * parent;
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
            item * makeTerrainItem(int,int,float,float,float);
        private:
            struct conf{
                bool                 haveBody;
                shapeGroup           shape;
                irr::scene::IMesh *  mesh;
                float                deltaHeight;
                irr::core::vector3df scale;
            };
            std::map<int,conf*> config;
            
            void loadConfig();
            void loadJSON(cJSON * json);
            void releaseConfig();
    };
}
#endif