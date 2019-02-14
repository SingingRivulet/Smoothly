#ifndef SMOOTHLY_REMOTEGRAPH
#define SMOOTHLY_REMOTEGRAPH
#include "utils.h"
#include "uuid.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
namespace smoothly{
    class remoteGraph{
        //负责访问远程的建筑节点结构，以及其他参数
        //碰撞，显示等等交给子类
        public:
            
            enum uploadMode {
                ADD_ITEM,REMOVE_ITEM
            };
            
            class chunk;
            class item;
            std::set<item*> removelist;
            std::set<item*> createlist;
            std::set<item*> genlist;
            int maxdeep;
            
            std::unordered_map<std::string,item*> items;
            std::map<ipair,chunk*> chunks;
            
            static inline void getChunkPosition(double ix,double iy,int & ox,int & oy){
                ox=ix/32;
                oy=iy/32;
            }
            
            class item{
                public:
                    chunk * inChunk;
                    remoteGraph * parent;
                    std::string uuid;
                    irr::core::vector3df position;
                    std::set<std::string> link,linkTo;
                    bool isRoot;
                    item * next;
                    inline void construct(){
                        uuid.clear();
                        link.clear();
                        linkTo.clear();
                        inChunk=NULL;
                        destroyed=false;
                    }
                    inline void destruct(){
                        uuid.clear();
                        link.clear();
                        linkTo.clear();
                    }
                    bool destroyed;//已经被摧毁
                    void attack(item *,int deep);
                    void remove(item *);
                    void destroy();
                    void destroy(int deep);
            };
            
            class chunk{
                public:
                    int x,y;//坐标，同terrain的chunk
                    std::set<item *> items;
                    remoteGraph * parent;
                    chunk * next;
                    void clear();
                    inline void construct(){
                        this->clear();
                    }
                    inline void destruct(){
                        this->clear();
                    }
            };
            
            item * addNode(//添加节点
                const irr::core::vector3df & position,//位置
                const std::set<std::string> & link//表示修建在什么节点上
            );
            
            item * genNode(
                const irr::core::vector3df & position,//位置
                const std::set<std::string> & link,//表示修建在什么节点上
                const std::set<std::string> & linkTo,
                const std::string & uuid,
                bool createmode
            );
            
            void removeNode(
                const std::string & uuid,
                bool updatemode=false//更新模式，不会引发变动，否则将会上传变动
            );
            
            void clearNodes();
            
            void setChunk(item *,const irr::core::vector3df & position);//将节点插入到区块和全局表里面
            void removeFromChunk(item *);
            
            item * seekNode(const std::string & uuid,bool download=true);
            
            item * createNode();
            void delNode(item *);
            
            chunk * createChunk();
            void delChunk(chunk *);
            
            void createChunk(int x,int y);
            
            bool inRange(item *);
            void removeApplay();
            void createListFilter();
            
            inline void buildingApplay(){
                removeApplay();
                createListFilter();
            }
            
            void updateBuildingChunks(int x,int y,int range);
            
            virtual void onGenBuilding(item *)=0;
            virtual void onFreeBuilding(item *)=0;
            virtual void onDestroyBuilding(item *)=0;
            virtual void onCreateBuilding(item *)=0;
            
            virtual void uploadBuilding(const std::string & uuid , uploadMode m)=0;
            virtual void downloadBuilding(int x,int y)=0;
            virtual item * downloadBuilding(const std::string & uuid)=0;
            
            void onMessageGen(
                const std::string & uuid,
                const irr::core::vector3df & position,
                const std::set<std::string> & link,
                const std::set<std::string> & linkTo
            );
            void onMessageCreate(
                const std::string & uuid,
                const irr::core::vector3df & position,
                const std::set<std::string> & link
            );
            void onMessageDestroy(const std::string & uuid);
            
        private:
            void * buildingPool;
            void * buildingChunkPool;
            void poolInit();
            void poolFree();
    };
}
#endif