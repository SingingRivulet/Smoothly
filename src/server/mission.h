#ifndef SMOOTHLY_SERVER_MISSION
#define SMOOTHLY_SERVER_MISSION

#include "building.h"

namespace smoothly{
namespace server{

class mission:public building{
    public:
        mission(int thnum);
        void loop()override;
        void release()override;

        struct mission_node_t{//任务节点
                bool needArrive;
                bool showPosition;//在玩家界面上显示任务地点
                vec3 position;//任务目标地点
                std::vector<resource_t> need,  //需要提交物品
                                        reward;//奖励
                std::string description;
                std::string parent;//父节点
                //任务完成的条件：
                //1.到达目的地
                //2.提交物品足够
                //3.parent为空或已完成
                mission_node_t() {
                    showPosition = false;
                    needArrive = true;
                }
                mission_node_t(const mission_node_t & i) {
                    needArrive  = i.needArrive;
                    position    = i.position;
                    need        = i.need;
                    reward      = i.reward;
                    description = i.description;
                    parent      = i.parent;
                    showPosition = i.showPosition;
                }
                const mission_node_t & operator=(const mission_node_t & i) {
                    needArrive  = i.needArrive;
                    position    = i.position;
                    need        = i.need;
                    reward      = i.reward;
                    description = i.description;
                    parent      = i.parent;
                    showPosition = i.showPosition;
                    return *this;
                }
                void loadString(const std::string & );
                void toString(std::string &);
        };
        class cache_mission_node_t:public cache<mission_node_t>{
                void onExpire(const std::string &,mission_node_t & )override;
                void onLoad(const std::string &, mission_node_t & )override;
            public:
                mission * parent;
        }cache_mission_node;


        struct mission_children_t{
                std::vector<std::string> children;//子节点
                void loadString(const std::string & );
                void toString(std::string &);
                mission_children_t(){}
                mission_children_t(const mission_children_t & i){
                    children = i.children;
                }
                const mission_children_t & operator=(const mission_children_t & i){
                    children = i.children;
                    return *this;
                }
        };
        class cache_mission_children_t:public cache<mission_children_t>{
                void onExpire(const std::string &,mission_children_t & )override;
                void onLoad(const std::string &, mission_children_t & )override;
            public:
                mission * parent;
        }cache_mission_children;


        bool isDone(const std::string & user,const std::string & mission_uuid);//检查任务是否完成过
        void setDone(const std::string & user,const std::string & mission_uuid);

        void setNowMissionParent(const std::string & user,const std::string & mission_uuid);//当前选择的剧情树的父节点
        void getNowMissionParent(const std::string & user,std::string & mission_uuid);
        inline void giveUpMission(const RakNet::SystemAddress & addr,const std::string & user){//放弃任务
            setNowMissionParent(user,"");
            sendAddr_missionList(addr,std::vector<std::string>());
        }
        inline void sendNowMission(const RakNet::SystemAddress & addr,const std::string & user){
            try{
                std::string uuid;
                getNowMissionParent(user,uuid);
                if(uuid.empty()){
                    sendAddr_missionList(addr,std::vector<std::string>());
                }else{
                    sendAddr_missionList(addr,cache_mission_children[uuid].children);
                }
            }catch(...){
                sendAddr_missionList(addr,std::vector<std::string>());
            }
        }

        //提交任务
        //要求：
        //父节点已完成或不存在父节点
        bool submitMission(const RakNet::SystemAddress & addr,  //客户端
                           const std::string & user,            //提交任务的用户
                           const std::string & body,            //提交任务的单位
                           const std::string & mission_uuid);   //提交的任务id

        void getChunkMissions(int x,int y,std::function<void(const std::string & ,const vec3 &)> callback);
        void setChunkMissions(const std::string & uuid,const vec3 & posi);

        void addMission(const std::string &uuid,mission_node_t & m);
        inline void addMission(mission_node_t & m){
            std::string uuid;
            getUUID(uuid);
            addMission(uuid,m);
        }

        virtual void sendAddr_missionList(const RakNet::SystemAddress & addr,const std::vector<std::string> & )=0;
};

}
}

#endif // SMOOTHLY_SERVER_MISSION
