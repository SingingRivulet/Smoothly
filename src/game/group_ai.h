#ifndef SMOOTHLY_GROUP_AI
#define SMOOTHLY_GROUP_AI
#include "BrainTree.h"
#include "../utils/cJSON.h"
#include <lua.hpp>
#include <string.h>
#include <vector>
#include "mail.h"

namespace smoothly{

class group_ai:public mail{
    public:
        group_ai();
        ~group_ai();
        struct group{
                group_ai * parent;
                BrainTree::BehaviorTree tree;
                class Action : public BrainTree::Node{
                    public:
                        group * G;
                        std::string func;
                        std::string arg;
                        Status update() override;
                };
                group(cJSON * data) {
                    this->load(data);
                }
                void update(){
                    tree.update();
                }
                void load(cJSON * data , std::shared_ptr<BrainTree::Composite> n);//data指向children元素
                void load(cJSON * data);
        };
    private:
        lua_State *  L;
};

}

#endif // SMOOTHLY_GROUP_AI
