#ifndef SMOOTHLY_SERVER_MAP
#define SMOOTHLY_SERVER_MAP
#include "db.h"
#include <exception>
#include <string>
#include <set>
#include <vector>
#include <list>
#include <iostream>
#include <functional>

#include "cache.h"
#include "../utils/cJSON.h"

#define CHUNKACL_UPDATE_TIME_OUT 30

namespace smoothly{
namespace server{

class map:public datas{
    public:
        void updateNode(const std::string & uuid,int x,int y,std::function<void (int,int)> ncallback);
        void addNode(const std::string & uuid,const std::string & owner,int x,int y);
        void removeNode(const std::string & uuid);
        void getNode(int x,int y,std::function<void (const std::string &)> callback);
        void getUsers(int x,int y,std::set<std::string> & o);
        
        std::string getNodeOwner(const std::string & uuid);
        void getUserNodes(const std::string & owner , std::set<ipair> & o, std::function<void(const std::string &, int, int)> bcallback);
        
        int visualField;
        virtual void loop();
        virtual void release();

        inline map(){
            cache_nodePosi.parent = this;
            cache_chunkACL.parent = this;
            userMaxChunk = 32;
        }
    private:
        static std::string getNodePrefix(int x,int y);
        void buildVisualFieldArray(int x,int y,std::function<void (int,int)> ncallback);
    public:

        class cache_nodePosi_t:public cache<ipair>{
                void onExpire(const std::string &,ipair & )override;
                void onLoad(const std::string &,ipair & )override;
            public:
                map * parent;
        }cache_nodePosi;

        struct chunkACL_t{
                bool allowBuildingWrite;    //是否允许修改建筑物
                bool allowCharacterDamage;  //是否允许单位受到伤害
                bool allowTerrainItemWrite; //是否允许修改地形物体（如植被）
                std::string owner;          //拥有者（可以破坏建筑物和物体而不受权限影响）
                time_t lastUpdateTiem;
                void toString(std::string &);
                void loadString(const std::string &);
                void set(bool b,bool c,bool t){
                    allowBuildingWrite = b;
                    allowCharacterDamage = c;
                    allowTerrainItemWrite = t;
                }
                chunkACL_t() {
                    allowBuildingWrite   = true;
                    allowCharacterDamage = true;
                    allowTerrainItemWrite= true;
                    lastUpdateTiem = 0;
                    owner.clear();
                }
        };
        class cache_chunkACL_t:public cache_ord<chunkACL_t,ipair>{
                void onExpire(const ipair &,chunkACL_t & )override;
                void onLoad(const ipair &,chunkACL_t & )override;
            public:
                map * parent;
        }cache_chunkACL;
        virtual void boardcast_chunkACL(const ipair & posi, map::chunkACL_t & acl)=0;
        void setChunkACL(int x,int y,bool b,bool c,bool t);
        void setChunkACL(const std::string & user,int x,int y,bool b,bool c,bool t);
        void setChunkOwner(int x,int y,const std::string & user);
        void giveUpChunk(const std::string & user,int x,int y);

        int getUserChunkNum(const std::string & user);
        void setUserChunkNum(const std::string & user,int num);
        int userMaxChunk;
};

}//////server
}//////smoothly

#endif
