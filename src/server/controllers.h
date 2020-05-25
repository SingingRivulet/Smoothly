#ifndef SMOOTHLY_SERVER_CTLS
#define SMOOTHLY_SERVER_CTLS

#include "handlers.h"

namespace smoothly{
namespace server{
/////////////////
class controllers:public handlers{
    public:
        controllers(int thnum);

        virtual void onMessage(const std::string & uuid,const RakNet::SystemAddress &,char c,char a,RakNet::BitStream * data);
        
        //controllers
        void ctl_addRemovedItem(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        
        void ctl_wearing_add(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_wearing_remove(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_wearing_get(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_setLookAt(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_setStatus(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_setPosition(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_setRotation(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_interactive(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_HPInc(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_getBody(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);

        void ctl_fire(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);

        void ctl_addBuilding(const std::string & uuid, const RakNet::SystemAddress &addr, RakNet::BitStream * data);
        void ctl_damageBuilding(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);
        void ctl_getBuilding(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data);

        void ctl_getTool(const std::string & uuid, const RakNet::SystemAddress &addr, RakNet::BitStream * data);
        void ctl_useTool(const std::string & uuid, const RakNet::SystemAddress &addr, RakNet::BitStream * data);
        void ctl_reloadTool(const std::string & uuid, const RakNet::SystemAddress &addr, RakNet::BitStream * data);
        
};
/////////////////
}//////server
}//////smoothly
#endif
