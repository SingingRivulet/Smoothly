#ifndef SMOOTHLY_SUBTANCE_ANIMATION
#define SMOOTHLY_SUBTANCE_ANIMATION
#include "utils.h"
#include "physical.h"
#include "mods.h"
namespace smoothly{
    
    class subsani{
        public:
            
            virtual void setLOD(float len)=0;
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl)=0;
            
            virtual const irr::core::vector3df & getPosition()=0;
            virtual const irr::core::vector3df & getRotation()=0;
            
            virtual void setPosition(const irr::core::vector3df &)=0;
            virtual void setRotation(const irr::core::vector3df &)=0;
            virtual void setDirection(const irr::core::vector3df &)=0;
            virtual void setStatus(int id)=0;
            
            virtual void destruct()=0;
            
            virtual void doAttaching(const std::list<ipair> & added,const std::list<ipair> & removed)=0;
            
            virtual const irr::core::matrix4 & getAbsoluteTransformation()=0;
            
            virtual void updateAbsolutePosition()=0;
            
            irr::scene::ISceneManager * scene;
            mods * m;
    };
    class subsaniStatic:public subsani{
        public:
            
            virtual void setLOD(float len);
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl);
            
            virtual const irr::core::vector3df & getPosition();
            virtual const irr::core::vector3df & getRotation();
            
            virtual void setPosition(const irr::core::vector3df &);
            virtual void setRotation(const irr::core::vector3df &);
            virtual void setDirection(const irr::core::vector3df &);
            virtual void setStatus(int id){}
            
            virtual void destruct();
            
            virtual void doAttaching(const std::list<ipair> & added,const std::list<ipair> & removed);
            
            virtual const irr::core::matrix4 & getAbsoluteTransformation();
            
            virtual void updateAbsolutePosition();
            
            subsaniStatic(
                irr::scene::ISceneManager * scene,
                mods::subsConf * sconf,
                mods * gconf,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r,
                const irr::core::vector3df & d
            );
            
        public:
            irr::scene::ISceneNode * node;
            mods::subsConf * conf;
    };
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    class subsaniChar:public subsani{//角色
        public:
            
            virtual void setLOD(float len);
            
            virtual const irr::core::vector3df & getPosition();
            virtual const irr::core::vector3df & getRotation();
            
            virtual void setPosition(const irr::core::vector3df &);
            virtual void setRotation(const irr::core::vector3df &);
            virtual void setDirection(const irr::core::vector3df &);
            virtual void setStatus(int id);
            
            virtual void playAnimation(float dtm,const irr::core::vector3df & dl);
            
            virtual void destruct();
            
            virtual void doAttaching(const std::list<ipair> & added,const std::list<ipair> & removed);
            
            virtual const irr::core::matrix4 & getAbsoluteTransformation();
            
            virtual void updateAbsolutePosition();
            
            subsaniChar(
                irr::scene::ISceneManager * scene,
                mods::subsConf * sconf,
                mods * gconf,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r,
                const irr::core::vector3df & d
            );
            
        private:
            irr::scene::IAnimatedMeshSceneNode * body;//物体贴body上
            std::map<irr::u32,irr::scene::IAnimatedMeshSceneNode*> items;//物体
            std::map<irr::u32,irr::scene::IAnimatedMeshSceneNode*> parts;//伴随body，如衣服等
            
            void doAniItem(irr::u32,int speed,int start,int end ,bool loop);
            void doAniPart(irr::u32,int speed,int start,int end ,bool loop);
            void doAni(int speed,int start,int end ,bool loop);
            void doAni(int id);
            void setItem(irr::u32,irr::scene::IAnimatedMesh*);
            void setPart(irr::u32,irr::scene::IAnimatedMesh*);
            void setBody(irr::scene::IAnimatedMesh*);
            void removeItem(irr::u32);
            void removePart(irr::u32);
            
            void mount(int p,int mesh);
            void umount(int p);
            /*
             * 骨骼挂载：
             * 1. id映射到真实骨骼id
             * 2. 获取mesh
             * 3. 挂载
            */
            void removeAll();
            
            void updateAnimation();
            
            bool walking;
            int status;
            float speed;
            
            static void doAni(irr::scene::IAnimatedMeshSceneNode*,int speed,int start,int end ,bool loop);
            
            mods::subsConf * conf;
    };
    
    subsani * subsaniFactory(
        irr::scene::ISceneManager * ,
        mods::subsConf *,
        mods * m,
        const irr::core::vector3df & p,
        const irr::core::vector3df & r,
        const irr::core::vector3df & d
    );
    
}
#endif