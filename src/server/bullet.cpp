#include "bullet.h"
namespace smoothly{
namespace server{
////////////////
void bullet::shoot(const std::string & user,int id,const vec3 & from,const vec3 & dir){
    boardcast_shoot(user,id,from,dir);
}
////////////////
}//////server
}//////smoothly