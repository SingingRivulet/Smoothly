#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include "utils.h"
#include "mods.h"
namespace smoothly{
    
    class perlin3d{
        public:
            int seed;
            int num;
            float step;
            float height;
            
            perlin3d();
            ~perlin3d();
            
            float rand(float x , float y , float z);
            float smooth(int x,int y,int z);
            float interpolate(float a,float b,float x);
            float inoise(float x,float y,float z);
            float get(float x , float y , float z);
    };
    
    class terrain{
        public:
            irr::IrrlichtDevice * device;
            irr::scene::ISceneManager * scene;//场景
            mods * m;
            terrain();
            ~terrain();
        public:
            class item{
                public:
                    int id;
                    mods::item * parent;
                    irr::scene::IMeshSceneNode * node;
                    inline void remove(){
                        node->remove();
                    }
                    inline void setPosition(const irr::core::vector3df & r){
                        node->setPosition(r);
                    }
                    inline void setRotation(const irr::core::vector3df & r){
                        node->setRotation(r);
                    }
                    inline void setScale(const irr::core::vector3df & r){
                        node->setScale(r);
                    }
            };
            
            class chunk:public mods::mapGenerator{
                public:
                    chunk * next;
                    std::list<item> items;
                    irr::video::IImage * T;
                    int x,y;
                    inline void remove(){
                        node->remove();
                        T->drop();
                        items.clear();
                    }
                    virtual void add(
                        int id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    );
            };
            
            //chunksize:128*128
            float pointNum;//每条边顶点数量
            float max,min,k;
            
        public:
        
            //地形生成器
            irr::u32 getHightf(float x , float y);
            inline irr::u32 getHillHight(float x , float y){//山高
                return getHightf(x/128 , y/128);
            }
            irr::u32 getAltitude(float x , float y);//海拔
            //真实高度=海拔+山高
            void genTerrain(
                irr::video::IImage * ,  //高度图边长=chunk边长+1
                irr::u32 x , irr::u32 y //chunk坐标，真实坐标/128
            );
            
        public:
            
            inline irr::u32 getHight(irr::u32 x , irr::u32 y){//chunk高度
                return getHightf(x , y);
            }
            irr::u32 getTemperature(irr::u32 x , irr::u32 y);//温度
            irr::u32 getHumidity(irr::u32 x , irr::u32 y);//湿度
            void getItems(irr::u32 x , irr::u32 y , chunk & ch);//获取chunk中所有物体
        
        public:
        
            //地图更新
            irr::u32 px;
            irr::u32 py;//之前角色位置
            chunk * visualChunk[7][7];//可见的chunk（脚下一个，前3个后3个）
            
            void * pool;//内存池（因为直接定义mempool会导致重复定义问题，所以用void指针）
            chunk * createChunk();//使用内存池创建一个chunk
            void removecChunk(chunk *);//回收chunk
            void updateChunk(chunk * , irr::u32 x , irr::u32 y);
            void visualChunkUpdate(irr::u32 x , irr::u32 y);//参数为chunk坐标，表示新的角色所在位置
            
        private:
        
            perlin3d generator;//噪声发生器
    };
    
}
#endif
