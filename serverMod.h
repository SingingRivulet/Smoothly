#ifndef SMOOTHLY_SERVER_MOD
#define SMOOTHLY_SERVER_MOD
#include "removeTable.h"
#include "graphServer.h"
#include "user.h"
namespace smoothly{
    class serverMod:public removeTable,public graphServer,public users{
        public://virtual functions
            virtual int getHPByType(long type);
            virtual int itemidNum(long id);
            virtual int getUserMaxSubs();
            virtual bool getSubsConf(long id , bool & isLasting , int & hp);
            bool checkAdminPwd(const std::string & name,const std::string & pwd);
        public:
            void scriptInit(const char * path);
            void scriptDestroy();
        public:
            struct subsConf{
                inline const subsConf & operator=(const subsConf & i){
                    lasting=i.lasting;
                    hp=i.hp;
                    return *this;
                }
                inline subsConf(const subsConf & i){
                    lasting=i.lasting;
                    hp=i.hp;
                }
                inline subsConf(){
                    lasting=true;
                    hp=1;
                }
                bool lasting;
                int hp;
            };
            float viewRange;
            int userMaxSubs;
            std::map<long,int> buildingHPMap;
            std::map<long,int> terrainNumMap;
            std::map<long,subsConf> subsMap;
            std::unordered_map<std::string,std::string> adminPwd;
            lua_State * L;
    };
}
#endif