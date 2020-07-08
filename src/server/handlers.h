#ifndef SMOOTHLY_SERVER_HANDLERS
#define SMOOTHLY_SERVER_HANDLERS
#include "admin.h"
namespace smoothly{
namespace server{
/////////////////
#define makeHeader(c,a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)MESSAGE_GAME); \
    bs.Write((RakNet::MessageID)'.'); \
    bs.Write((RakNet::MessageID)c); \
    bs.Write((RakNet::MessageID)a);

class handlers:public admin{
    public:
        handlers(int thnum);
        void boardcast_wearing_add(const std::string & uuid,int x,int y,int d)override;
        void boardcast_wearing_remove(const std::string & uuid,int x,int y,int d)override;
        void boardcast_hp(const std::string & uuid,int x,int y,int hp)override;
        void boardcast_setStatus(const std::string & uuid,int x,int y,int s)override;
        void boardcast_setLookAt(const std::string & uuid,int x,int y,const vec3 & v)override;
        void boardcast_setPosition(const std::string & uuid,int x,int y,const vec3 & v)override;
        void boardcast_setRotation(const std::string & uuid,int x,int y,const vec3 & v)override;
        void boardcast_interactive(const std::string & uuid,int x,int y,const std::string & s)override;
        
        void sendAddr_mainControl(const RakNet::SystemAddress & addr,const std::string & uuid)override;

        void boardcast_bodyRemove(const std::string & uuid,int x,int y)override;
        
        void boardcast_createBody(const std::string & uuid,int x,int y,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l)override;
        
        void sendAddr_body(const RakNet::SystemAddress & addr,
            const std::string & uuid,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l,
            const std::set<int> & wearing)override;
        
        void sendRemoveTable(const ipair & p , const std::string & to)override;
        void sendAddr_removeTable(const RakNet::SystemAddress & addr,
            int x,int y,
            const std::list<std::pair<int,int> > & rmt)override;
        void sendAddr_chunkACL(const RakNet::SystemAddress & addr, const ipair & posi, chunkACL_t &)override;
        void boardcast_chunkACL(const ipair & posi, map::chunkACL_t & acl);

        void sendAddr_bag(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & text)override;
        void sendAddr_bag_resourceNum(const RakNet::SystemAddress & addr,const std::string & uuid,int id,int num)override;
        void sendAddr_bag_toolDur(const RakNet::SystemAddress & addr,const std::string & uuid,int dur,int pwr)override;
        void sendAddr_bag_tool_add(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID);
        void sendAddr_bag_tool_remove(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID);
        void sendAddr_bag_tool_use(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID);
        
        void boardcast_addRemovedItem(int x,int y,int id,int index);
        
        void boardcast_shoot(const std::string & user,int id,const vec3 & from,const vec3 & dir)override;

        void boardcast_buildingRemove(const std::string & uuid , int x,int y)override;
        void boardcast_buildingAdd(const std::string & uuid ,int id,const vec3 & p , const vec3 & r, int x,int y);
        void sendBuildingChunk(int32_t x, int32_t y,const RakNet::SystemAddress & addr);

        void boardcast_packageRemove(int x,int y,const std::string & uuid);
        void boardcast_packageAdd(int x,int y,const std::string & uuid,const std::string & text);
        void sendAddr_packageAdd(const RakNet::SystemAddress & addr,int x,int y,const std::string & uuid,const std::string & text);

        void sendAddr_unlockTech(const RakNet::SystemAddress & addr,bool newTech,int id);
        void sendAddr_techTarget(const RakNet::SystemAddress & addr,bool newTarget,int id);

        void sendAddr_missionList(const RakNet::SystemAddress & addr,const std::vector<std::string> & );
        void sendAddr_missionText(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & text);
        void boardcast_mission(const vec3 & posi, const std::string & muuid);

        void sendUser_newMail(const std::string & user);
        void sendAddr_mail(const RakNet::SystemAddress & addr , const std::string & text);
        void sendAddr_mailPackagePickedUp(const RakNet::SystemAddress & addr);

        void boardcast(int x,int y,RakNet::BitStream * data);
        
};
/////////////////
}//////server
}//////smoothly
#endif
