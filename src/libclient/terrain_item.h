//地形物体生成
#ifndef SMOOTHLY_WORLD_TERRAIN_ITEM
#define SMOOTHLY_WORLD_TERRAIN_ITEM
#include <list>
namespace smoothly{
namespace world{
namespace terrain{
////////////////
class predictableRand{
    public:
        inline int rand(){
            next = next * 1103515245 + 12345;
            return((unsigned)((next/65536)&0x7fffffff) % 32768);
        }
        inline float frand(){
            return ((float)rand())/32768.0f;
        }
        inline void setSeed(unsigned long seed){
            next=seed;
        }
    private:
        unsigned long next=1;
};
class terrain_item{
    public:
        struct genProb{
            int id,num;
            float prob;
        };
        inline void gen(int x,int y,int tem,int hu,float h){
            std::list<genProb> pl;//物体->概率
            predictableRand randg;
            
            getGenList(x,y,tem,hu,h,pl);
            
            randg.setSeed((x+10)*(y+20)*(hu+30)*(tem+40));
            
            for(auto it:pl){
                int delta=it.prob*1000;
                for(int i=0;i<it.num;i++){
                    int pr=(randg.rand())%1000;
                    if(pr>delta){
                        float mx=(randg.frand()+x)*32;
                        float my=(randg.frand()+y)*32;
                        float mr=randg.frand()*360;
                        gen(mx,my,mr,it.id);
                    }
                }
            }
        }
        virtual void getGenList(int x,int y,int tem,int hu,float h, std::list<genProb> & pl)=0;
        virtual void gen(float x,float y,float r,int id)=0;
};
////////////////
}//////terrain
}//////world
}//////smoothly
#endif
