#ifndef SMOOTHLY_SERVER_DIG
#define SMOOTHLY_SERVER_DIG
#include "map.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <tuple>
#include <sys/stat.h>
#include <raknet/BitStream.h>

namespace smoothly{
namespace server{

class dig:public map{
    public:
        dig();
        void setDigDepth(const std::vector<std::pair<std::pair<int32_t,int32_t>,int16_t> > & dig);
        void sendDigMap(const RakNet::SystemAddress & addr, int32_t x, int32_t y);
        virtual void boardcast_setDigDepth(int x,int y,const std::vector<std::pair<int16_t,int16_t> > & dig)=0;
        virtual void sendAddr_digMap(const RakNet::SystemAddress & addr, int32_t x, int32_t y, const char * data , int len)=0;

    private:
        std::map<ipair,std::tuple<int,int16_t*,std::vector<std::pair<int16_t,int16_t> > > > mapHandlers;
        void setDigInChunk(int x,int y,int rx,int ry,int16_t depth);
        void setDigInChunk(int x,int y,int index,int16_t depth);
        void buildDigCache();
};

}//////server
}//////smoothly

#endif // SMOOTHLY_SERVER_DIG
