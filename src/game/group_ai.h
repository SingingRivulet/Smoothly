#ifndef SMOOTHLY_GROUP_AI
#define SMOOTHLY_GROUP_AI
#include "BrainTree.h"
#include "../utils/cJSON.h"
#include <lua.hpp>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "mail.h"

namespace smoothly{

class group_ai:public mail{
    public:
        group_ai();
        ~group_ai();
        struct group{
                group_ai * parent;
                BrainTree::BehaviorTree tree;
                int id;
                vec3 center;//中心点
                dbvt3d::AABB * box;
                bool merging;
                std::set<bodyItem*> member;//小队成员
                class Action : public BrainTree::Node{
                    public:
                        group * G;
                        std::string func;
                        std::string arg;
                        Status update() override;
                };
                group(cJSON * data) {
                    this->load(data);
                    box = NULL;
                }
                ~group(){
                    if(box){
                        box->autodrop();
                    }
                }
                void update(){
                    tree.update();
                }
                void load(cJSON * data , std::shared_ptr<BrainTree::Composite> n);//data指向children元素
                void load(cJSON * data);
                void updateGroup();
                void updateBox();
                bool canMerge(group*);
        };

        std::map<int,std::shared_ptr<group> > groups;
        dbvt3d groupBox;//小队包围盒，作为合并小队的信标

        float groupMergeLength;//两个小队距离过近会合并
        float groupMaxLengthSQ;//超过会当做掉队（保存为平方的形式）
        u32 groupMaxMember;

        void loop()override;
        void createBodyAI(bodyItem*)override;

    private:
        int group_id;
        lua_State *  L;
        cJSON * groupAITree;
        std::vector<std::pair<int,int> > merging;
        std::vector<std::shared_ptr<group> > groupBuffer;
        void addGroupBuffer();
        void mergeGroup(int a , int b);
        std::shared_ptr<group> createGroup(cJSON * data);
        std::shared_ptr<group> createGroup(){
            return createGroup(groupAITree);
        }

        void clearEmptyGroup();
        void mergeGroups();

        void updateGroupAI();
};

}

#endif // SMOOTHLY_GROUP_AI
