#ifndef SMOOTHLY_SUBSTANCE
#define SMOOTHLY_SUBSTANCE
#include "building.h"
#include "mempool.h"
namespace smoothly{
    class substance:public buildings{
        public:
            typedef mods::subsType subsType;
            
            struct subs{
                
                std::string uuid;
                std::string owner;
                long id;
                int status,hp;
                bodyInfo info;
                btRigidBody      * rigidBody;
                btMotionState    * bodyState;
                irr::scene::ISceneNode * node;
                
                subsType type;
                
                subs * next;
                substance * parent;
                
                bool wake;
                
                void updateByWorld();
                void setMotion(const irr::core::vector3df & p,const irr::core::vector3df & r);
                void setPosition(const irr::core::vector3df & p);
                void teleport(const irr::core::vector3df & p);
                
                inline const btVector3 & getAngularVelocity(){
                    return rigidBody->getAngularVelocity();
                }
                inline const btVector3 & getLinearVelocity(){
                    return rigidBody->getLinearVelocity();
                }
                
                void setRandUUID();
                void init(mods::subsConf *,const irr::core::vector3df & p,const irr::core::vector3df & r);
                void release();
                
                void setAsBrief(int life);
                
                void update();

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
            void setSubs(//设置物体（持久），由服务器调用
                const std::string & uuid ,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r,  
                const btVector3& lin_vel ,
                const btVector3& ang_vel
            );
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
                int hp,int status
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
            
            virtual void requestRemoveSubs(const std::string & uuid)=0;//请求删除持久物体（非持久不需要删除）
            
            virtual void uploadBodyStatus(//上传持久型物体状态
                const std::string & uuid , 
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
            
        public:
            
            inline substance():briefSubs(),subses(),subsRMT(){
                subsPoolInit();
            }
            inline ~substance(){
                subsPoolDestroy();
            }
            
        private:
            void * sPool;
            subs * createSubs();
            void delSubs(subs *);
            void subsPoolInit();
            void subsPoolDestroy();
            
    };
}
#endif
