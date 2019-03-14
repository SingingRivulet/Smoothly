#ifndef SMOOTHLY_TERRAIN
#define SMOOTHLY_TERRAIN
#include "utils.h"
#include "mods.h"
#include "physical.h"
#include <map>
#include <set>
#include <list>
#include <queue>
#include <mutex>
#include <condition_variable>
namespace smoothly{
    class terrain:public physical{
        public:
            irr::IrrlichtDevice * device;
            irr::scene::ISceneManager * scene;//场景
            irr::ITimer * timer;
            btDiscreteDynamicsWorld * dynamicsWorld;
            mods * m;
            terrain();
            ~terrain();
            void destroy();
            
            struct terrainBuffer{
                terrainBuffer * next;
                float ** buf;
            };
        private:
            void * tbufpool;
            terrainBuffer * createTBuf();
            void delTBuf(terrainBuffer * p);
        public:
            class chunk;
            class item:public mods::itemBase{
                public:
                    long id;
                    mods::itemBase * parent;
                    chunk * inChunk;
                    irr::scene::IMeshSceneNode * node;
                    
                    btRigidBody   * rigidBody;
                    btMotionState * bodyState;
                    
                    item * next;
                    int mapId;
                    inline void remove(){
                        node->remove();
                        inChunk->parent->dynamicsWorld->removeRigidBody(rigidBody);
                        if(rigidBody){
                            delete rigidBody;
                            delete bodyState;
                        }
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
                    std::set<void*> buildings;//预留给子类
                    irr::scene::IMesh * mesh;
                    
                    
                    btRigidBody      * rigidBody;
                    btMotionState    * bodyState;
                    btCollisionShape * bodyShape;
                    btTriangleMesh   * bodyMesh;
                    
                    terrainBuffer * tbuf;
                    
                    terrain * parent;
                    int x,y;
                    std::map<long,int> itemNum;
                    irr::scene::ITriangleSelector * selector;
                    std::map<long,std::set<int> > removeTable;
                    bool nodeInited;
                    inline void remove(){
                        //printf("remove (%d,%d)\n",x,y);
                        for(auto it:items){
                            parent->onFreeTerrainItem(it);
                            parent->allItems.erase(mapid(this->x,this->y,it->id,it->mapId));
                            it->remove();
                            parent->destroyItem(it);
                        }
                        if(nodeInited){
                            parent->dynamicsWorld->removeRigidBody(this->rigidBody);
                            delete rigidBody;
                            delete bodyState;
                            delete bodyShape;
                            delete bodyMesh;
                            selector->drop();
                            node->remove();
                        }
                        if(mesh)
                            mesh->drop();
                        items.clear();
                        itemNum.clear();
                        removeTable.clear();
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
        private:
            std::list<ipair> updatePath;
            void createUpdatePath(int range);
        public:
            int pointNum;//每条边顶点数量
            float altitudeK;
            float hillK;
            float temperatureK;
            float humidityK;
            
            float altitudeArg;
            float hillArg;
            float temperatureArg;
            float humidityArg;
            
            float temperatureMax;
            float humidityMax;
            
            float temperatureMin;
            float humidityMin;
            
            irr::video::ITexture* texture;
            
            void genTexture();
            void destroyTexture();
        public:
        
            irr::scene::IMesh * createTerrainMesh(
                irr::video::ITexture* texture,
                float ** heightmap, 
                irr::u32 hx,irr::u32 hy,
                const irr::core::dimension2d<irr::f32>& stretchSize,
                const irr::core::dimension2d<irr::u32>& maxVtxBlockSize, //网眼大小。官方文档没写
                bool debugBorders) const;
            //irrlicht自带的太差，所以自己实现一个
            
        public:
            /*
             * 
             * 0-------------------------------------->湿度
             * |戈壁                              雪地
             * |
             * |
             * |
             * |
             * |
             * |
             * |
             * |
             * |                泥地
             * |
             * |
             * |
             * |
             * |
             * |
             * |
             * |沙漠                              沼泽
             * V
             * 温度
             * 
             */
            //地形生成器
            
            inline void setSeed(int seed){
                genRiver(seed);
            }
            
            struct river{
                int a,b,c,d,e,f,g,h,i,j;
                float w;
                float getD(float x,float y);
                float getK(float x,float y);
                float sigmoid(float x);
            };
            
            river rivers[3];
            
            void genRiver(int seed);
            
            float getRiver(float x , float y);
            
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
                return (getHillHight(x,y)+getAltitude(x,y))*getRiver(x,y);
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
            
            inline static short getLevel(float min,float max,float v){
                if(v<=min)
                    return 0;
                if(v>=max)
                    return 16;
                float l=max-min;
                float r=v-min;
                return (r/l)*16;
            }
            inline short getHumidityLevel(float x,float y){
                return getLevel(humidityMin,humidityMax,getHumidityF(x,y));
            }
            inline short getTemperatureLevel(float x,float y){
                return getLevel(temperatureMin,temperatureMax,getTemperatureF(x,y));
            }
            
            void terrainLoop();
            void terrainParseOne();
            void updateTerrainThread();
            
        private:
            enum tuMethod {TU_CREATE,TU_DELETE};
            std::queue<std::pair<chunk*,tuMethod> > sendTChunkQ,recvTChunkQ;
            std::mutex sendTChunkQL,recvTChunkQL;
            std::mutex sqmtx;
            std::mutex chunkmtx;
            std::condition_variable sqcv;
            
        public:
            inline void terrainWake(){
                std::unique_lock <std::mutex> lck(sqmtx);
                sqcv.notify_all();
            }
            
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
            
            bool remove(const mapid & mid);
            void remove(item * i);
        
        public:
        
            //地图更新
            irr::s32 px;
            irr::s32 py;//之前角色位置
            
            void * cpool;//内存池（因为直接定义mempool会导致重复定义问题，所以用void指针）
            chunk * createChunk();//使用内存池创建一个chunk
            void removeChunk(chunk *);//回收chunk
            void updateChunk(chunk *);
            void updateChunkThread(chunk *);
            void visualChunkUpdate(irr::s32 x , irr::s32 y , bool force=false);//参数为chunk坐标，表示新的角色所在位置
            
            bool selectPointM(//准心拾取(查询周围4格)
                float x,
                float y,
                const irr::core::line3d<irr::f32>& ray,
                irr::core::vector3df& outCollisionPoint,
                irr::core::triangle3df& outTriangle,
                irr::scene::ISceneNode*& outNode
            );
            
            inline bool chunkExist(int x,int y){
                chunkmtx.lock();
                auto it=chunks.find(ipair(x,y));
                bool res=(it!=chunks.end());
                chunkmtx.unlock();
                return res;
            }
            
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
                chunkmtx.lock();
                auto it=chunks.find(ipair(x,y));
                if(it==chunks.end()){
                    chunkmtx.unlock();
                    return false;
                }else{
                    auto res=it->second->getCollisionPoint(ray,outCollisionPoint,outTriangle,outNode);
                    chunkmtx.unlock();
                    return res;
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
            inline void addIntoRemoveTable(const mapid & mid){
                chunkmtx.lock();
                auto it=chunks.find(ipair(mid.x , mid.y));
                if(it!=chunks.end()){
                    if(it->second)
                        it->second->removeTable[mid.itemId].insert(mid.mapId);
                }
                chunkmtx.unlock();
            }
            
            chunk * getChunkFromStr(const char * buf);
            item *  getItemFromStr(const char * buf);
            
            void setRemoveTable(int x,int y,const std::list<std::pair<long,int> > & t);
            void removeTableApply(int x,int y);
            
            virtual void onGenChunk(chunk *)=0;
            virtual void onFreeChunk(chunk *)=0;
            virtual void requestRemoveItem(const mapid & mid)=0;
            virtual void requestUpdateTerrain(int x,int y)=0;
            
            virtual void onFreeTerrainItem(item *)=0;
            virtual void onGenTerrainItem(item *)=0;
        public:
        
            perlin3d generator;//噪声发生器
    };
    
}
#endif
