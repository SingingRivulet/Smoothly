#ifndef SMOOTHLY_SERVER_WATCH
#define SMOOTHLY_SERVER_WATCH
#include "graphServer.h"
#include "removeTable.h"
#include "hbb.h"
#include "serverMod.h"
namespace smoothly{
    class watch:public serverMod{
        public:
            HBB eventHBB;
            struct listener{
                HBB::AABB * box;
                listener * next;
                RakNet::SystemAddress address;
                watch * parent;
            };
            std::map<RakNet::SystemAddress,listener*> listeners;
            void * listenerPool;
            
            listener * createListener();
            inline bool addrExist(const RakNet::SystemAddress & addr){
                return listeners.find(addr)!=listeners.end();
            }
            inline void setUserPosition(const irr::core::vector3df & position,const RakNet::SystemAddress & addr){
                HBB::vec3 from  (position.X-viewRange , position.Y-viewRange , position.Z-viewRange);
                HBB::vec3 to    (position.X+viewRange , position.Y+viewRange , position.Z+viewRange);
                setListener(from,to,addr);
            }
            void setListener(
                const HBB::vec3 & from,
                const HBB::vec3 & to,
                const RakNet::SystemAddress & addr
            );
            void delListener(listener *);
            void lstPoolInit();
            void lstPoolFree();
            void delListener(const RakNet::SystemAddress & addr);
            void boardcastByPoint(const HBB::vec3 & point,RakNet::BitStream * bs);
            
            virtual void sendMessage(RakNet::BitStream * data,const RakNet::SystemAddress & address)=0;
            virtual void onCreateNode(
                const std::string & uuid,
                const irr::core::vector3df & position,
                const irr::core::vector3df & rotation,
                const std::set<std::string> & link,
                long type,
                int hp,
                int x,int y
            );
            virtual void onDestroyNode(const std::string & uuid,int x,int y);
            virtual void onAttackNode(const std::string & uuid,int hp,int x,int y);
            virtual void onRemoveTerrain(int x,int y,long itemid,int mapid);
    };
}
#endif