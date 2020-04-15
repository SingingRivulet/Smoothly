#ifndef SMOOTHLY_PATHFINDING
#define SMOOTHLY_PATHFINDING
#include "body.h"
#include <unordered_set>
namespace smoothly{

class pathFinding:public body{
    public:
        pathFinding();
        bool pathFindingMode;
        class pathFindingContext{
            public:
                struct node{
                        blockPosition position;
                        int g; //起始点到当前点实际代价
                        int h;//当前节点到目标节点最佳路径的估计代价
                        int f;//估计值
                        node * parent;//父节点
                };

                pathFinding * parent;

                node * processing;
                node * result;
                inline pathFindingContext(){
                    parent     = NULL;
                    processing = NULL;
                    result     = NULL;
                }

                std::map<blockPosition,node*> openlist,closelist;

                blockPosition start,target;

                int heuristic(const blockPosition & p1,blockPosition & p2);//启发函数

                void getWalkablePoint(int x, int y, int z, std::vector<blockPosition> & res);
                void checkWalkablePoint(int x, int y, int z, std::vector<blockPosition> & res);

                bool failed;

                void init();//初始化寻路（先设置好start,target）
                void process();
                void clear();
                void buildRoad(std::function<void(const building::blockPosition &)> callback);

                bool pathFindingStart(std::function<void(const building::blockPosition &)> callback);
        };
        inline bool findPath(const blockPosition & A,const blockPosition & B,std::function<void(const building::blockPosition &)> callback){
            pathFindingContext ctx;
            ctx.parent = this;
            ctx.start = A;
            ctx.target = B;
            return ctx.pathFindingStart(callback);
        }
        bool findPath(const irr::core::vector3df & A,const irr::core::vector3df & B,std::list<irr::core::vector3df> & r);

        void findPathByRay(const vec3 & start,const vec3 & end);
        inline void findPathByRay(){
            auto ori    = camera->getPosition();
            auto dir    = camera->getTarget()-ori;
            dir.normalize();
            auto start  = ori;
            auto end    = ori+dir*200;
            findPathByRay(start,end);
        }

        void onDraw()override;
        void cancle()override;
    private:
        irr::video::ITexture * texture_pathTarget;
        irr::video::ITexture * texture_pathPoint;
};

}

#endif // SMOOTHLY_PATHFINDING
