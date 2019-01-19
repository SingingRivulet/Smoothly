#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include "utils.h"
#include "mods.h"
#include <map>
namespace smoothly{
    
    class perlin3d{
        public:
            int seed;
            int num;
            float step;
            float height;
            
            perlin3d();
            ~perlin3d();
            
            float rand(int x , int y , int z);
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
            void destroy();
        public:
            class ipair{
                public:
                    int x,y;
                    inline bool operator==(const ipair & i)const{
                        return (x==i.x) && (y==i.y);
                    }
                    inline bool operator<(const ipair & i)const{
                        if(x<i.x)
                            return true;
                        else
                        if(x==i.x){
                            if(y<i.y)
                                return true;
                        }
                            return false;
                    }
                    //inline bool operator<(const ipar & i)const{
                    //    if((x==i.x) && (y==i.y))
                    //        return false;
                    //    return
                    //}
                    inline ipair & operator=(const ipair & i){
                        x=i.x;
                        y=i.y;
                        return *this;
                    }
                    inline ipair(const ipair & i){
                        x=i.x;
                        y=i.y;
                    }
                    inline ipair(int & ix , int & iy){
                        x=ix;
                        y=iy;
                    }
                    inline ipair(){
                        x=0;
                        y=0;
                    }
            };
            class item:public mods::itemBase{
                public:
                    int id;
                    mods::itemBase * parent;
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
                    float ** T;
                    irr::scene::IMesh * mesh;
                    int x,y;
                    inline void remove(){
                        for(auto it:items){
                            it.remove();
                        }
                        node->remove();
                        mesh->drop();
                        items.clear();
                    }
                    virtual void add(
                        int id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    );
            };
            
            std::map<ipair,chunk*> chunks;
            //chunksize:128*128
            int pointNum;//每条边顶点数量
            float altitudeK;
            float hillK;
            float temperatureK;
            float humidityK;
            
            float altitudeArg;
            float hillArg;
            float temperatureArg;
            float humidityArg;
            irr::video::IImage* texture;
            
            void genTexture();
            void destroyTexture();
        public:
        
            irr::scene::IMesh * createTerrainMesh(
                irr::video::IImage* texture,
                float ** heightmap, 
                irr::u32 hx,irr::u32 hy,
                const irr::core::dimension2d<irr::f32>& stretchSize,
                irr::video::IVideoDriver* driver,
                const irr::core::dimension2d<irr::u32>& maxVtxBlockSize, //网眼大小。官方文档没写
                bool debugBorders) const;
            //irrlicht自带的太差，所以自己实现一个
            
        public:
            //地形生成器
            inline float getHightf(float x , float y){
                return generator.get(x/altitudeArg , y/altitudeArg , 1024)*altitudeK;
            }
            inline float getHillHight(float x , float y){//山高
                return generator.get(x/hillArg , y/hillArg , 2048)*hillK;
            }
            inline float getAltitude(float x , float y){//海拔
                return getHightf(x/128 , y/128);
            }
            
            //真实高度=海拔+山高
            
            inline float getTemperatureF(float x , float y){
                return generator.get(x/temperatureArg , y/temperatureArg , 4096)*temperatureK;
            }
            
            inline float getHumidityF(float x , float y){
                return generator.get(x/humidityArg , y/humidityArg , 8192)*humidityK;
            }
            
            float genTerrain(
                float ** ,  //高度图边长=chunk边长+1
                irr::u32 x , irr::u32 y //chunk坐标，真实坐标/128
            );//返回最大值
            
        public:
            
            inline float getHight(irr::u32 x , irr::u32 y){//chunk高度(近似海拔)
                return getHightf(x , y);
            }
            inline float getTemperature(irr::u32 x , irr::u32 y){//温度
                return getTemperatureF(x , y);
            }
            inline float getHumidity(irr::u32 x , irr::u32 y){//湿度
                return getHumidityF(x , y);
            }
            void getItems(irr::u32 x , irr::u32 y , chunk * ch);//获取chunk中所有物体
        
        public:
        
            //地图更新
            irr::u32 px;
            irr::u32 py;//之前角色位置
            chunk * visualChunk[7][7];//可见的chunk（脚下一个，前3个后3个）
            
            void * pool;//内存池（因为直接定义mempool会导致重复定义问题，所以用void指针）
            chunk * createChunk();//使用内存池创建一个chunk
            void removecChunk(chunk *);//回收chunk
            void updateChunk(chunk * , irr::u32 x , irr::u32 y);
            void visualChunkUpdate(irr::u32 x , irr::u32 y , bool force=false);//参数为chunk坐标，表示新的角色所在位置
            
        public:
        
            perlin3d generator;//噪声发生器
    };
    
}
#endif
