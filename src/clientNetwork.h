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
            void onMessageUpdateAttaching(RakNet::Packet * data);
            void onMessageUpdateAttack(RakNet::Packet * data);
            
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
            
            void onMessageUpdateAttachingSet(RakNet::BitStream * data);
            
            //attack
            void onMessageUpdateAttackDo(RakNet::BitStream * data);
            
            virtual void uploadAttackAm(
                const std::string & uuid,
                const irr::core::vector3df & dir,
                uint32_t bid,
                int id
            );
            
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
            
            virtual void uploadBodyStatus(//上传持久型物体状态
                const std::string & uuid , 
                int status,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r ,
                const irr::core::vector3df & direction , 
                const btVector3& lin_vel ,
                const btVector3& ang_vel
            );
            
            void requestCreateSubs(//请求创建物体
                long id,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & d, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & config = std::string("")
            );
            virtual void requestCreateBriefSubs(//请求创建物体（非持久）
                long id,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & d, 
                const btVector3& impulse,
                const btVector3& rel_pos
            );
            virtual void requestCreateLastingSubs(//请求创建物体（持久）
                long id,
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const irr::core::vector3df & d, 
                const btVector3& impulse,
                const btVector3& rel_pos
            );
            virtual void requestDownloadSubstanceChunk(int x,int y);
            void requestDownloadSubstanceUUID(const std::string & uuid);
            
            virtual void requestRemoveSubs(const std::string & uuid);//请求删除持久物体（非持久不需要删除）
            virtual void requestTeleport(const std::string & uuid,const irr::core::vector3df & p);
            virtual void requestAttackSubs(const std::string & uuid,int dmg);
            
            void setUserPosition(const irr::core::vector3df & p);
            void setUserRotation(const irr::core::vector3df & r);
            
            void login(const std::string & name,const std::string & pwd);
            
            bool canConnect;
        private:
            RakNet::RakPeerInterface * connection;
            std::string addr;
            short port;
    };
}
#endif