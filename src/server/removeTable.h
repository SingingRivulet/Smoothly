#ifndef SMOOTHLY_SERVER_RMT
#define SMOOTHLY_SERVER_RMT
#include "map.h"
namespace smoothly{
namespace server{

class removeTable:public map{
    public:
        void getRemovedItem(int x,int y,std::list<std::pair<int,int> > &);//id , index
        void addRemovedItem(int x,int y,int id,int index);
        void clearRemovedChunk(int x,int y);
    private:
        std::pair<int,int> getIdFromStr(const char * );
};

}//////server
}//////smoothly
#endif