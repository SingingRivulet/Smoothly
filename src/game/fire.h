#ifndef SMOOTHLY_FIRE
#define SMOOTHLY_FIRE
#include "interactive.h"
namespace smoothly{

class fire:public interactive{
    /*
     * 先执行完攻击动作，然后worldLoop
     */
    public:
        fire();
        ~fire();
        void fireTo(const std::string & uuid , int id , const vec3 & from , const vec3 & dir)override;

    private:
        void openConfig();
        void closeConfig();

    private:

        lua_State * L;
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
            int id;

            fire_type   type;
            float       range;          //范围(砍，激光，辐射)
            int         lifeTime;       //动画生命周期
            float       radius;         //半径（子弹）
            bool        breakWhenHit;   //击中后消失（子弹）

            irr::core::array<irr::video::ITexture*> textures;
            irr::s32                                timePerFrame;

            int         streamParticleNum;
            float       streamParticleVelocity;

            float       mass;
            btVector3   inertia;

            btConvexShape * castShape;  //碰撞体
            shapeGroup      shape;      //子弹形状

            float       impulse;//发射子弹的冲量
            float       billboardSize;//billboard模式下子弹的大小

            audioBuffer * startAudio;//释放技能时的声音
            audioBuffer * flyAudio;//飞行时的声音（仅子弹）
            audioBuffer * endAudio;//飞行时的声音（仅子弹）

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
            std::string                   owner;//拥有这颗子弹的单位
            fireConfig                  * config;
            unsigned int                  expire;
            audioSource                 * audio;
            void update();
        };
        struct emitter{
            fireConfig *    config;
            int             leave;
            std::string     uuid;
            int             lastProcess;
            bool            attack;
        };
        std::set<emitter*> emitters;
        std::set<bullet*>  bullets;
        std::set<bullet*>  bulletRemove;

        void shoot(const std::string & uuid , fireConfig * , const vec3 & from , const vec3 & dir,bool attack = false);

        void attackBody(const std::string & uuid , fireConfig * conf , bodyInfo * body , float force = 1);

        int  attackBody(const std::string & uuid , fireConfig * conf , bodyItem * , float force = 1);
        bool attackTerrainItem(const std::string & uuid , fireConfig * conf , mapId * , float force = 1);
        void attackTerrain(const vec3 & pos);

        void bulletAttackBody(bullet * b , const btCollisionObject * body);
        void releaseBullet(bullet * b);

        bool processEmitter(emitter * em);
        void releaseEmitter(emitter * em);

        void processBullets();
        void processBulletRemove();

        void onCollision(bodyInfo * A,bodyInfo * B,const btManifoldPoint & point);
        virtual void onCollision(btPersistentManifold *contact)override;

        int attackBuilding(const std::string & uuid, fireConfig * conf, buildingBody * b, float force);

        irr::scene::IMesh * laserMesh;
        void createLaserMesh();

        void loadAPIs();
    public:

        static bodyItem * getBodyFromBodyPart(bodyInfo*);
        void worldLoop()override;
};

}
#endif
