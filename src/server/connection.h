#ifndef SMOOTHLY_SERVER_CONNECTION
#define SMOOTHLY_SERVER_CONNECTION
#include "body.h"
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <map>
#include <unordered_map>
namespace smoothly{
namespace server{

class connection:public body{
    public:
        void start(unsigned short port,int maxcl);
        void release();
        
        void login(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & pwd);
        void logout(const RakNet::SystemAddress & addr);
        void logout(const std::string & uuid);
        void setPwd(const std::string & uuid,const std::string & pwd);
        
        void getAddrByUUID(const std::string & uuid,RakNet::SystemAddress & addr);//没找到的话，直接throw掉
        void getUUIDByAddr(const RakNet::SystemAddress & addr,std::string & uuid);//没找到的话，直接throw掉
        
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
        
    private:
        RakNet::RakPeerInterface * con;
        std::map<RakNet::SystemAddress,std::string> addrs;
        std::unordered_map<std::string,RakNet::SystemAddress> uuids;
        void linkUUID(const std::string & uuid,const RakNet::SystemAddress & addr);
        void onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address);
};

}//////server
}//////smoothly
#endif
