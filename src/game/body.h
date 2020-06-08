#ifndef SMOOTHLY_BODY
#define SMOOTHLY_BODY
#include "technology.h"
#include "bone.h"
#include <unordered_map>
#include <functional>
#include <stdlib.h>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
namespace smoothly{

class body:public technology{
    public:
        body();
        ~body();
    private:
        //覆盖网络处理
        void msg_setVisualRange(int v)override;
        void msg_wearing_add(const char*,int d)override;
        void msg_wearing_set(const char*,const std::set<int> &)override;
        void msg_wearing_remove(const char*,int d)override;
        void msg_HPInc(const char*,int d)override;
        void msg_setStatus(const char*,int d)override;
        void msg_setLookAt(const char*,float,float,float)override;
        void msg_setPosition(const char*,float,float,float)override;
        void msg_setRotation(const char*,float,float,float)override;
        void msg_interactive(const char*,const char*)override;
        void msg_removeBody(const char*)override;
        void msg_createBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float)override;
        void msg_setBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float,const std::set<int> &)override;
        void msg_setMainControl(const char *)override;//设置第一人称的body
        void msg_setBag(const char *,const char *);
        void msg_setBagResource(const char *,int,int);
        void msg_setBagTool(const char *,const char *)override;
        void msg_setBagToolDur(const char *,int,int)override;
        void msg_bag_tool_add(const char *,const char *)override;
        void msg_bag_tool_remove(const char *,const char *)override;
        void msg_bag_tool_use(const char *,const char *)override;

    public:
        typedef enum:int32_t{//身体姿势掩码

            BM_VANISH       = 1,

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
            BM_ACT_CHOP     = 1<<12,                //砍

            BM_WALK_F       = 1<<13,            //前行走
            BM_WALK_B       = 1<<14,            //后行走
            BM_WALK_L       = 1<<15,            //左行走
            BM_WALK_R       = 1<<16             //右行走

        }bodyStatusMask;

        struct bodyStatus{
            int walk_forward;
            int walk_leftOrRight;

            typedef enum:int32_t{
                BS_BODY_STAND   = 0,
                BS_BODY_SQUAT   = 1<<1,
                BS_BODY_SIT     = 1<<2,
                BS_BODY_RIDE    = 1<<3,
                BS_BODY_LIE     = 1<<1 & 1<<2,
                BS_BODY_LIEP    = 1<<1 & 1<<3
            }bodyPosture_t;

            typedef enum:int32_t{
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

    public:
        class bodyItem;
        void onDraw()override;

    private:
        struct bodyConf;
        class bodyAmCallback:public irr::scene::IAnimationEndCallBack{
            public:
                void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode *);
                bodyItem * parent;
        };
        struct wearingBullet{
            int id,deltaTime,reloadTime;
            inline wearingBullet(){}
            inline wearingBullet(int i , int d , int r){
                id          =   i;
                deltaTime   =   d;
                reloadTime  =   r;
            }
            inline wearingBullet(const wearingBullet & w){
                id          =   w.id;
                deltaTime   =   w.deltaTime;
                reloadTime  =   w.reloadTime;
            }
        };
        std::map<int,wearingBullet> wearingToBullet;

        btVector3 gravity;

    public:
        class bodyItem{
            friend class body;
            public:
                character                            m_character;
                bodyStatus                           status;
                int32_t                              status_mask,lastStatus;
                int                                  id,hp;
                std::string                          uuid,owner;
                irr::scene::IAnimatedMeshSceneNode * node;
                vec3                                 lookAt;
                body                               * parent;
                bodyConf                           * config;
                std::map<int,irr::scene::IAnimatedMeshSceneNode*> wearing;
                bodyInfo                             info;
                bool                                 uncreatedChunk;//未创建区块，锁住物体

                std::map<int,int>                    resources;
                std::unordered_set<std::string>      tools;
                int                                  weight,maxWeight;
                std::string                          usingTool;
                void loadBag(const char * str);

                btRigidBody      * coll_rigidBody;
                btMotionState    * coll_bodyState;
                bodyInfo           coll_rigidBodyInfo;

                bool firing;
                int firingWearingId;
                int fireId;
                int lastPitchAngle;
                unsigned int lastFireTime;
                unsigned int fireDelta;

                void doFire();
                void reloadTool();
                void reloadStart();
                void reloadEnd();
                irr::u32 reloadStartTime;
                irr::u32 reloadNeedTime;
                int reloading;

                void ghostTest(const btTransform &,std::function<void(bodyInfo*)> callback);//假如此角色在哪个位置，会碰撞哪些物体

                std::list<vec3> autoWalk;

                irr::core::vector2d<s32> screenPosition;

                void setFollow(bodyItem *);//设置跟随

                float getPitchAngle();

                irr::core::array<irr::scene::IAnimatedMeshSceneNode::IAnimationBlend> animationBlend;//动画混合器

                inline void setBlenderAsDefault(){
                    animationBlend.clear();
                    animationBlend.insert(scene::IAnimatedMeshSceneNode::IAnimationBlend((irr::scene::ISkinnedMesh*)config->mesh,1.0));
                }
                void make(int id);

            protected:
                bodyItem                           * follow;//跟随
                std::set<bodyItem*>                  followers;
                bodyItem(btScalar w,btScalar h,const btVector3 & position,bool wis,bool jis);
                void updateFromWorld();
                void updateStatus(bool finish = false);
                void doAnimation(float speed, int start, int end , float frame, bool loop);
                void interactive(const char *);
                void walk(int forward,int leftOrRight/*-1 left,1 right*/,float speed);

                //向服务器发送数据
                void interactive_c(const std::string & );
                void wearing_add_c(int d);
                void wearing_remove_c(int d);
                void wearing_clear_c();

                vec3 lastPosition,lastRotation,lastLookAt;
                class JointCallback:public irr::scene::ISkinnedMesh::ISkinnedMeshAnimationOverrideCallback{
                    public:
                        bool getFrameData(irr::f32 frame, irr::scene::ISkinnedMesh::SJoint *joint,
                                            irr::core::vector3df &position, irr::s32 &positionHint,
                                            irr::core::vector3df &scale, irr::s32 &scaleHint,
                                            irr::core::quaternion &rotation, irr::s32 &rotationHint)override;
                }jointCallback;

            public:
                void setLookAt(const vec3 &);

                void HP_inc_c(int d);//设置body血量
        };

    protected:
        lua_State * L;//
    public:
        std::unordered_map<std::string,bodyItem*> bodies;
        std::unordered_map<std::string,bodyItem*> myBodies;
        std::set<bodyItem*> selectedBodies;
        void selectBodyByScreenPoint(const irr::core::vector2d<s32> &,int range);
        void selectBodyStart();
        void selectBodyEnd();
    private:
        void selectBodyUpdate();
        int selectBodyStartTime;
        bool selecting;
        int selectBodyRange;
        bool selectAllBodies;
    protected:
        void setBodyPosition(const std::string & uuid , const vec3 & posi);
        void removeBody(const std::string & uuid);
        void releaseBody(bodyItem*);

        void addBody(const std::string & uuid,int id,int hp,int32_t sta_mask,const std::string & owner,const vec3 & p,const vec3 & r,const vec3 & l);
        void setWearing(bodyItem * , const std::set<int> & wearing);
        void addWearing(bodyItem *, int wearing);
        void removeWearing(bodyItem *, int wearing);

        bool addWearingNode(bodyItem *, int wearing);

    private:
        struct bodyConf{
            float width,height,deltaY,jump;
            bool walkInSky,jumpInSky;
            bool teleport;//传送功能(通过指挥功能使用)
            irr::scene::IAnimatedMesh * mesh;
            int aniCallback;
            bool haveAniCallback;
            float walkVelocity;
            irr::video::ITexture * texture;
            std::vector<irr::scene::ISkinnedMesh*> animation;//用于混合的动画
            inline bodyConf(){
                mesh         = NULL;
                texture      = NULL;
                walkInSky    = false;
                jumpInSky    = false;
                width        = 1;
                height       = 1;
                deltaY       = 1;
                walkVelocity = 0.003;
                jump         = 10;
                teleport     = false;
                haveAniCallback = false;
            }
        };
        std::map<int,bodyConf*> bodyConfig;

        struct wearingConf{
            std::map<int,std::string>   attach;
            irr::scene::IAnimatedMesh * mesh;
            irr::video::ITexture      * texture;
            vec3                        skillFrom;//从这个点开始放技能
            inline wearingConf(){
                mesh    =NULL;
                texture =NULL;
                attach.clear();
            }
        };
        std::map<int,wearingConf*> wearingConfig;

        void loadBodyConfig();
        void loadWearingConfig();

        void releaseBodyConfig();
        void releaseWearingConfig();

    public:
        static void setPositionByTransform(irr::scene::ISceneNode * n , const btTransform & t);
        virtual void setInteractiveNode(bodyItem * b , const std::string & method)=0;
        virtual void fireTo(const std::string & uuid , int id , const vec3 & from , const vec3 & dir)=0;
        virtual void loop()override;
        bodyItem * seekBody(const std::string &);
        bodyItem * seekMyBody(const std::string &);
        bodyItem * mainControlBody;

    public:
        //commond
        typedef enum{
            CMD_SET_LOOKAT,
            CMD_SET_POSITION,   //直接设置位置，慎用
            CMD_SET_ROTATION,
            CMD_JUMP,
            CMD_STATUS_SET,
            CMD_STATUS_ADD,     //添加掩码
            CMD_STATUS_REMOVE,  //删除掩码
            CMD_INTERACTIVE,
            CMD_WEARING_ADD,
            CMD_WEARING_REMOVE,
            CMD_WEARING_CLEAR,
            CMD_FIRE_BEGIN,
            CMD_FIRE_END,
            CMD_TOOL_RELOAD_START,
            CMD_TOOL_RELOAD_END
        }bodyCmd_t;
        struct commond{
            std::string uuid;
            bodyCmd_t   cmd;      //指令
            int32_t     data_int;
            std::string data_str;
            vec3        data_vec;
            commond();
            commond(const commond &);
        };
        void pushCommond(const commond &);
    private:
        void doCommond(const commond &);
        std::mutex cmdQueue_locker;
        std::queue<commond> cmdQueue;
        void doCommonds();

    private:
        struct tool{
                int id;
                int dur;
                int pwr;
                inline tool(){
                    dur = 0;
                    id  = 0;
                    pwr = 0;
                }
                inline tool(const tool & i){
                    dur = i.dur;
                    id  = i.id;
                    pwr = i.pwr;
                }
                inline const tool & operator=(const tool & i){
                    dur = i.dur;
                    id  = i.id;
                    pwr = i.pwr;
                    return * this;
                }
                void loadStr(const char *);
        };
        std::unordered_map<std::string,tool> tools;
        irr::gui::IGUISpriteBank * bag_icons;
        std::map<int,irr::s32> bag_res_icons_mapping,bag_tool_icons_mapping;
        void loadBagIcons();
        void make(int id)override;

    private:
        struct fire_cost{
                int id;
                int cost_id;//开火消耗
                int cost_num;
                int get_id;//开火获得
                int get_num;
                int dur_cost;
                int pwr_cost;
                inline fire_cost(){
                    id          = 0;
                    cost_id     = 0;
                    cost_num    = 0;
                    get_id      = 0;
                    get_num     = 0;
                    dur_cost    = 0;
                    pwr_cost    = 0;
                }
                inline fire_cost(const fire_cost & i){
                    id          = i.id;
                    cost_id     = i.cost_id;
                    cost_num    = i.cost_num;
                    get_id      = i.get_id;
                    get_num     = i.get_num;
                    dur_cost    = i.dur_cost;
                    pwr_cost    = i.pwr_cost;
                }
                inline const fire_cost & operator=(const fire_cost & i){
                    id          = i.id;
                    cost_id     = i.cost_id;
                    cost_num    = i.cost_num;
                    get_id      = i.get_id;
                    get_num     = i.get_num;
                    dur_cost    = i.dur_cost;
                    pwr_cost    = i.pwr_cost;
                    return * this;
                }
        };
        std::map<int,fire_cost> fire_costs;
        void loadFireCost();
        void updateBagUI();
        struct hand_t{
                bool isTool;
                std::string toolUUID;
                int resourceId;
                hand_t(const std::string & uuid) {
                    isTool = true;
                    toolUUID = uuid;
                }
                hand_t(int rid) {
                    isTool = false;
                    resourceId = rid;
                }
                hand_t(const hand_t & h){
                    isTool = h.isTool;
                    toolUUID = h.toolUUID;
                    resourceId = h.resourceId;
                }
        };
        std::vector<hand_t> handItems;//可以使用的物体
        bool usingResource;
        int usingResource_id;
        int bag_selectId;

    public:
        void useTool(int id);
        void dropHand();
        void bag_selectLast();
        void bag_selectNext();
        void pickupPackageToBag(int x,int y,const std::string & uuid)override;
        bool needUpdateUI;
        int bagPage;
        irr::gui::IGUIListBox * body_bag_resource , * body_bag_using;
        irr::gui::IGUIStaticText * body_bag_page;

    private:
        std::string defaultMainControl;
    public:
        void setMainControl(const std::string & uuid);
        inline void resetMainControl(){
            setMainControl(defaultMainControl);
        }
        inline void controlSelectedBody(){
            auto len = selectedBodies.size();
            if(len==0){
                resetMainControl();
            }else{
                if(len==1){
                    auto it = selectedBodies.begin();
                    if(it!=selectedBodies.end()){
                        if((*it)->uuid==mainControl)
                            resetMainControl();
                        else
                            setMainControl((*it)->uuid);
                    }
                }
            }
        }

};

}
#endif // SMOOTHLY_BODY
