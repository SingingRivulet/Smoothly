//smoothly/world/body
//物体的基类
#ifndef SMOOTHLY_WORLD_BODY
#define SMOOTHLY_WORLD_BODY
#include "utils/utils.h"
#include <string>
namespace smoothly{
namespace world{
namespace body{
////////////////
class body{//body接口
    public:
        int id;//物体id
        std::string uuid;
        std::string owner;
        int hp;
        virtual void toString(std::string & s)=0;           //转字符串
        virtual void loadString(const std::string & s)=0;   //加载字符串
        virtual void doString(const std::string & s);       //执行字符串（命令）
};
////////////////
}//////body
}//////world
}//////smoothly
#endif