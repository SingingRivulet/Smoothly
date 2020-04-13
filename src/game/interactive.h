#ifndef SMOOTHLY_INTERACTIVE
#define SMOOTHLY_INTERACTIVE
#include "pathfinding.h"

namespace smoothly{

class interactive:public pathFinding{
    public:
        virtual void setInteractiveNode(bodyItem * b , const std::string & method);
};

}

#endif
