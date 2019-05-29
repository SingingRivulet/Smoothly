#ifndef SMOOTHLY_SUBSTANCE_CHANGING
#define SMOOTHLY_SUBSTANCE_CHANGING
#include "utils.h"
namespace smoothly{
    class subsChanging{
        //数据对比，判断上传
        public:
            void init();
            
            bool diff(
                const irr::core::vector3df & p,
                const irr::core::vector3df & r,
                const irr::core::vector3df & d,
                const btVector3 & l,
                const btVector3 & a,
                int s
            );
            
        private:
            bool first;
            irr::core::vector3df position;
            irr::core::vector3df rotation;
            irr::core::vector3df direction;
            btVector3 lin_vel;
            btVector3 ang_vel;
            int status;
    };
}
#endif