#ifndef SMOOTHLY_UTILS
#define SMOOTHLY_UTILS
#include <irrlicht/irrlicht.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <list>
namespace smoothly{
    inline void quaternion2euler(const btQuaternion & q , irr::core::vector3df & e){
        irr::core::quaternion irrq(q.x(),q.y(),q.z(),q.w());
        irrq.toEuler(e);
    }
    inline void euler2quaternion(const irr::core::vector3df & e , btQuaternion & q){
        irr::core::quaternion irrq(e);
        q.setValue(irrq.X , irrq.Y , irrq.Z , irrq.W);
    }
    class ipair{
        public:
            int x,y;
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
            inline ipair(int & ix , int & iy){
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
