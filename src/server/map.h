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
                void toString(std::string &);
                void loadString(const std::string &);
                chunkACL_t() {
                    allowBuildingWrite   = true;
                    allowCharacterDamage = true;
                    allowTerrainItemWrite= true;
                    owner.clear();
                }
        };
        class cache_chunkACL_t:public cache_ord<chunkACL_t,ipair>{
                void onExpire(const ipair &,chunkACL_t & )override;
                void onLoad(const ipair &,chunkACL_t & )override;
            public:
                map * parent;
        }cache_chunkACL;
};

}//////server
}//////smoothly

#endif
