#include "group_ai.h"

namespace smoothly{

group_ai::group_ai(){
    L = luaL_newstate();
}

group_ai::~group_ai(){
    lua_close(L);
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

void group_ai::group::load(cJSON * data, std::shared_ptr<BrainTree::Composite> n){
    if(data && data->type==cJSON_Array){
        auto line = data->child;
        while(line){
            if(line->type == cJSON_Object){
                auto type = cJSON_GetObjectItem(line,"type");
                auto decorator = cJSON_GetObjectItem(line,"decorator");
                auto children = cJSON_GetObjectItem(line,"children");
                if(type->type==cJSON_String){

                    #define addnode \
                        if(decorator && decorator->type==cJSON_String){\
                            if(strcmp(decorator->valuestring,"succeeder")==0){\
                                auto d = std::make_shared<BrainTree::Succeeder>();\
                                d->setChild(newnode);\
                                n->addChild(d);\
                            }else if(strcmp(decorator->valuestring,"failer")==0){\
                                auto d = std::make_shared<BrainTree::Failer>();\
                                d->setChild(newnode);\
                                n->addChild(d);\
                            }else if(strcmp(decorator->valuestring,"repeater")==0){\
                                auto lim = cJSON_GetObjectItem(line,"lim");\
                                int limit = 0;\
                                if(lim && lim->type==cJSON_String){\
                                    limit = lim->valueint;\
                                }\
                                auto d = std::make_shared<BrainTree::Repeater>(limit);\
                                d->setChild(newnode);\
                                n->addChild(d);\
                            }else{\
                                n->addChild(newnode);\
                            }\
                        }else{\
                            n->addChild(newnode);\
                        }

                    if(strcmp(type->valuestring,"action")==0){

                        auto func = cJSON_GetObjectItem(line,"func");
                        auto arg = cJSON_GetObjectItem(line,"arg");
                        if(func && func->type==cJSON_String){
                            auto newnode = std::make_shared<Action>();
                            newnode->G = this;
                            newnode->func = func->valuestring;
                            if(arg && arg->type==cJSON_String){
                                newnode->arg = arg->valuestring;
                            }
                            addnode;
                        }

                    }else if(strcmp(type->valuestring,"selector")==0){
                        auto newnode = std::make_shared<BrainTree::Selector>();
                        if(children){
                            this->load(children , newnode);
                        }
                        addnode;
                    }else if(strcmp(type->valuestring,"sequence")==0){
                        auto newnode = std::make_shared<BrainTree::Sequence>();
                        if(children){
                            this->load(children , newnode);
                        }
                        addnode;
                    }
                    #undef addnode
                }
            }
            line = line->next;
        }
    }
}

void group_ai::group::load(cJSON * data){
    auto sequence = std::make_shared<BrainTree::Sequence>();
    auto rep = std::make_shared<BrainTree::Repeater>();
    rep->setChild(sequence);
    tree.setRoot(rep);
    if(data && data->type==cJSON_Array){
        this->load(data , sequence);
    }
}

}
