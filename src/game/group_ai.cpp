#include "group_ai.h"
#include <QFile>
#include <QByteArray>

namespace smoothly{

group_ai::group_ai(){
    L = luaL_newstate();
    loadBodyLuaAPI(L);
    group_id = 0;
    groupMaxMember = 8;
    groupMergeLength = 10;
    groupMaxLengthSQ = 20*20;

    QFile file("../../ai/BehaviorTree.json");
    if(file.open(QFile::ReadOnly)){
        QByteArray allData = file.readAll();
        file.close();
        auto str = allData.toStdString();
        groupAITree = cJSON_Parse(str.c_str());
    }
}

group_ai::~group_ai(){
    groups.clear();
    groupBuffer.clear();
    lua_close(L);
    if(groupAITree){
        cJSON_Delete(groupAITree);
    }
}

void group_ai::loop(){
    mail::loop();
    updateGroupAI();
}

void group_ai::createBodyAI(body::bodyItem * b){
    if(groupAITree)
        createGroup()->member.insert(b);
}

std::shared_ptr<group_ai::group> group_ai::createGroup(cJSON * data){
    int id = ++group_id;
    auto p = std::make_shared<group_ai::group>(data);
    p->id = id;
    p->parent = this;
    //groups[id] = p;
    groupBuffer.push_back(p);
    return p;
}

void group_ai::clearEmptyGroup(){
    auto it = groups.begin();
    while(it!=groups.end()){
        if(it->second->member.empty()){
            auto tmp = it;
            ++it;
            groups.erase(tmp);
        }else{
            ++it;
        }
    }
}

void group_ai::mergeGroups(){
    merging.clear();
    for(auto it:groups){
        if(it.second->box && !it.second->merging){//有box说明不是满编
            dbvt3d::AABB box;
            box.from = it.second->box->from;
            box.to = it.second->box->to;
            box.from.X -= groupMergeLength;
            box.from.Y -= groupMergeLength;
            box.from.Z -= groupMergeLength;
            box.to.X += groupMergeLength;
            box.to.Y += groupMergeLength;
            box.to.Z += groupMergeLength;
            struct merge_t{
                    group * a , * b;
                    group * self;
            }mt;
            mt.self = it.second.get();
            mt.a = NULL;
            mt.b = NULL;
            groupBox.collisionTest(&box,[](dbvt3d::AABB * box , void * argp){
                auto g=(group*)(box->data);
                auto self=(merge_t*)argp;
                if(g->merging)
                    return;
                if(g->member.size() + self->self->member.size() <= self->self->parent->groupMaxMember){//人数允许合并
                    if(self->self->canMerge(g)){
                        self->a = self->self;
                        self->b = g;
                    }
                }
            },&mt);
            if(mt.a && mt.b){
                //合并
                mt.a->merging = true;
                mt.b->merging = true;
                merging.push_back(std::pair<int,int>(mt.a->id , mt.b->id));
            }
        }
    }
    for(auto it:merging){
        mergeGroup(it.first , it.second);
    }
    merging.clear();
}

void group_ai::updateGroupAI(){
    clearEmptyGroup();
    for(auto it:groups){
        it.second->merging = false;
        it.second->updateBox();
        try{
            it.second->update();
        }catch(...){}
        it.second->updateGroup();
    }
    addGroupBuffer();
    mergeGroups();
}

void group_ai::addGroupBuffer(){
    for(auto it:groupBuffer){
        groups[it->id] = it;
    }
    groupBuffer.clear();
}

void group_ai::mergeGroup(int a, int b){
    auto it1 = groups.find(a);
    auto it2 = groups.find(b);
    if(it1!=groups.end() && it2!=groups.end()){
        if(it1->second->member.size() >= it2->second->member.size()){
            for(auto it:it2->second->member){
                it1->second->member.insert(it);
            }
            groups.erase(it2);
        }else{
            for(auto it:it1->second->member){
                it2->second->member.insert(it);
            }
            groups.erase(it1);
        }
    }
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

void group_ai::group::updateGroup(){
    auto it = member.begin();
    while(it!=member.end()){
        if(((*it)->node->getPosition()-center).getLengthSQ()>parent->groupMaxLengthSQ){
            //放入单独的小队
            auto tmpg = parent->createGroup();
            tmpg->member.insert(*it);
            auto tmp = it;
            ++it;
            member.erase(tmp);
        }else{
            ++it;
        }
    }
}

void group_ai::group::updateBox(){
    //计算中心点
    vec3 sum(0,0,0);
    int num = 0;
    for(auto it:member){
        sum+=it->node->getPosition();
        ++num;
    }
    if(num>0){
        sum.X /= num;
        sum.Y /= num;
        sum.Z /= num;
    }
    center = sum;
    //满编小队没有box
    if(member.size()>=parent->groupMaxMember){
        if(box){
            box->autodrop();
        }
        box = NULL;
    }else{
        auto it = member.begin();
        if(it!=member.end()){
            irr::core::aabbox3df tmpbox((*it)->node->getPosition());
            for(;it!=member.end();++it){
                tmpbox.addInternalPoint((*it)->node->getPosition());
            }
            if(box){
                if((tmpbox.MinEdge-box->from).getLengthSQ()<1 && (tmpbox.MaxEdge-box->to).getLengthSQ()<1){

                }else{
                    box->autodrop();
                    box = parent->groupBox.add(tmpbox.MinEdge , tmpbox.MaxEdge , this);
                }
            }else{
                box = parent->groupBox.add(tmpbox.MinEdge , tmpbox.MaxEdge , this);
            }
        }
    }
}

bool group_ai::group::canMerge(group_ai::group * t){
    lua_settop(parent->L,0);
    bool res = false;
    lua_getglobal(parent->L , "canMerge");
    if(lua_isfunction(parent->L , -1)){
        lua_pushlightuserdata(parent->L , this);
        lua_pushlightuserdata(parent->L , t);
        if(lua_pcall(parent->L, 2, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(parent->L, -1));
        else{
            if(lua_isboolean(parent->L , -1)){
                res = lua_toboolean(parent->L , -1);
            }
        }
    }
    lua_settop(parent->L,0);
    return res;
}

}
