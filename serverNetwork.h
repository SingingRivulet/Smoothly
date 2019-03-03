#ifndef SMOOTHLY_SERVER_NETWORK
#define SMOOTHLY_SERVER_NETWORK
#include "utils.h"
#include "watch.h"
namespace smoothly{
    class serverNetwork:public watch{
        public:
            virtual void removeTableApplay(const RakNet::SystemAddress & to,int x,int y);
            
            virtual void sendRemoveTable(
                const RakNet::SystemAddress & to,
                const std::list<std::pair<long,int> > & l,
                int x,int y
            );
            virtual void onSendNode(
                const std::string & uuid,
                const irr::core::vector3df & position,
                const irr::core::vector3df & rotation,
                const std::set<std::string> & link,
                const std::set<std::string> & linkTo,
                long type,
                int hp,
                int x,int y,
                const RakNet::SystemAddress & to
            );
            
            inline void sendMessage(RakNet::BitStream * data,const RakNet::SystemAddress & address){
                if(connection)
                    connection->Send( data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false );
            }
            inline void sendMessageU(RakNet::BitStream * data,const RakNet::SystemAddress & address){
                if(connection)
                    connection->Send( data, HIGH_PRIORITY, RELIABLE, 0, address, false );
            }
            
            inline void kick(const RakNet::SystemAddress & address){
                if(connection)
                    connection->CloseConnection(address, true, 0);
                delListener(address);
            }
            
            bool loged(const RakNet::SystemAddress & address);
            
            void recv();
            void onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateBuilding(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateTerrain(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateUser(RakNet::Packet * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateUserPosition(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateUserRotation(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateBuildingDownload(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateBuildingAttack(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateBuildingCreate(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateTerrainGetRMT(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateTerrainRemove(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            serverNetwork(const char * pathGra,const char * pathRMT,const char * modpath,short port,int maxcl);
            serverNetwork(const serverNetwork &)=delete;
            ~serverNetwork();
        private:
            RakNet::RakPeerInterface * connection;
    };
}
#endif