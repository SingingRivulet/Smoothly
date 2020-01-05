#ifndef SMOOTHLY_FIRE
#define SMOOTHLY_FIRE
#include "body.h"
namespace smoothly{

class fire:public body{
    public:
        fire();
        void fireTo(const std::string & uuid , int id , const vec3 & from , const vec3 & dir);
    private:
        virtual void msg_fire(const char *,int,float,float,float,float,float,float);
        void fireTo_act(const std::string & uuid , int id , const vec3 & from , const vec3 & dir,bool attack = false);

    private:

        typedef enum{
            FIRE_STREAM,//射流
            FIRE_SHOOT, //射击
            FIRE_CHOP,  //砍
            FIRE_RADIO, //向周围辐射
            FIRE_LASER  //激光
        }fire_type;

        typedef enum{
            PTL_POINT_TARGET,       //由点向目标发射
            PTL_POINT,              //由点向周围发射
            PTL_LINE                //由线向周围发射
        }Particle_type;

        struct fireConfig{

            fire_type   type;
            float       range;          //范围(砍，激光，辐射)
            int         lifeTime;       //动画生命周期
            float       radius;         //半径（子弹）
            bool        breakWhenHit;   //击中后消失（子弹）

            int         streamParticleNum;
            float       streamParticleVelocity;

            btConvexShape * castShape;  //碰撞体
            shapeGroup      shape;      //子弹形状

            struct{
                bool have;
                struct{
                    bool have;
                    vec3 gravity;
                    unsigned int  timeForceLost;
                }gravity;//重力

                irr::video::SColor minStartColor;
                irr::video::SColor maxStartColor;

                unsigned int lifeTimeMin;
                unsigned int lifeTimeMax;

                unsigned int minParticlesPerSecond;
                unsigned int maxParticlesPerSecond;

                irr::core::dimension2df minStartSize;
                irr::core::dimension2df maxStartSize;

                unsigned int maxAngleDegrees;

                irr::video::ITexture * texture;

                bool light;

            }particleConfig;//粒子效果配置

            struct{
                bool have;
                irr::scene::IAnimatedMesh * mesh;
                bool billboardMode;
                irr::video::ITexture * texture;
                bool light;
            }bulletConf;

            fireConfig();
        };

        void releaseConfig(fireConfig *);

        std::map<int,fireConfig*> config;

    private:
        struct bullet{
            bullet();
            bodyInfo info;
            btRigidBody                 * rigidBody;
            btMotionState               * bodyState;
            fire                        * parent;
            irr::scene::ISceneNode      * node;
            bool                          attack;
            std::string                   owner;
            fireConfig                  * config;
        };
        struct emitter{
            fireConfig *    config;
            int             leave;
            std::string     uuid;
        };
        std::list<emitter*> emitters;
        std::list<bullet*>  bullets;

        void shoot(const std::string & uuid , fireConfig * , const vec3 & from , const vec3 & dir,bool attack = false);
};

}
#endif
