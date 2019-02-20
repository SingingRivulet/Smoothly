#ifndef SMOOTHLY_SERVER_GRAPH
#define SMOOTHLY_SERVER_GRAPH
#include <leveldb/db.h>
#include <string>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include "utils.h"
#include "uuid.h"
namespace smoothly{
    class graphServer{
        public:
            leveldb::DB * db;
            int maxdeep;
            std::map<ipair,std::set<std::string> > removeList;
            static inline void getChunkPosition(double ix,double iy,int & ox,int & oy){
                ox=ix/32;
                oy=iy/32;
            }
            
            void removeApplay();
            bool nodeExist(const std::string & uuid);
            
            void attack(const std::string & uuid,int hurt);
            int hurt(const std::string & uuid,int hurt);
            void remove(const std::string & uuid,int deep);
            void remove(const std::string & uuid);
            
            int removeLinkTo(const std::string & uuid,const std::string & rm);
            int removeLink(const std::string & uuid,const std::string & rm);
            void removeAllLink(const std::string & uuid);
            void removeAllLinkTo(const std::string & uuid,int deep);
            int removeSet(const std::string & pre,const std::string & uuid,const std::string & rm);
            
            void addLink(const std::string & uuid,const std::string & a);
            void addLinkTo(const std::string & uuid,const std::string & a);
            void addSet(const std::string & pre,const std::string & uuid,const std::string & a);
            
            void getChunk(const std::string & uuid,int & x,int & y);
            void removeFromChunk(const std::string & uuid,int x,int y);
            void addIntoChunk(const std::string & uuid,int x,int y);
            
            void createNode(
                long type,
                const irr::core::vector3df & position,
                const irr::core::vector3df & rotation,
                const std::set<std::string> & link
            );
            //void destroyNode(const std::string & uuid);
            //请使用attack
            
            virtual void onCreateNode(
                const std::string & uuid,
                const irr::core::vector3df & position,
                const irr::core::vector3df & rotation,
                const std::set<std::string> & link,
                int x,int y
            )=0;
            virtual void onDestroyNode(const std::string & uuid,int x,int y)=0;
            virtual int getHPByType(long hp)=0;
    };
}
#endif