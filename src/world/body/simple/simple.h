//smoothly/world/body/simple
//简单物体
#ifndef SMOOTHLY_WORLD_BODY_SIMPLE
#define SMOOTHLY_WORLD_BODY_SIMPLE
#include "world/body/body.h"
#include <sstream>
namespace smoothly{
namespace world{
namespace body{
////////////////
class simple:public body{
    public:
        void toString(std::string & s);
        void loadString(const std::string & s);
        vec3 position,rotation;
};
////////////////
}//////body
}//////world
}//////smoothly
#endif