#include "group_ai.h"

namespace smoothly{

group_ai::group_ai(){

}

BrainTree::Node::Status group_ai::group::Action::update(){
    BrainTree::Node::Status res = Node::Status::Failure;
    if(!func.empty() && G && G->parent){
        lua_getglobal(G->parent->L , func.c_str());
        if(lua_isfunction(G->parent->L , -1)){
            lua_pushlightuserdata(G->parent->L , G);
            lua_pushstring(G->parent->L , arg.c_str());
            if(lua_pcall(G->parent->L, 2, 1, 0) != 0)
                 printf("error running function : %s \n",lua_tostring(G->parent->L, -1));
            else{
                if(lua_isstring(G->parent->L , -1)){
                    auto str = lua_tostring(G->parent->L , -1);
                    if(strcmp(str,"Success")==0){
                        res = Node::Status::Success;
                    }else if(strcmp(str,"Failure")==0){
                        res = Node::Status::Failure;
                    }else if(strcmp(str,"Running")==0){
                        res = Node::Status::Running;
                    }
                }
            }
        }
        lua_settop(G->parent->L,0);
    }
    return res;
}

}
