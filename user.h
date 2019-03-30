#ifndef SMOOTHLY_USER
#define SMOOTHLY_USER
#include "subsServer.h"
#include "nameLocker.h"
#include <map>
namespace smoothly{
    class users:public subsServer{
        protected:
            leveldb::DB * db;
            pthread_rwlock_t rwlock;
            struct user{
                std::string subsUUID;
                std::string userUUID;
                RakNet::SystemAddress address;
                irr::core::vector3df position;
                users * parent;
                int subsNum;
                void update();
                void getSubsUUID();
                void subsNumInc();
                void subsNumDec();
                void save();
                void createRandomPlayer(long id);
            };
            std::unordered_map<std::string,user*> userUUIDs;
            std::map<RakNet::SystemAddress,std::string> userAddrs;
            
            virtual void setOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual void removeFromOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual bool userCanPutSubs(const std::string & useruuid);
            virtual void moveUserPosition(const std::string & subsuuid,const std::string & muuid,const irr::core::vector3df & posi);
            sNameLocker uLocker;
        public:
            void login(const std::string & uuid , const RakNet::SystemAddress & address,const std::string & pwd);
            void logout(const std::string & uuid);
            void createUser(const std::string & uuid , const std::string & pwd);
            void changePwd(const std::string & uuid , const std::string & pwd);
            void getChunkSubs(const std::string & uuid , const RakNet::SystemAddress address,int x,int y);
            
            inline users(const char * spath,const char * upath):subsServer(spath){
                pthread_rwlock_init(&rwlock, NULL);
                leveldb::Options opt;
                opt.create_if_missing=true;
                leveldb::DB::Open(opt,upath,&this->db);
            }
            inline ~users(){
                delete this->db;
                pthread_rwlock_destroy(&rwlock);
            }
        private:
            bool checkPasswd(const std::string & uuid,const std::string & pwd);
            user * seekOnLine(const std::string & uuid);
            int getUserNumSubs(const std::string & uuid);
            void setUserNumSubs(const std::string & uuid,int num);
            
        public:
            virtual int getUserMaxSubs()=0;
    };
}
#endif