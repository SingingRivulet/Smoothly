#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING

#include "bullet.h"
#include "buildinggraph.h"

namespace smoothly{
namespace server{

class building:public bullet{
    public:
        building(int thnum);
        ~building();
        void createBuilding(const vec3 & position , const vec3 & rotation , const std::list<std::string> & conn , int id);
        void removeBuilding(const std::string & uuid);
        void damageBuilding(const std::string & uuid , int dthp);
        void getBuildingChunk(int x, int y,
                std::function<void(const std::string &, const vec3 &, const vec3 &, int)> callback);
        void loop();

        virtual void boardcast_buildingRemove(const std::string & uuid , int x,int y)=0;
        virtual void boardcast_buildingAdd(const std::string & uuid ,int id,const vec3 & p , const vec3 & r, int x,int y)=0;

        void release()override;
    private:

        void releaseBuilding(const std::string & uuid);//释放数据库和缓存中的建筑物

        buildingGraph buildingSolver;//建筑求解器

        class cache_building_hp_t:public cache<int>{//建筑血量缓存
            friend class building;
                void onExpire(const std::string &,int & )override;
                void onLoad(const std::string &,int & )override;
            protected:
                building * parent;
        }cache_building_hp;

        struct buildingStatus{
                vec3 position,rotation;
                int id;
                buildingStatus():position(0,0,0),rotation(0,0,0),id(0) {}
                buildingStatus(const buildingStatus & bs){
                    position = bs.position;
                    rotation = bs.rotation;
                    id       = bs.id;
                }
                const buildingStatus & operator =(const buildingStatus & bs){
                    position = bs.position;
                    rotation = bs.rotation;
                    id       = bs.id;
                    return *this;
                }
        };

        class cache_building_transform_t:public cache<buildingStatus>{//建筑位置，旋转
            friend class building;
                void onExpire(const std::string &,buildingStatus & )override;
                void onLoad(const std::string &,buildingStatus & )override;
            protected:
                building * parent;
        }cache_building_transform;

        struct conf{
                int hp;
                inline conf(){
                    hp=1;
                }
        };
        std::map<int,conf*> config;
};

}//////server
}//////smoothly
#endif // SMOOTHLY_BUILDING
