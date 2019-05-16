#ifndef SMOOTHLY_SUBTANCE_ANIMATION
#define SMOOTHLY_SUBTANCE_ANIMATION
#include "utils.h"
namespace smoothly{
    class subsani{
        public:
            
            virtual void setLOD(float len)=0;
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl)=0;
            
            virtual void setPosition(const irr::core::vector3df &)=0;
            virtual void setRotation(const irr::core::vector3df &)=0;
            virtual void setDirection(const irr::core::vector3df &)=0;
            virtual void setStatus(int id)=0;
            
            virtual void destruct()=0;
            
            virtual void doAttaching(const bodyAttaching & )=0;
            
            irr::scene::ISceneManager * scene;
            mods * m;
    };
    class subsaniStatic:public subsani{
        public:
            
            virtual void setLOD(float len);
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl);
            
            virtual void setPosition(const irr::core::vector3df &);
            virtual void setRotation(const irr::core::vector3df &);
            virtual void setDirection(const irr::core::vector3df &);
            virtual void setStatus(int id){}
            
            virtual void destruct();
            
            irr::scene::ISceneNode * node;
    };
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    class subsaniChar:public subsani{//角色
        public:
            
            virtual void setLOD(float len);
            
            virtual void setPosition(const irr::core::vector3df &);
            virtual void setRotation(const irr::core::vector3df &);
            virtual void setDirection(const irr::core::vector3df &);
            virtual void setStatus(int id);
            
            virtual void destruct();
            
            subsaniChar(irr::scene::ISceneManager * scene);
            
        private:
            irr::scene::IAnimatedMesh * body;//物体贴body上
            std::set<irr::u32,irr::scene::IAnimatedMesh*> items;//物体
            std::set<irr::u32,irr::scene::IAnimatedMesh*> parts;//伴随body，如衣服等
            
            void doAniItem(irr::u32,int speed,int start,int end ,bool loop);
            void doAniPart(irr::u32,int speed,int start,int end ,bool loop);
            void doAni(int speed,int start,int end ,bool loop);
            void setItem(irr::u32,irr::scene::IAnimatedMesh*);
            void setPart(irr::u32,irr::scene::IAnimatedMesh*);
            void setBody(irr::scene::IAnimatedMesh*);
            void removeItem(irr::u32);
            void removePart(irr::u32);
            
            void removeAll();
            
            static void doAni(irr::scene::IAnimatedMesh*,int speed,int start,int end ,bool loop);
    };
}
#endif