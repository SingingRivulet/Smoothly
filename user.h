#ifndef SMOOTHLY_USER
#define SMOOTHLY_USER
#include "subsServer.h"
namespace smoothly{
    class users:public subsServer{
        protected:
            leveldb::DB * db;
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
                void createRandomPlayer(long id);
            };
            virtual bool getUUIDByAddr(std::string & uuid , const RakNet::SystemAddress & addr);
            virtual bool getAddrByUUID(RakNet::SystemAddress & addr , const std::string & uuid);
            virtual void setOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual void removeFromOwner(const std::string & subsuuid,const std::string & useruuid);
            virtual bool userCanPutSubs(const std::string & useruuid);
            virtual void moveUserPosition(const std::string & uuid,const std::string & muuid,const irr::core::vector3df & posi);
            std::mutex uSubsCountLocker;
        public:
            void login(const std::string & uuid , const RakNet::SystemAddress address,const std::string & pwd);
            void logout(const RakNet::SystemAddress & addr);
            void createUser(const std::string & uuid , const std::string & pwd);
            void changePwd(const std::string & uuid , const std::string & pwd);
            void getChunkSubs(const std::string & uuid , const RakNet::SystemAddress address,int x,int y);
            
            inline users(const char * spath,const char * upath):subsServer(spath){
                leveldb::Options opt;
                opt.create_if_missing=true;
                leveldb::DB::Open(opt,upath,&this->db);
            }
            inline ~users(){
                delete this->db;
            }
    };
}
#endif