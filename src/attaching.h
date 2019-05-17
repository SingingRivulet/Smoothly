#ifndef SMOOTHLY_SERVER_ATTACHING
#define SMOOTHLY_SERVER_ATTACHING
#include "subsServer.h"
namespace smoothly{
    class attaching:public subsServer{//substance的附带品
        protected:
            leveldb::DB * db;
        public:
            void setAttaching(
                const std::string & user , 
                const std::string & subs , 
                bodyAttaching & att//不声明const是因为可以被回调函数修改
            );
            
            virtual void getAttaching(const std::string & subs,const RakNet::SystemAddress & to);
            
            virtual void delAttaching(const std::string & subs);
            
            void getAttachingFromDB(const std::string & subs,bodyAttaching & att);
            void setAttachingToDB  (const std::string & subs,const bodyAttaching & att);
            
            bool setAttachingCallback(//返回false阻止写入
                const std::string & user , 
                subs * , 
                bodyAttaching & att//不声明const是因为可以被回调函数修改
            );
            
            virtual void boardcastAttaching(
                const std::string & subs,
                const bodyAttaching & att,
                const irr::core::vector3df & p
            )=0;
            virtual void sendAttaching(
                const std::string & subs,
                const bodyAttaching & att,
                const RakNet::SystemAddress & to
            )=0;
            
            void attachingInit(const char * path);
            void attachingDestroy();
    };
}
#endif