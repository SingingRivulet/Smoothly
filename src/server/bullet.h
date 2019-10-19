#ifndef SMOOTHLY_SERVER_BULLET
#define SMOOTHLY_SERVER_BULLET
#include "body.h"
namespace smoothly{
namespace server{

class bullet:public body{
    public:
        void shoot(const std::string & user,int id,const vec3 & from,const vec3 & dir);
        virtual void boardcast_shoot(const std::string & user,int id,const vec3 & from,const vec3 & dir)=0;
};

}//////server
}//////smoothly
#endif