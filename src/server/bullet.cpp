#include "bullet.h"
namespace smoothly{
namespace server{
////////////////
void bullet::shoot(const std::string & user,int /*id*/,const vec3 & from,const vec3 & dir){
    char key[256];
    std::string value;
    snprintf(key,sizeof(key),"sType:%s", user.c_str());
    db->Get(leveldb::ReadOptions(), key , &value);
    if(!value.empty()){
        int rid = atoi(value.c_str());
        boardcast_shoot(user,rid,from,dir);
    }
}
////////////////
}//////server
}//////smoothly
