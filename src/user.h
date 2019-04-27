#ifndef SMOOTHLY_USER
#define SMOOTHLY_USER
#include "subsServer.h"
#include "nameLocker.h"
#include <map>
#include <atomic>
namespace smoothly{
    class users:public subsServer{
        protected:
            leveldb::DB * db;
            pthread_rwlock_t rwlock;
            struct user{
                std::string subsUUID;
                std::string userUUID;
                RakNet::SystemAddress address;
                users * parent;
                int subsNum;
                void update();
                void getSubsUUID();
                void save();
            };
            std::unordered_map<std::string,user*> userUUIDs;
            std::map<RakNet::SystemAddress,std::string> userAddrs;
            std::atomic<int> onLineNum;
            virtual void setOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual void removeFromOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual bool userCanPutSubs(const std::string & useruuid);
            virtual void moveUserPosition(const std::string & subsuuid,const std::string & muuid,const irr::core::vector3df & posi);
            virtual bool userOnline(const std::string & uuid);
            sNameLocker uLocker;
        public:
            bool login(const std::string & uuid , const RakNet::SystemAddress & address,const std::string & pwd);
            inline bool logout(const RakNet::SystemAddress & address){
                std::string uuid;
                if(getUUIDByAddr(uuid,address)){
                    logout(uuid);
                    return true;
                }
                return false;
            }
            void logout(const std::string & uuid);
            void changePwd(const std::string & uuid ,const std::string & opwd ,const std::string & npwd);
            void getChunkSubs(const std::string & uuid , const RakNet::SystemAddress & address,int x,int y);
            
            void createUser(
                std::string & uuid ,
                const std::string & pwd , 
                irr::core::vector3df position ,
                long id
            );
            
            bool getUUIDByAddr(std::string & uuid,const RakNet::SystemAddress & address);
            
            inline users(){
                pthread_rwlock_init(&rwlock, NULL);
            }
            inline ~users(){
                pthread_rwlock_destroy(&rwlock);
            }
            inline void usersInit(const char * upath){
                leveldb::Options opt;
                opt.create_if_missing=true;
                leveldb::DB::Open(opt,upath,&this->db);
                onLineNum=0;
            }
            inline void createSubs(//添加物体
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & config,
                const RakNet::SystemAddress & from
            ){
                std::string muuid;
                if(getUUIDByAddr(muuid,from)){
                    subsServer::createSubs(id,p,r,impulse,rel_pos,muuid,config,from);
                }
            }
            inline void setSubs(
                const std::string & uuid,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& lin_vel ,
                const btVector3& ang_vel ,
                int status,
                const RakNet::SystemAddress & from
            ){
                std::string muuid;
                if(getUUIDByAddr(muuid,from)){
                    subsServer::setSubs(uuid,p,r,lin_vel,ang_vel,status,muuid);
                }
            }
            inline void giveUpControl(const std::string & uuid,const RakNet::SystemAddress & from){
                std::string muuid;
                if(getUUIDByAddr(muuid,from)){
                    subsServer::giveUpControl(uuid,muuid);
                }
            }
            inline void userDestroy(){
                delete this->db;
            }
        private:
            bool checkPasswd(const std::string & uuid,const std::string & pwd);
            user * seekOnLine(const std::string & uuid);
            int getUserNumSubs(const std::string & uuid);
            void setUserNumSubs(const std::string & uuid,int num);
            
            void setPwd(const std::string & uuid,const std::string & pwd);
            bool getPwd(const std::string & uuid,std::string & pwd);
            
            void sendSubsList(const RakNet::SystemAddress & address,const std::string & uuid);
            
        public:
            virtual void sendSetUserSubs(const RakNet::SystemAddress & address,const std::string & uuid)=0;
            virtual int getUserMaxSubs()=0;
            virtual void sendUserSubsUUID(const std::string & uuid,const RakNet::SystemAddress & address)=0;
            virtual void sendUserSubsUUIDs(const std::list<std::string> & uuid,const RakNet::SystemAddress & address)=0;
            virtual void setUserPosition(const irr::core::vector3df & position,const RakNet::SystemAddress & addr)=0;
            virtual void delListener(const RakNet::SystemAddress & addr)=0;
    };
}
#endif