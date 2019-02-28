#ifndef SMOOTHLY_SERVER_MOD
#define SMOOTHLY_SERVER_MOD
#include "removeTable.h"
#include "graphServer.h"
namespace smoothly{
    class serverMod:public removeTable,public graphServer{
        public:
            virtual int getHPByType(long type);
            virtual int itemidNum(long id);
            void scriptInit(const char * path);
            float viewRange;
            std::map<long,int> buildingHPMap;
            std::map<long,int> terrainNumMap;
    };
}
#endif