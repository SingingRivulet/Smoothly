#ifndef SMOOTHLY_CLIENT_NETWORK
#define SMOOTHLY_CLIENT_NETWORK
#include "substance.h"
namespace smoothly{
    class clientNetwork:public substance{
        public:
            void init(const char * addr,short port);
            void shutdown();
            void recv();
            void connect();
            
            void onRecvMessage(RakNet::Packet * data);
            void sendMessage(RakNet::BitStream * data);
            void sendMessageU(RakNet::BitStream * data);
            
            void onMessageUpdateBuilding(RakNet::Packet * data);
            void onMessageUpdateSubs(RakNet::Packet * data);
            void onMessageUpdateTerrain(RakNet::Packet * data);
            
            //building
            void onMessageUpdateBuildingGen(RakNet::BitStream * data);
            void onMessageUpdateBuildingAttack(RakNet::BitStream * data);
            void onMessageUpdateBuildingCreate(RakNet::BitStream * data);
            void onMessageUpdateBuildingDestroy(RakNet::BitStream * data);
            
            //terrain
            void onMessageUpdateTerrainGetRMTable(RakNet::BitStream * data);
            void onMessageUpdateTerrainRMTApply(RakNet::BitStream * data);
            void onMessageUpdateTerrainRemove(RakNet::BitStream * data);
            
            //substance
            void onMessageUpdateSubsTeleport(RakNet::BitStream * data);
            void onMessageUpdateSubsSetStatus(RakNet::BitStream * data);
            void onMessageUpdateSubsCreate(RakNet::BitStream * data);
            void onMessageUpdateSubsCreateBrief(RakNet::BitStream * data);
            void onMessageUpdateSubsAttack(RakNet::BitStream * data);
            void onMessageUpdateSubsRemove(RakNet::BitStream * data);
            void onMessageUpdateSubsFail(RakNet::BitStream * data);
            void onMessageUpdateSubsSetUserSubs(RakNet::BitStream * data);
            void onMessageUpdateSubsAddOneSubs(RakNet::BitStream * data);
            void onMessageUpdateSubsAddSubs(RakNet::BitStream * data);
            void onMessageUpdateSubsRunChunk(RakNet::BitStream * data);
            
            virtual void uploadAttack(const std::string & uuid , int hurt);
            virtual void downloadBuilding(int x,int y);
            virtual remoteGraph::item * downloadBuilding(const std::string & uuid);
            virtual void addNode(//添加节点
                const irr::core::vector3df & position,//位置
                const irr::core::vector3df & rotation,
                const std::set<std::string> & link,//表示修建在什么节点上
                int hp,
                long type
            );
            
            
            virtual void requestRemoveItem(const mapid & mid);
            virtual void requestUpdateTerrain(int x,int y);
            virtual void buildOnFloor(
                long type,
                const irr::core::vector3df &,
                const irr::core::vector3df &
            );
            virtual void buildOn(
                long type,
                const irr::core::vector3df &,
                const irr::core::vector3df &,
                const std::list<std::string> &
            );
            void setUserPosition(const irr::core::vector3df & p);
            void setUserRotation(const irr::core::vector3df & r);
            void move(const irr::core::vector3df & delta);
            
            void login(const std::string & name,const std::string & pwd);
            
            bool canConnect;
        private:
            RakNet::RakPeerInterface * connection;
            std::string addr;
            short port;
    };
}
#endif