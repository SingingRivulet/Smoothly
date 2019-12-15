#ifndef SMOOTHLY_INTERACTIVE
#define SMOOTHLY_INTERACTIVE
#include "body.h"

namespace smoothly{

class interactive:public body{
    public:
        virtual void setInteractiveNode(bodyItem * b , const std::string & method);
};

}

#endif
