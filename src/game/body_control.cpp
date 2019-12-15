#include "body.h"

namespace smoothly{

inline void rotate2d(irr::core::vector2df & v,double a){
    auto cosa=cos(a);
    auto sina=sin(a);
    auto x=v.X*cosa - v.Y*sina;
    auto y=v.X*sina + v.Y*cosa;
    v.X=x;
    v.Y=y;
    //v.normalize();
}

void body::bodyItem::walk(int forward,int leftOrRight/*-1 left,1 right*/,float speed){

    irr::core::vector2df delta(0,0);
    irr::core::vector3df direct=lookAt;
    irr::core::vector2df direct2d(direct.X , direct.Z);
    irr::core::vector2df p2d;
    direct2d.normalize();

    if(forward==1){
        delta+=direct2d;
    }else
    if(forward==-1){
        delta-=direct2d;
    }

    if(leftOrRight==1){
        p2d=direct2d;
        rotate2d(p2d, 3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }else
    if(leftOrRight==-1){
        p2d=direct2d;
        rotate2d(p2d, -3.1415926535897/2);
        p2d.normalize();
        delta+=p2d;
    }
    delta.normalize();
    delta*=speed;
    m_character.setWalkDirection(btVector3(delta.X , 0 , delta.Y));
}

void body::bodyItem::setLookAt(const vec3 & l){
    m_character.setDir(l);
    lookAt = l;
}
void body::doCommond(const commond & c){
    switch (c.cmd) {
    case CMD_SET_LOOKAT:
        break;
    case CMD_SET_POSITION:
        break;
    case CMD_SET_ROTATION:
        break;
    case CMD_JUMP:
        break;
    case CMD_STATUS_SET:
        break;
    case CMD_STATUS_ADD:
        break;
    case CMD_STATUS_REMOVE:
        break;
    case CMD_INTERACTIVE:
        break;
    case CMD_WEARING_ADD:
        break;
    case CMD_WEARING_REMOVE:
        break;
    case CMD_WEARING_CLEAR:
        break;
    default:
        break;
    }
}

}
