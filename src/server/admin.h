#ifndef SMOOTHLY_SERVER_ADMIN
#define SMOOTHLY_SERVER_ADMIN
#include "connection.h"
#include <unordered_set>
#include <mutex>
namespace smoothly{
namespace server{

class admin:public connection{
    public:
        admin(int thnum);
        virtual void adminMessage(RakNet::BitStream * data,const RakNet::SystemAddress &);
    private:
        std::unordered_set<std::string> keys;//调用admin的key
        std::mutex keyLocker;
        bool checkKey(const std::string & k);

        void admin_handler(const char * , RakNet::BitStream * data,const RakNet::SystemAddress &);
        void adminMap_init();
};

}//////server
}//////smoothly
#endif // ADMIN_H
