#ifndef SMOOTHLY_MISSION
#define SMOOTHLY_MISSION
#include "technology.h"
namespace smoothly{

class mission:public technology{
    public:

        void msg_addMission(const char * uuid , float x,float y,float z)override;
        void msg_setMission(const char * uuid , const char * text)override;
        void msg_submitMissionStatus(const char * uuid , bool status)override;
        void msg_missionList(int len,const std::vector<std::string> & missions)override;

        mission();

        struct resource_t{
                int id,num;
                resource_t(){
                    id = 0;
                    num = 0;
                }
                resource_t(const resource_t & i){
                    id = i.id;
                    num = i.num;
                }
                resource_t(int i,int n){
                    id = i;
                    num = n;
                }
        };
        struct mission_node_t{//任务节点
                bool needArrive;
                bool showPosition;//在玩家界面上显示任务地点
                vec3 position;//任务目标地点
                std::vector<resource_t> need,  //需要提交物品
                                        reward;//奖励
                std::string description;
                std::string uuid;
                std::string parent;//父节点
                //任务完成的条件：
                //1.到达目的地
                //2.提交物品足够
                //3.parent为空或已完成
                mission_node_t() {
                    showPosition = false;
                    needArrive = true;
                    box = NULL;
                    uuid.clear();
                }
                mission_node_t(const mission_node_t & i) {
                    needArrive  = i.needArrive;
                    position    = i.position;
                    need        = i.need;
                    reward      = i.reward;
                    description = i.description;
                    parent      = i.parent;
                    showPosition = i.showPosition;
                    box         = i.box;
                    uuid        = i.uuid;
                }
                const mission_node_t & operator=(const mission_node_t & i) {
                    needArrive  = i.needArrive;
                    position    = i.position;
                    need        = i.need;
                    reward      = i.reward;
                    description = i.description;
                    parent      = i.parent;
                    showPosition = i.showPosition;
                    box         = i.box;
                    uuid        = i.uuid;
                    return *this;
                }
                void loadString(const std::string & );
                void toString(std::string &);
                dbvt3d::AABB * box;
        };
        std::unordered_map<std::string,mission_node_t *> missions;
        dbvt3d missions_indexer;
        void releaseMission(mission_node_t*);
        void getMission(const vec3 & posi, std::vector<mission_node_t *> & m);

};

}

#endif // SMOOTHLY_MISSION
