#ifndef SMOOTHLY_SUBTANCE_ANIMATION
#define SMOOTHLY_SUBTANCE_ANIMATION
#include "utils.h"
namespace smoothly{
    class subsani{
        public:
            virtual void addIntoScene()=0;
            virtual void removeFromScene()=0;
            
            virtual void setLOD(float len)=0;
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl)=0;
            
            virtual void setPosition(const irr::core::vector3df &)=0;
            virtual void setRotation(const irr::core::vector3df &)=0;
            virtual void setDirection(const irr::core::vector3df &)=0;
            virtual void setStatus(int id)=0;
            
            irr::scene::ISceneManager * scene;
    };
    class subsaniStatic:public subsani{
        
    };
    class subsaniBone:public subsani{
        
    };
    class subsaniStatic:public subsani{
        
    };
}
#endif