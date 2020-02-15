#ifndef SMOOTHLY_SERVER_CONNECTION
#define SMOOTHLY_SERVER_CONNECTION

#include "bullet.h"
#include "heartbeat.h"
#include "../utils/dbvt2d.h"

#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <map>
#include <unordered_map>
#include <unordered_set>
namespace smoothly{
namespace server{

class connection:public bullet{
    public:
        virtual void start(unsigned short port,int maxcl,int vf);
        void release()override;
        
        void login(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & pwd);
        void logout(const RakNet::SystemAddress & addr);
        void logout(const std::string & uuid);
        void setPwd(const std::string & uuid,const std::string & pwd);
        
        void getAddrByUUID(const std::string & uuid,RakNet::SystemAddress & addr);//没找到的话，直接throw掉
        void getUUIDByAddr(const RakNet::SystemAddress & addr,std::string & uuid);//没找到的话，直接throw掉
        
        inline void setTimeout(int t){
            hb.timeout=t;
        }

        inline void sendMessage(RakNet::BitStream * data,const RakNet::SystemAddress & address){
            if(con)
                con->Send( data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false );
        }
        inline void sendMessageU(RakNet::BitStream * data,const RakNet::SystemAddress & address){
            if(con)
                con->Send( data, HIGH_PRIORITY, RELIABLE, 0, address, false );
        }
            
        inline void kick(const RakNet::SystemAddress & address){
            if(con)
                con->CloseConnection(address, true, 0);
            logout(address);
        }
        
        void recv();
        
        enum GameMessage{
            MESSAGE_GAME=ID_USER_PACKET_ENUM+1
        };

        virtual void adminMessage(RakNet::BitStream * data,const RakNet::SystemAddress &)=0;
        
        virtual void onMessage(const std::string & uuid,const RakNet::SystemAddress &,char c,char a,RakNet::BitStream * data)=0;
        
        virtual void sendAddr_mainControl(const RakNet::SystemAddress & addr,const std::string & uuid)=0;

        virtual void sendAddr_body(const RakNet::SystemAddress & addr,
            const std::string & uuid,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l,
            const std::set<int> & wearing)=0;
        
        virtual void sendAddr_removeTable(const RakNet::SystemAddress & addr,
            int x,int y,
            const std::list<std::pair<int,int> > & rmt)=0;
        
        virtual void send_body(const std::string & to,
            const std::string & uuid,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l,
            const std::set<int> & wearing);
        
        void sendBodyToAddr(const RakNet::SystemAddress & addr,const std::string & uuid);
        
        connection();

    private:
        void linkUUID(const std::string & uuid,const RakNet::SystemAddress & addr);
        void onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address);

        struct userSet;
        struct charBB{
            std::string owner;
            userSet * userp;
            std::string uuid;
            RakNet::SystemAddress address;
            dbvt2d::AABB * box;
            int x,y;
            long long timeStep;
        };
        struct userSet{
            std::set<charBB*> owned;
            long long timeStep;
        };
        std::unordered_map<std::string,charBB*>  charBBs;
        std::unordered_map<std::string,userSet*> charOwners;
        dbvt2d viewDBVT;
        long long dbvtTimeStep;
        void updateChunkDBVT(const std::string & uuid,const std::string & owner,int x, int y)override;
        void removeFromDBVT(const std::string & uuid)override;
        void addToDBVT(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & owner,int x,int y);
        charBB * updateDBVT(const std::string & uuid,const std::string & owner,int x, int y,bool create = true);
        void removeDBVT(const std::string & uuid);
        void removeUserBox(const std::string & owner);
        userSet *seekUserSet(const std::string & owner);
    public:
        void fetchByDBVT(int x,int y,std::function<void (charBB*)> callback);
        void fetchUserByDBVT(int x,int y,std::function<void (const std::string & owner,const RakNet::SystemAddress &addr)> callback);
    private:
        RakNet::RakPeerInterface * con;
        std::map<RakNet::SystemAddress,std::string> addrs;
        std::unordered_map<std::string,RakNet::SystemAddress> uuids;
        heartbeat<RakNet::SystemAddress> hb;
        int lastAutoKickTime;
};

}//////server
}//////smoothly
#endif
