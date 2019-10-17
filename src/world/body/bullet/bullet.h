//smoothly/world/body/bullet
//子弹
#ifndef SMOOTHLY_WORLD_BODY_BULLET
#define SMOOTHLY_WORLD_BODY_BULLET
#include "world/body/simple/simple.h"
namespace smoothly{
namespace world{
namespace body{
////////////////
class bullet:public simple{
    //子弹不需要存储，只需要创建，有自己的生命周期
    public:
        void toString(std::string & s);
        void loadString(const std::string & s);
        vec3 direction;
};
////////////////
}//////body
}//////world
}//////smoothly
#endif