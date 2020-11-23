#ifndef SMOOTHLY_GROUP_AI
#define SMOOTHLY_GROUP_AI
#include "BrainTree.h"
#include "../utils/cJSON.h"
#include <lua.hpp>
#include <string.h>
namespace smoothly{

class group_ai{
    public:
        group_ai();
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
                group() {}
                void update(){
                    tree.update();
                }
        };
    private:
        lua_State *  L;
};

}

#endif // SMOOTHLY_GROUP_AI
