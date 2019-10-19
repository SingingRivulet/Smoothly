#ifndef SMOOTHLY_SERVER_HANDLERS
#define SMOOTHLY_SERVER_HANDLERS
#include "connection.h"
namespace smoothly{
namespace server{
/////////////////
#define makeHeader(c,a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)MESSAGE_GAME); \
    bs.Write((RakNet::MessageID)'.'); \
    bs.Write((RakNet::MessageID)c); \
    bs.Write((RakNet::MessageID)a);

class handlers:public connection{
    public:
        virtual void boardcast_wearing_add(const std::string & uuid,int x,int y,int d);
        virtual void boardcast_wearing_remove(const std::string & uuid,int x,int y,int d);
        virtual void boardcast_hp(const std::string & uuid,int x,int y,int hp);
        virtual void boardcast_setStatus(const std::string & uuid,int x,int y,int s);
        virtual void boardcast_setLookAt(const std::string & uuid,int x,int y,const vec3 & v);
        virtual void boardcast_setPosition(const std::string & uuid,int x,int y,const vec3 & v);
        virtual void boardcast_setRotation(const std::string & uuid,int x,int y,const vec3 & v);
        virtual void boardcast_interactive(const std::string & uuid,int x,int y,const std::string & s);
        
        virtual void boardcast_bodyRemove(const std::string & uuid,int x,int y);
        
        virtual void boardcast_createBody(const std::string & uuid,int x,int y,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l);
        
        virtual void sendAddr_body(const RakNet::SystemAddress & addr,
            const std::string & uuid,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l,
            const std::set<int> & wearing);
        
        virtual void sendRemoveTable(const ipair & p , const std::string & to);
        virtual void sendAddr_removeTable(const RakNet::SystemAddress & addr,
            int x,int y,
            const std::list<std::pair<int,int> > & rmt);
        
        virtual void boardcast_addRemovedItem(int x,int y,int id,int index);
        
        void boardcast(int x,int y,RakNet::BitStream * data);
        
};
/////////////////
}//////server
}//////smoothly
#endif