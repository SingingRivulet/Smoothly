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
                ++setCharNum;
                b.setPosition(name,x,y);
                if(name==mainControl)
                    v.setPosition(x,y);
            }
            inline void removeCharacterChunk(const std::string & name){
                b.removePosition(name);
            }
            inline void process(){
                if(setCharNum!=0){
                    setCharNum = 0;
                    b.setPositionEnd();
                }
                b.process();
                v.process();
            }
            bool chunkLoaded(int x,int y)override;

            std::string mainControl;
        private:
            int setCharNum;
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
