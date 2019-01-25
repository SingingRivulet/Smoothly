#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include "utils.h"
#include "mods.h"
#include <map>
#include <set>
namespace smoothly{
    btRigidBody * makeBulletMeshFromIrrlichtNode( const irr::scene::ISceneNode * node );
    class terrain{
        public:
            irr::IrrlichtDevice * device;
            irr::scene::ISceneManager * scene;//场景
            mods * m;
            terrain();
            ~terrain();
            void destroy();
        public:
            class chunk;
            class mapid{
                public:
                    int x,y,mapId;
                    long itemId;
                    inline bool operator==(const mapid & i)const{
                        return (x==i.x) && (y==i.y) && (itemId==i.itemId) && (mapId==i.mapId);
                    }
                    inline bool operator<(const mapid & i)const{
                        if(x<i.x)
                            return true;
                        else
                        if(x==i.x){
                            if(y<i.y)
                                return true;
                            else
                            if(y==i.y){
                                if(itemId<i.itemId)
                                    return true;
                                else
                                if(itemId==i.itemId){
                                    if(mapId<i.mapId)
                                        return true;
                                }
                            }
                        }
                            return false;
                    }
                    mapid(){
                        x=0;
                        y=0;
                        itemId=0;
                        mapId=0;
                    }
                    mapid(const mapid & i){
                        x=i.x;
                        y=i.y;
                        itemId=i.itemId;
                        mapId=i.mapId;
                    }
                    mapid(int ix,int iy,long iitemId,int imapId){
                        x=ix;
                        y=iy;
                        itemId=iitemId;
                        mapId=imapId;
                    }
                    mapid & operator=(const mapid & i){
                        x=i.x;
                        y=i.y;
                        itemId=i.itemId;
                        mapId=i.mapId;
                        return *this;
                    }
            };
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
                    long id;
                    mods::itemBase * parent;
                    chunk * inChunk;
                    irr::scene::IMeshSceneNode * node;
                    item * next;
                    int mapId;
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
                    std::set<item*> items;
                    float ** T;
                    irr::scene::IMesh * mesh;
                    terrain * parent;
                    int x,y;
                    std::map<long,int> itemNum;
                    irr::scene::ITriangleSelector * selector;
                    inline void remove(){
                        for(auto it:items){
                            parent->allItems.erase(mapid(this->x,this->y,it->id,it->mapId));
                            it->remove();
                            parent->destroyItem(it);
                        }
                        node->remove();
                        selector->drop();
                        mesh->drop();
                        items.clear();
                        itemNum.clear();
                        //printf("remove chunk:(%d,%d)\n",x,y);
                    }
                    inline bool getCollisionPoint(
                        const irr::core::line3d<irr::f32>& ray,
                        irr::core::vector3df& outCollisionPoint,
                        irr::core::triangle3df& outTriangle,
                        irr::scene::ISceneNode*& outNode
                    ){
                        return scene->getSceneCollisionManager()->getCollisionPoint(
                            ray,
                            selector,
                            outCollisionPoint,
                            outTriangle,
                            outNode
                        );
                    }
                    inline int getId(const long & iid){
                        auto it=itemNum.find(iid);
                        if(it==itemNum.end()){
                            itemNum[iid]=1;
                            return 0;
                        }else{
                            return (it->second)++;
                        }
                    }
                    virtual int add(
                        long id,
                        const irr::core::vector3df & p,
                        const irr::core::vector3df & r,
                        const irr::core::vector3df & s
                    );
                    virtual float getRealHight(float x,float y);
            };
            
            std::map<ipair,chunk*> chunks;
            //chunksize:32*32
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
                return getHightf(x/32 , y/32);
            }
            
            //真实高度=海拔+山高
            inline float getRealHight(int x , int y){
                return getHillHight(x,y)+getAltitude(x,y);
                //return x+y;
            }
            
            inline float getTemperatureF(float x , float y){
                return generator.get(x/temperatureArg , y/temperatureArg , 4096)*temperatureK;
            }
            
            inline float getHumidityF(float x , float y){
                return generator.get(x/humidityArg , y/humidityArg , 8192)*humidityK;
            }
            
            inline float getDefaultCameraHight(float x , float y){
                return getRealHight(x,y)+1;
            }
            
            float genTerrain(
                float ** ,  //高度图边长=chunk边长+1
                irr::s32 x , irr::s32 y //chunk坐标，真实坐标/32
            );//返回最大值
            
        public:
            
            inline float getHight(irr::s32 x , irr::s32 y){//chunk高度(近似海拔)
                return getHightf(x , y);
            }
            inline float getTemperature(irr::s32 x , irr::s32 y){//温度
                return getTemperatureF(x , y);
            }
            inline float getHumidity(irr::s32 x , irr::s32 y){//湿度
                return getHumidityF(x , y);
            }
            void getItems(irr::s32 x , irr::s32 y , chunk * ch);//获取chunk中所有物体
            
            void * ipool;//内存池（因为直接定义mempool会导致重复定义问题，所以用void指针）
            
            std::map<mapid,item*> allItems;
            item * createItem();
            void destroyItem(item *);
            
            virtual bool itemExist(int ix,int iy,long iitemId,int imapId);
            bool remove(const mapid & mid);
        
        public:
        
            //地图更新
            irr::s32 px;
            irr::s32 py;//之前角色位置
            chunk * visualChunk[7][7];//可见的chunk（脚下一个，前3个后3个）
            
            void * cpool;//内存池（因为直接定义mempool会导致重复定义问题，所以用void指针）
            chunk * createChunk();//使用内存池创建一个chunk
            void removecChunk(chunk *);//回收chunk
            void updateChunk(chunk * , irr::s32 x , irr::s32 y);
            void visualChunkUpdate(irr::s32 x , irr::s32 y , bool force=false);//参数为chunk坐标，表示新的角色所在位置
            
            void getNearChunk(bool(*callback)(chunk*,void*)/*return false:stop loop*/,void * arg);
            
            bool selectPoint(//准心拾取(查询周围9格)，建议用selectPointM代替
                const irr::core::line3d<irr::f32>& ray,
                irr::core::vector3df& outCollisionPoint
            );
            
            bool selectPointM(//准心拾取(查询周围4格)
                float x,
                float y,
                const irr::core::line3d<irr::f32>& ray,
                irr::core::vector3df& outCollisionPoint,
                irr::core::triangle3df& outTriangle,
                irr::scene::ISceneNode*& outNode
            );
            
            inline bool selectPointM(//准心拾取(自动定位chunk)
                const irr::core::line3d<irr::f32>& ray,
                irr::core::vector3df& outCollisionPoint,
                irr::core::triangle3df& outTriangle,
                irr::scene::ISceneNode*& outNode
            ){
                return selectPointM(
                    ray.start.X,
                    ray.start.Z,
                    ray,
                    outCollisionPoint,
                    outTriangle,
                    outNode
                );
            }
            
            inline bool selectPointInChunk(
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
                    return it->second->getCollisionPoint(ray,outCollisionPoint,outTriangle,outNode);
                }
            }
            
            inline void getCameraChunkPosition(float x,float y,int & ox,int & oy){
                ox=(int)(x/32);
                oy=(int)(y/32);
            }
        public:
            //生成节点名字，便于识别
            inline void getChunkName(char * buf,int len,int x,int y){
                snprintf(buf,len,"tc %d %d",x,y);
            }
            inline void getItemName(char * buf,int len,int x,int y,long id,int itemid){
                snprintf(buf,len,"ti %d %d %ld %d",x,y,id,itemid);
            }
            inline bool isChunk(const char * buf){
                if(buf[0]!='t')
                    return false;
                if(buf[1]=='c')
                    return true;
                else
                    return false;
            }
            inline bool isItem(const char * buf){
                if(buf[0]!='t')
                    return false;
                if(buf[1]=='i')
                    return true;
                else
                    return false;
            }
            chunk * getChunkFromStr(const char * buf);
            item *  getItemFromStr(const char * buf);
        public:
        
            perlin3d generator;//噪声发生器
    };
    
}
#endif
