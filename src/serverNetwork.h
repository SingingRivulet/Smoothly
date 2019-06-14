#ifndef SMOOTHLY_SERVER_NETWORK
#define SMOOTHLY_SERVER_NETWORK
#include "utils.h"
#include "watch.h"
namespace smoothly{
    class serverNetwork:public watch{
        public:
            virtual void removeTableApply(const RakNet::SystemAddress & to,int x,int y);
            
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
            virtual void sendSubsStatus(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r ,
                const irr::core::vector3df & direction , 
                const btVector3& lin_vel,
                const btVector3& ang_vel,
                int status,
                int hp,
                const std::string & useruuid,
                const std::string & config,
                const RakNet::SystemAddress & to
            );
            virtual void sendPutSubsFail(const RakNet::SystemAddress & to);
            virtual void sendSetUserSubs(const RakNet::SystemAddress & address,const std::string & uuid);
            virtual void sendUserSubsUUID(const std::string & uuid,const RakNet::SystemAddress & address);
            virtual void sendUserSubsUUIDs(const std::list<std::string> & uuid,const RakNet::SystemAddress & address);
            virtual void sendChunkRun(int x,int y,const RakNet::SystemAddress & to);
            virtual void sendAttaching(const std::string & subs,const bodyAttaching & att,const RakNet::SystemAddress & to);
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
                logout(address);
            }
            
            bool loged(const RakNet::SystemAddress & address);
            
        public:
            void recv();
            //events
            void onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateBuilding(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateTerrain(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateUser(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubs(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageUpdateAttaching(RakNet::Packet * data,const RakNet::SystemAddress & address);
            void onMessageAdmin(RakNet::Packet * data,const RakNet::SystemAddress & address);
            
            //user
            void onMessageUpdateUserLogin(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateUserChangePwd(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateUserLogout(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            
            //building
            void onMessageUpdateBuildingDownload(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateBuildingAttack(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateBuildingCreate(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            
            //terrain
            void onMessageUpdateTerrainGetRMT(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateTerrainRemove(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            
            //substances
            void onMessageUpdateSubsCreate(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsRemove(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsAttack(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsRequestChunk(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsRequestUUID(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsSetSubs(RakNet::BitStream * data,const RakNet::SystemAddress & address);//所有用户不断调用，至于采纳谁的，由服务器决定
            void onMessageUpdateSubsTeleport(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageUpdateSubsGiveUp(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            void onMessageUpdateAttachingUpload(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            //admin
            void onMessageAdminCreateUser(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageAdminTeleport(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageAdminSetOwner(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            void onMessageAdminSetSubsStr(RakNet::BitStream * data,const RakNet::SystemAddress & address);
            
            serverNetwork();
            serverNetwork(const serverNetwork &)=delete;
            ~serverNetwork();
            
            void start(const char * path,short port,int maxcl);
            void release();
        private:
            RakNet::RakPeerInterface * connection;
    };
}
#endif