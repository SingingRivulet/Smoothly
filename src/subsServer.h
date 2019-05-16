#ifndef SMOOTHLY_SERVER_SUBS
#define SMOOTHLY_SERVER_SUBS
#include "cache.h"
#include "dblist.h"
#include "uuid.h"
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <pthread.h>
//c++11没有读写锁，故直接使用pthread
namespace smoothly{
    class subsServer{
        protected:
            leveldb::DB * db;
            pthread_rwlock_t rwlock;
            cache subsCache;
        public:
            struct subs:public cache::node{
                std::string manager;//临时管理者
                subsServer * parent;
                long id;
                int hp;
                int status;
                int lastChMan;
                std::string userUUID;
                std::string uuid;
                irr::core::vector3df position;
                irr::core::vector3df rotation;
                irr::core::vector3df direction;
                btVector3 lin_vel;
                btVector3 ang_vel;
                int x,y;
                void updateChunkPosition();
                void genUUID();
                void save(bool updateChunk=false,bool tp=false);
                void saveDo();
                void saveToDB();
                void load();
                void remove();
                void send();
                void send(const RakNet::SystemAddress & addr,bool sendconf=false);
                void removeFromChunk(int x,int y);
                void addIntoChunk(int x,int y);
                virtual void onFree();
                void encode(char * vbuf,int len);//encode to string
                void decode(const char * vbuf);//decode string
                void setConfig(const std::string & conf);
                void getConfig(std::string & conf);
                void delConfig();
                inline void checkPosition(){
                    if(position.X<-65535)
                        position.X=-65535;
                    else
                    if(position.X>65535)
                        position.X=65535;
                    
                    if(position.Z<-65535)
                        position.Z=-65535;
                    else
                    if(position.Z>65535)
                        position.Z=65535;
                    
                    if(position.Y<-2000)
                        position.Y=-2000;
                    else
                    if(position.Y>2000)
                        position.Y=2000;
                }
                inline subs():manager("NULL"){
                    lastChMan=0;
                    status=0;
                    id=0;
                    hp=1;
                    parent=NULL;
                }
            };
        public:
            void setSubs(
                const std::string & uuid,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & d, 
                const btVector3& lin_vel ,
                const btVector3& ang_vel ,
                int status,
                const std::string & muuid
            );//move a lasting substance
            void teleport(
                const std::string & uuid,
                const irr::core::vector3df & p,
                bool checkOwner,
                const std::string & owner
            );//传送，在客户端不会插值
            void createSubs(//添加物体
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & d, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & muuid,
                const std::string & config,
                const RakNet::SystemAddress & from
            );
            void createSubsForUSer(
                long id,
                const irr::core::vector3df & posi,
                const std::string & muuid,
                std::string & subsuuid
            );
            void giveUpControl(const std::string & uuid,const std::string & muuid);
            void changeManager(const std::string & uuid,const std::string & muuid);
            void removeSubs(const std::string & uuid);
            void attackSubs(const std::string & uuid,int dmg);
            void sendSubs(const RakNet::SystemAddress & addr,int x,int y);
            void sendSubs(const RakNet::SystemAddress & addr,const std::string & uuid);
            void getSubsPosition(irr::core::vector3df & posi,const std::string & uuid);
            
            inline void clearCache(){
                subsCache.clear();
            }
            
            virtual bool getSubsConf(long id , bool & isLasting , int & hp)=0;
            virtual void setOwner(const std::string & subsuuid,const std::string & useruuid)=0;
            virtual void removeFromOwner(const std::string & subsuuid,const std::string & useruuid)=0;
            virtual bool userCanPutSubs(const std::string & useruuid)=0;
            virtual void moveUserPosition(const std::string & uuid,const std::string & muuid,const irr::core::vector3df & posi)=0;
            
            virtual bool getUUIDByAddr(std::string & uuid , const RakNet::SystemAddress & address)=0;
            virtual bool getAddrByUUID(RakNet::SystemAddress & address , const std::string & uuid)=0;
            
            virtual void boardcastSubsCreate(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r ,
                const irr::core::vector3df & direction, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & useruuid,
                const std::string & config
            )=0;
            virtual void boardcastSubsCreate(
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & direction, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & useruuid,
                const std::string & config,
                const RakNet::SystemAddress & ext
            )=0;
            virtual void boardcastSubsStatus(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r ,
                const irr::core::vector3df & direction, 
                const btVector3& lin_vel,
                const btVector3& ang_vel,
                int status,
                int hp,
                const std::string & useruuid,
                const RakNet::SystemAddress & ext
            )=0;
            virtual void sendSubsStatus(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & direction, 
                const btVector3& lin_vel,
                const btVector3& ang_vel,
                int status,
                int hp,
                const std::string & useruuid,
                const std::string & config,
                const RakNet::SystemAddress & to
            )=0;
            virtual void boardcastSubsRemove(const std::string & subsuuid,const irr::core::vector3df & p)=0;
            virtual void boardcastSubsAttack(const std::string & subsuuid,const irr::core::vector3df & p,int hp,int delta)=0;
            virtual void sendPutSubsFail(const RakNet::SystemAddress & to)=0;
            virtual void boardcastTeleport(
                const std::string & subsuuid,
                const irr::core::vector3df & p
            )=0;
            virtual void sendChunkRun(int x,int y,const RakNet::SystemAddress & to)=0;
            virtual bool userOnline(const std::string & uuid)=0;
            
            virtual void delAttaching(const std::string & subs)=0;
        
        public:
            subs * seekSubs(const std::string & uuid);
        
        private:
            subs * createSubs(const std::string & uuid);
            bool uuidExist(const std::string & uuid);
            
        public:
            subsServer();
            ~subsServer();
            void subsInit(const char * path);
            void subsDestroy();
    };
}
#endif
