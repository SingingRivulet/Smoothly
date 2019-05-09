#ifndef SMOOTHLY_SUBSTANCE
#define SMOOTHLY_SUBSTANCE
#include "building.h"
#include "mempool.h"
#include <mutex>
#include <queue>
namespace smoothly{
    class substance:public buildings{
        public:
            typedef mods::subsType subsType;
            struct subs;
            typedef std::pair<subs*,float> subsContact;
            
            struct subs{
                
                std::string uuid;
                std::string owner;
                long id;
                int status,hp;
                
                float walkSpeed;
                float liftForce;
                float pushForce;
                float jumpImp;
                float deltaCamera;
                
                bodyInfo info;
                
                bodyBase * body;
                
                irr::scene::ISceneNode * node;
                
                mods::subsConf * subsConf;
                
                subsType type;
                
                subs * next;
                substance * parent;
                
                bool noFallDown;
                
                bool inWorld;
                int x,y;//this variable only will be use while initializate;
                
                irr::core::vector3df direction;
                
                irr::core::vector2df walkDirection;
                
                void moveUpdate(int forward,int leftOrRight);
                
                void setPowerAsDefault();
                
                inline void setNoFallDown(){
                    noFallDown=true;
                    body->setAngularFactor(btVector3(0,1,0));
                }
                
                void updateByWorld();
                void setMotion(const irr::core::vector3df & p,const irr::core::vector3df & r);
                void setPosition(const irr::core::vector3df & p);
                void setRotation(const irr::core::vector3df & r);
                void teleport(const irr::core::vector3df & p);
                void move(const irr::core::vector3df & d);
                void walk(const irr::core::vector3df & d);
                
                void fly(float lift,float push);
                void fly(float lift,const irr::core::vector3df & push);
                void jump(const irr::core::vector3df & d);
                void walk(const irr::core::vector2df & d);
                void flyUpdate(bool flying,bool lifting);
                
                void walk(int forward,int leftOrRight/*-1 left,1 right*/,float length);
                
                irr::core::vector3df getDirection();
                void setDirection(const irr::core::vector3df & d);
                
                inline btVector3 getAngularVelocity(){
                    return body->getAngularVelocity();
                }
                inline btVector3 getLinearVelocity(){
                    return body->getLinearVelocity();
                }
                
                void setRandUUID();
                void init(mods::subsConf *,const irr::core::vector3df & p,const irr::core::vector3df & r);
                void release();
                
                void setAsBrief(int life);
                void checkPosition();
                void update();
                
                bool addIntoWorld();

            };
            
            inline mods::subsConf * seekSubsConf(long id){
                auto it=m->subsConfs.find(id);
                if(it==m->subsConfs.end())
                    return NULL;
                else
                    return it->second;
            }
            
            std::unordered_map<std::string,subs*> subses;
            std::set<std::string> subsRMT;//remove table
            
            //非持久物体管理器
            struct briefTime{
                std::string uuid;
                int createTime;
                int life;
                inline briefTime(const std::string & u,int t,int l){
                    uuid=u;
                    createTime=t;
                    life=l;
                }
                inline briefTime(const briefTime & b){
                    uuid=b.uuid;
                    createTime=b.createTime;
                    life=b.life;
                }
            };
            std::list<briefTime> briefSubs;//非持久物体表
            void clearDiedSubs();//清除到期的物体
            
            inline subs * seekSubs(const std::string & uuid){
                auto it=subses.find(uuid);
                if(it==subses.end())
                    return NULL;
                else
                    return it->second;
            }
            
            //本地直接调用的删除函数
            void removeSubs(subs * );//删除物体（调用removeLocalSubs）
            inline void removeSubs(const std::string & uuid){
                auto p=seekSubs(uuid);
                if(p)
                    removeSubs(p);
            }
            
            //更新
            void removeApply();//执行删除列表
            void subsUpdate();
            
            //添加物体
            subs * addLastingSubs(//添加持久物体
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos
            );
            subs * addBriefSubs(//添加非持久物体
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos
            );
            
            
        public:
            void genSubs(//添加物体（持久），由服务器调用
                const std::string & uuid ,
                const std::string & owner ,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3 & impulse,
                const btVector3 & rel_pos
            );
            void genSubs(//添加物体（非持久）
                long id , 
                const std::string & owner ,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos
            );
            
            void updateSubs(//更新物体状态，由服务器调用
                long id,
                const std::string & uuid , 
                const std::string & owner ,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& lin_vel ,
                const btVector3& ang_vel ,
                int hp,int status,
                const std::string & conf
            );
            
            
            inline void removeLocalSubs(const std::string & uuid){//删除本地对象（仅添加至删除表）
                subsRMT.insert(uuid);
            }
            
            inline void attackLocalSubs(const std::string & uuid,int nhp,int delta){
                auto p=seekSubs(uuid);
                if(p){
                    p->hp=nhp;
                    if(p->hp <= 0)
                        subsRMT.insert(uuid);
                }
            }
            
            void onCollision(bodyInfo *,bodyInfo *,const btManifoldPoint &);
            
            virtual void requestRemoveSubs(const std::string & uuid)=0;//请求删除持久物体（非持久不需要删除）
            virtual void requestTeleport(const std::string & uuid,const irr::core::vector3df & p)=0;
            virtual void requestAttackSubs(const std::string & uuid,int dmg)=0;
            
            virtual void uploadBodyStatus(//上传持久型物体状态
                const std::string & uuid , 
                int status,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& lin_vel ,
                const btVector3& ang_vel
            )=0;
            
            virtual void requestCreateBriefSubs(//请求创建物体（非持久）
                long id,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos
            )=0;
            virtual void requestCreateLastingSubs(//请求创建物体（持久）
                long id,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos
            )=0;
            virtual void requestDownloadSubstanceChunk(int x,int y)=0;
        public:
            
            inline substance():briefSubs(),subses(),subsRMT(){
                subsPoolInit();
                deltaTime=0;
            }
            inline ~substance(){
                subsPoolDestroy();
            }
            
        public:
            std::unordered_set<std::string> mySubs;
            std::string myUUID;
            std::string mainControlUUID;
            inline void setOwner(const std::string & subs,const std::string & user){
                if(myUUID==user)
                    mySubs.insert(subs);
            }
            inline void removeOwned(const std::string & subs,const std::string & user){
                if(myUUID==user)
                    mySubs.erase(subs);
            }
            irr::core::vector3df mainControlPosition;
            float deltaCamera;
        private:
            std::map<ipair,std::set<subs*> > chunkLocker;
            virtual void onGenChunk(terrain::chunk *);
            virtual void onFreeChunk(terrain::chunk *);
            
        public:
            void lockChunk(int x,int y);
            void unlockChunk(int x,int y);
            
        private:
            void * sPool;
            subs * createSubs();
            void delSubs(subs *);
            void subsPoolInit();
            void subsPoolDestroy();
            
        public:
            float deltaTime;
            
            struct subsCommond{
                std::string uuid;
                enum Method{
                    JUMP,
                    WALK,
                    WALK_STOP,
                    FLY,
                    FLY_STOP,
                    DIRECT
                };
                Method method;
                int walkForward;
                int walkLeftOrRight;
                irr::core::vector3df vec;
                bool flying;
                bool lifting;
            };
            void pushSubsCommond(const subsCommond & cmd);
        private:
            std::mutex commondLocker;
            std::queue<subsCommond> cmdQueue;
            void parseAllCommond();
            void parseCommond(const subsCommond & cmd);
    };
}
#endif
