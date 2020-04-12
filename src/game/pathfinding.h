#ifndef SMOOTHLY_PATHFINDING
#define SMOOTHLY_PATHFINDING
#include "body.h"
namespace smoothly{

class pathFinding:public body{
    public:
        pathFinding() {}
        bool checkObst(btConvexInternalShape & shape,btVector3 & posi);//检查一个位置是否能走
};

}

#endif // SMOOTHLY_PATHFINDING
