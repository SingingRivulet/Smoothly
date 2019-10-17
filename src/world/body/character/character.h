//smoothly/world/body/character
//角色
#ifndef SMOOTHLY_WORLD_BODY_CHARACTER
#define SMOOTHLY_WORLD_BODY_CHARACTER
#include "world/body/body.h"
#include <sstream>
#include <stdio.h>
#include <set>
namespace smoothly{
namespace world{
namespace body{
////////////////
class character:public body{
    public:
        void toString(std::string & s);
        void loadString(const std::string & s);
        vec3 position,rotation,lookAt;
        std::set<int> wearing;
        void doString(const std::string & s);
        
        virtual void wearing_add(int);
        virtual void wearing_remove(int);
        virtual void setLookAt(const vec3 & );
        virtual void setPosition(const vec3 & );
        virtual void setRotation(const vec3 & );
        virtual void interactive(const std::string & m)=0;
};
namespace character_cmd{
    std::string wearing_add(int);
    std::string wearing_remove(int);
    std::string setLookAt(const vec3 & );
    std::string setPosition(const vec3 & );
    std::string setRotation(const vec3 & );
    std::string interactive(const std::string & m);
}//////cmd
////////////////
}//////body
}//////world
}//////smoothly
#endif
