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
            virtual void setUserPosition(const irr::core::vector3df & position,const RakNet::SystemAddress & addr);
            void setListener(
                const HBB::vec3 & from,
                const HBB::vec3 & to,
                const RakNet::SystemAddress & addr
            );
            void delListener(listener *);
            void lstPoolInit();
            void lstPoolFree();
            virtual void delListener(const RakNet::SystemAddress & addr);
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
            virtual void boardcastSubsCreate(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & useruuid
            );
            virtual void boardcastSubsCreate(
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& impulse,
                const btVector3& rel_pos,
                const std::string & useruuid,
                const RakNet::SystemAddress & ext
            );
            virtual void boardcastSubsStatus(
                const std::string & subsuuid,
                long id , 
                const irr::core::vector3df & p,
                const irr::core::vector3df & r, 
                const btVector3& lin_vel,
                const btVector3& ang_vel,
                int status,
                int hp,
                const std::string & useruuid
            );
            virtual void boardcastSubsRemove(const std::string & subsuuid,const irr::core::vector3df & p);
            virtual void boardcastSubsAttack(const std::string & subsuuid,const irr::core::vector3df & p,int hp,int delta);
            virtual void boardcastTeleport(
                const std::string & subsuuid,
                const irr::core::vector3df & p
            );
    };
}
#endif