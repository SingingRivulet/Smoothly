#ifndef SMOOTHLY_BODY
#define SMOOTHLY_BODY
#include "terrainDispather.h"
#include <unordered_map>
#include <stdlib.h>
namespace smoothly{

class body:public terrainDispather{
    public:
        body();
        ~body();
    private:
        //覆盖网络处理
        virtual void msg_wearing_add(const char*,int d);
        virtual void msg_wearing_set(const char*,const std::set<int> &);
        virtual void msg_wearing_remove(const char*,int d);
        virtual void msg_HPInc(const char*,int d);
        virtual void msg_setStatus(const char*,int d);
        virtual void msg_setLookAt(const char*,float,float,float);
        virtual void msg_setPosition(const char*,float,float,float);
        virtual void msg_setRotation(const char*,float,float,float);
        virtual void msg_interactive(const char*,const char*);
        virtual void msg_removeBody(const char*);
        virtual void msg_createBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float);
        virtual void msg_setBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float,const std::set<int> &);
        virtual void msg_setMainControl(const char *);//设置第一人称的body

    public:
        typedef enum{//身体姿势掩码

            BM_WALK         = 1,            //行走(为0表示停止行走)

            //姿势，六选一
            BM_SQUAT        = 1<<1,         //蹲
            BM_SIT          = 1<<2,         //坐
            BM_RIDE         = 1<<3,         //骑
            BM_LIE          = 1<<1 & 1<<2,  //躺
            BM_LIEP         = 1<<1 & 1<<3,  //趴

            //手部姿势
            ////涉及的手
            BM_HAND_LEFT    = 1<<4,
            BM_HAND_RIGHT   = 1<<5,
            BM_HAND_BOTH    = 1<<4 & 1<<5,
            ////状态
            BM_AIM          = 1<<6,                 //瞄准
            BM_THROW        = 1<<7,                 //投掷瞄准
            BM_BUILD        = 1<<8,                 //建筑
            BM_BUILDP       = 1<<8 & 1<<6,          //准备建筑
            BM_OPERATE      = 1<<8 & 1<<7,          //操作机器
            BM_LIFT         = 1<<8 & 1<<7 & 1<<6,   //举近战武器

            BM_ACT_SHOT_L   = 1<<9,                 //左射击
            BM_ACT_SHOT_R   = 1<<10,                //右射击
            BM_ACT_THROW    = 1<<11,                //投掷
            BM_ACT_CHOP     = 1<<12                 //砍

        }bodyStatusMask;

        struct bodyStatus{
            bool walking;
            typedef enum{
                BS_BODY_STAND   = 0,
                BS_BODY_SQUAT   = 1<<1,
                BS_BODY_SIT     = 1<<2,
                BS_BODY_RIDE    = 1<<3,
                BS_BODY_LIE     = 1<<1 & 1<<2,
                BS_BODY_LIEP    = 1<<1 & 1<<3
            }bodyPosture_t;
            typedef enum{
                BS_HAND_NONE    = 0,
                BS_HAND_AIM     = 1<<6,
                BS_HAND_THROW   = 1<<7,
                BS_HAND_BUILD   = 1<<8,
                BS_HAND_BUILDP  = 1<<8 & 1<<6,
                BS_HAND_OPERATE = 1<<8 & 1<<7,
                BS_HAND_LIFT    = 1<<8 & 1<<7 & 1<<6
            }handPosture_t;
            bodyPosture_t bodyPosture;
            handPosture_t handPosture;
            bool useLeft,useRight,shotLeft,shotRight,throwing,chop;

            int32_t toMask()const;
            void loadMask(int32_t);
            bodyStatus();
            bodyStatus(const bodyStatus &);
            bodyStatus(int32_t);
            const bodyStatus & operator=(const bodyStatus &);
            const bodyStatus & operator=(int32_t);

        };

    private:
        struct bodyItem;
        class bodyAmCallback:public irr::scene::IAnimationEndCallBack{
            public:
                void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode *);
                bodyItem * parent;
        };
        struct bodyItem{
            character                            m_character;
            bodyStatus                           status;
            int32_t                              status_mask;
            int                                  id,hp;
            std::string                          uuid,owner;
            irr::scene::IAnimatedMeshSceneNode * node;
            vec3                                 lookAt;
            body                               * parent;
            std::map<int,irr::scene::IAnimatedMeshSceneNode*> wearing;
            bodyItem(btScalar w,btScalar h,const btVector3 & position,bool wis,bool jis);
            void updateFromWorld();
            void updateStatus(bool finish = false);
            void doAnimation(int speed,int start,int end ,bool loop);
        };
        lua_State * L;//
        std::unordered_map<std::string,bodyItem*> bodies;
        void setBodyPosition(const std::string & uuid , const vec3 & posi);
        void removeBody(const std::string & uuid);
        void releaseBody(bodyItem*);

        void addBody(const std::string & uuid,int id,int hp,int32_t sta_mask,const std::string & owner,const vec3 & p,const vec3 & r,const vec3 & l);
        void setWearing(bodyItem * , const std::set<int> & wearing);
        void addWearing(bodyItem *, int wearing);
        void removeWearing(bodyItem *, int wearing);

        void addWearingNode(bodyItem *, int wearing);

    private:
        struct bodyConf{
            float width,height;
            bool walkInSky,jumpInSky;
            irr::scene::IAnimatedMesh * mesh;
            std::string aniCallback;
        };
        std::map<int,bodyConf*> bodyConfig;

        struct wearingConf{
            std::string attach;
            irr::scene::IAnimatedMesh * mesh;
        };
        std::map<int,wearingConf*> wearingConfig;

        void loadBodyConfig();
        void loadWearingConfig();

        void releaseBodyConfig();
        void releaseWearingConfig();

    public:
        static void setPositionByTransform(irr::scene::ISceneNode * n , const btTransform & t);

};

}
#endif // SMOOTHLY_BODY
