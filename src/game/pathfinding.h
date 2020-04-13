#ifndef SMOOTHLY_PATHFINDING
#define SMOOTHLY_PATHFINDING
#include "body.h"
namespace smoothly{

class pathFinding:public body{
    public:
        pathFinding() {}
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
                pathFindingContext();

                std::map<blockPosition,node*> openlist,closelist;

                blockPosition start,target;

                int heuristic(const blockPosition & p1,blockPosition & p2);//启发函数

                void getWalkablePoint(int x, int y, int z, std::vector<blockPosition> & res);
                void checkWalkablePoint(int x, int y, int z, std::vector<blockPosition> & res);

                bool failed;

                void init();//初始化寻路（先设置好start,target）
                void process();
                void clear();
                void buildRoad(std::list<blockPosition> & res);

                bool pathFindingStart(std::list<building::blockPosition> & r);
        };
};

}

#endif // SMOOTHLY_PATHFINDING
