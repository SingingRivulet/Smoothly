#include "simple.h"
#include <stdio.h>
namespace smoothly{
namespace world{
namespace body{
////////////////
using namespace std;
void simple::toString(std::string & s){
    char buf[512];
    snprintf(buf,sizeof(buf),"%f %f %f %f %f %f",
        position.X,position.Y,position.Z,
        rotation.X,rotation.Y,rotation.Z);
    s = buf;
}
void simple::loadString(const std::string & s){
    sscanf(s.c_str(),"%f %f %f %f %f %f",
        &(position.X),&(position.Y),&(position.Z),
        &(rotation.X),&(rotation.Y),&(rotation.Z));
}
////////////////
}//////body
}//////world
}//////smoothly