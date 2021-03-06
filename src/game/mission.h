#ifndef SMOOTHLY_MISSION
#define SMOOTHLY_MISSION
#include "technology.h"
#include <codecvt>
#include <locale>
namespace smoothly{

class mission:public technology{
    public:

        void msg_addMission(const char * uuid , float x,float y,float z)override;
        void msg_setMission(const char * uuid , const char * text)override;
        void msg_submitMissionStatus(const char * uuid , bool status)override;
        void msg_missionList(const std::vector<std::string> & missions)override;
        void msg_missionText(const char * uuid , const char * text)override;

        mission();
        ~mission();

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
                std::vector<std::wstring> description_buffer;
                void updateBuffer();
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
                    node = NULL;
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
                    node        = i.node;
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
                    node        = i.node;
                    return *this;
                }
                void loadString(const std::string & );
                void toString(std::string &);
                dbvt3d::AABB * box;
                irr::scene::ISceneNode * node;
        };
        std::unordered_map<std::string,mission_node_t *> missions;
        dbvt3d missions_indexer;
        void releaseMission(mission_node_t*);
        void getMission(const vec3 & posi, std::vector<mission_node_t *> & m);

        int printString(const std::vector<std::wstring> & str, int start=64);
        void printString(const std::wstring & str, int h);
        void drawNearMission(int h);
        std::vector<mission_node_t*> mission_result;

        std::vector<std::wstring> missionText_buffer;

        void onDraw()override;
        void loop()override;

        bool showMissionText;
        bool submitShowingMissions;
        time_t lastSubmitMissionsTime;

        void addMissionWindow();
        void addMission(const std::string & title, const std::string & description, bool showPosi);
        void goParentMission();

        virtual std::string getMissionAccepter()=0;

        std::string missionParentUUID;
        mission_node_t * missionParent;

        void setFullMapMode(bool m)override;

        irr::video::ITexture * texture_mission_point;
        irr::video::ITexture * texture_mission_target;

        irr::gui::IGUIButton * button_mission_giveup;

    public:
        bool openMissionEditBox;

    public:
        void scanAnimate(){
            scan_animation_time = timer->getTime();
            scan_animation_showing = 1;
        }
};

}

#endif // SMOOTHLY_MISSION
