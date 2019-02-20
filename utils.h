#ifndef SMOOTHLY_UTILS
#define SMOOTHLY_UTILS
#include <irrlicht/irrlicht.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <list>
namespace smoothly{
    
    enum GameMessage{
        MESSAGE_GAME=ID_USER_PACKET_ENUM+1
    };
    
    enum MessageMethodFlag{
        M_UPDATE_BUILDING = 'b',
        M_UPDATE_TERRAIN  = 't',
        M_UPDATE_OBJECT   = 'o'
    };
    
    enum BuildingMethod{
        B_ATTACK            ='a',
        B_ATTACK_UPLOAD     ='u',
        B_CREATE            ='c',
        B_CREATE_UPLOAD     ='p',
        B_DESTROY           ='d',
        B_GENER             ='g',
        B_DOWNLOAD_CHUNK    ='l',
        B_DOWNLOAD_UUID     ='i'
    };
    
    inline void quaternion2euler(const btQuaternion & q , irr::core::vector3df & e){
        irr::core::quaternion irrq(q.x(),q.y(),q.z(),q.w());
        irrq.toEuler(e);
    }
    inline void euler2quaternion(const irr::core::vector3df & e , btQuaternion & q){
        irr::core::quaternion irrq(e);
        q.setValue(irrq.X , irrq.Y , irrq.Z , irrq.W);
    }
    inline float getLenSq(const irr::core::vector3df & f,const irr::core::vector3df & t){
        auto l=t-f;
        return (l.X*l.X + l.Y*l.Y + l.Z*l.Z);
    }
    class ipair{
        public:
            int32_t x,y;
            inline bool operator==(const ipair & i)const{
                return (x==i.x) && (y==i.y);
            }
            inline bool operator<(const ipair & i)const{
                if(x<i.x)
                    return true;
                else
                if(x==i.x){
                    if(y<i.y)
                        return true;
                }
                    return false;
            }
            inline ipair & operator=(const ipair & i){
                x=i.x;
                y=i.y;
                return *this;
            }
            inline ipair(const ipair & i){
                x=i.x;
                y=i.y;
            }
            inline ipair(const int & ix , const int & iy){
                x=ix;
                y=iy;
            }
            inline ipair(){
                x=0;
                y=0;
            }
    };
}
#endif
