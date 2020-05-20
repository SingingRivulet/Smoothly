#ifndef SMOOTHLY_SERVER_BULLET
#define SMOOTHLY_SERVER_BULLET
#include "bag.h"
namespace smoothly{
namespace server{

class bullet:public bag{
    public:
        bullet();
        void shoot(const RakNet::SystemAddress & addr, const std::string & uuid, int id, const vec3 & from, const vec3 & dir);
        virtual void boardcast_shoot(const std::string & user,int id,const vec3 & from,const vec3 & dir)=0;
        struct fire_cost{
                int id;
                int cost_id;//开火消耗
                int cost_num;
                int get_id;//开火获得
                int get_num;
                inline fire_cost(){
                    id          = 0;
                    cost_id     = 0;
                    cost_num    = 0;
                    get_id      = 0;
                    get_num     = 0;
                }
                inline fire_cost(const fire_cost & i){
                    id          = i.id;
                    cost_id     = i.cost_id;
                    cost_num    = i.cost_num;
                    get_id      = i.get_id;
                    get_num     = i.get_num;
                }
                inline const fire_cost & operator=(const fire_cost & i){
                    id          = i.id;
                    cost_id     = i.cost_id;
                    cost_num    = i.cost_num;
                    get_id      = i.get_id;
                    get_num     = i.get_num;
                    return * this;
                }
        };
        std::map<int,fire_cost> fire_costs;
};

}//////server
}//////smoothly
#endif
