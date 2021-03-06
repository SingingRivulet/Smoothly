#ifndef SMOOTHLY_SERVER_RMT
#define SMOOTHLY_SERVER_RMT
#include "dig.h"
namespace smoothly{
namespace server{

class removeTable:public dig{
    public:
        removeTable();
        void getRemovedItem(int x,int y,std::list<std::pair<int,int> > &);//id , index
        bool addRemovedItem(int cx, int cy, float x, float y, int id, int index);
        void clearRemovedChunk(int x,int y);

        virtual void destroyTerrainItem(float x,float y,int id)=0;

    private:
        std::pair<int,int> getIdFromStr(const char * );
};

}//////server
}//////smoothly
#endif
