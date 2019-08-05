#ifndef SMOOTHLY_ATTACK
#define SMOOTHLY_ATTACK
#include "utils.h"
namespace smoothly{
    class attackInter{
        public:
            /*
             * 只有substance才能发起攻击
             * 总共有三个攻击上传函数
                void uploadAttack(const std::string & uuid , int hurt);     //对建筑
                void requestAttackSubs(const std::string & uuid,int dmg);   //对物理物体
                void requestRemoveItem(const mapid & mid);                  //对地形物体
             * 物理物体和建筑采用uuid表示，地形物体采用(区块x，区块y，物体id，物体编号)表示
             * 地形物体采用量子化设计，即达到伤害要求直接破坏，达不到怎么攻击都没用，以节省remove table空间。
             *******************************************************************************
             * 攻击时，先在irrlicht上createDeleteAnimator，然后创建攻击动画，并上传攻击动画
             * 然后在bullet上检测被攻击的目标，上传
             * 物理世界中，激光由射线检测处理
             * 
             * note:-1是保留值，表示没有武器
            */
            
            //从mods类中获取攻击配置信息，并调用addAttackAm系列函数
            //dir的模是发射距离
            virtual void doAttackAm(
                const irr::core::vector3df & from,
                const irr::core::vector3df & dir,
                int id,
                irr::scene::IAnimatedMeshSceneNode *
            )=0;
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////
            
            class attackAmBase{
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    )=0;
                    virtual void init(const std::map<std::string,std::string> & )=0;
                    virtual void destroy()=0;
                    
                    static inline void setVanishingTime(irr::scene::ISceneNode * node,irr::scene::ISceneManager * scene,int tm){//设置指定时间后消失
                        auto delAct=scene->createDeleteAnimator(tm);
                        node->addAnimator(delAct);
                        delAct->drop();
                    }
            };
            //添加攻击动画，所有attackAm由下面几种组成，但是攻击只能是单个类型
            
            class attackAmLaser:public attackAmBase{
                //激光
                //长条形模型加着色器组成
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    );
                    virtual void destroy();
                    
                    irr::video::ITexture* texture;//纹理
                    int delayMs;
            };
            
            class attackAmCloseComb:public attackAmBase{
                //近战
                //粒子发射器贴在武器上
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    );
                    virtual void destroy();
                    
                    irr::video::SColor minStartColor,maxStartColor;
                    int delayMs;
                    float particleSpeed;//粒子速度
                    irr::u32 minParticlesPerSecond,
                             maxParticlesPerSecond,
                             lifeTimeMin,
                             lifeTimeMax;
            };
            
            class attackAmExplode:public attackAmBase{
                //爆炸
                //粒子发射器组成
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    );
                    virtual void destroy();
                    
                    irr::video::SColor minStartColor,maxStartColor;
                    int delayMs;
                    float particleSpeed;//粒子速度
                    irr::u32 minParticlesPerSecond,
                             maxParticlesPerSecond,
                             lifeTimeMin,
                             lifeTimeMax;
                    int      timeForceLost;
            };
            
            class attackAmShot:public attackAmBase{
                //子弹射击
                //仅播放发射动画，弹道在brief substance中实现
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    );
                    virtual void destroy();
                    
                    irr::video::SColor minStartColor,maxStartColor;
                    int delayMs;
                    float particleSpeed;//粒子速度
                    int      maxAngleDegrees;
                    irr::u32 minParticlesPerSecond,
                             maxParticlesPerSecond,
                             lifeTimeMin,
                             lifeTimeMax;
            };
            /*
             * 可使用大量子弹代替
            class attackAmStream:public attackAmBase{
                //射流
                //粒子发射器与flyStraightAnimator组成，到终点后停留一段时间
                public:
                    virtual void add(
                        irr::scene::ISceneManager * scene,
                        irr::scene::IAnimatedMeshSceneNode *,
                        const irr::core::vector3df & from,
                        const irr::core::vector3df & dir
                    );
                    virtual void destroy();
            };
            */
            ///////////////////////////////////////////////////////////////////////////////////////////////////////
            
            //上传攻击消息
            virtual void uploadAttackAm(
                const std::string & uuid,
                const irr::core::vector3df & dir,
                uint32_t bid,
                int id
            )=0;
    };
}
#endif