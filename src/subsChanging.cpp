#include "subsChanging.h"
namespace smoothly{

void subsChanging::init(){
    first=true;
}

bool subsChanging::diff(
    const irr::core::vector3df & p,
    const irr::core::vector3df & r,
    const irr::core::vector3df & d,
    const btVector3 & l,
    const btVector3 & a,
    int s
){
    bool res;
    if(first){
        first=false;
        res=true;
    }else{
        
        res=false;
        
        if(p!=position) res=true;
        else
        if(r!=rotation) res=true;
        else
        if(d!=direction)res=true;
        else
        if(l!=lin_vel)  res=true;
        else
        if(a!=ang_vel)  res=true;
        else
        if(s!=status)   res=true;
        
        if(res==false)
            return false;
            
    }
    position    =p;
    rotation    =r;
    direction   =d;
    lin_vel     =l;
    ang_vel     =a;
    status      =s;
    
    return res;
}

}