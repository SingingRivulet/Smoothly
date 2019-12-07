#ifndef SMOOTHLY_TERRAIN_DISPATHER
#define SMOOTHLY_TERRAIN_DISPATHER
#include "terrain_item.h"
#include "../libclient/terrainDispather.h"
namespace smoothly{
    class terrainDispather:public terrain_item{
        public:
            terrainDispather();
            ~terrainDispather();
            inline void setCharacterChunk(const std::string & name , int x,int y){
                b.setPosition(name,x,y);
                if(name==mainControl)
                    v.setPosition(x,y);
            }
            inline void removeCharacterChunk(const std::string & name){
                b.removePosition(name);
            }
            inline void process(){
                b.process();
                v.process();
            }
            std::string mainControl;
        private:
            struct bDs:public world::terrain::bodyDispather{
                terrainDispather * parent;
                virtual void createChunk(int x , int y);
                virtual void removeChunk(int x , int y);
            }b;
            struct vDs:public world::terrain::viewDispather{
                terrainDispather * parent;
                virtual void createChunk(int x , int y);
                virtual void removeChunk(int x , int y);
            }v;
    };
}
#endif
