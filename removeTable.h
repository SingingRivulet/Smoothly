#ifndef SMOOTHLY_SERVER_REMOVETABLE
#define SMOOTHLY_SERVER_REMOVETABLE
#include "graphServer.h"
#include <set>
#include <list>
namespace smoothly{
    class removeTable{
        public:
            leveldb::DB * db;
            
            void init(const char * path);
            void destroy();
            
            inline void str2mapid(const char * s,long * iid,int * mid){
                sscanf(s,"%ld,%d",iid,mid);
            }
            inline void mapid2str(long iid,int mid,char * s,int len){
                snprintf(s,len,"%ld,%d",iid,mid);
            }
            
            void resetTerrain(int x,int y);
            
            void removeTerrain(int x,int y,long itemid,int mapid);
            virtual int itemidNum(long id)=0;
            
            bool mapidExist(int x,int y,long itemid,int mapid);
            void setMapidElement(int x,int y,long itemid,int mapid);
            void removeMapidElement(int x,int y,long itemid,int mapid);
            
            void getRemoveTable(const RakNet::SystemAddress & to,int x,int y);
            virtual void sendRemoveTable(
                const RakNet::SystemAddress & to,
                const std::list<std::pair<long,int> > & l,
                int x,int y
            )=0;
            virtual void removeTableApplay(const RakNet::SystemAddress & to,int x,int y)=0;
            virtual void onRemoveTerrain(int x,int y,long itemid,int mapid)=0;
    };
}
#endif