#include "body.h"

namespace smoothly{

void body::bodyConf::buildIKTree(scene::ISkinnedMesh::SJoint * ori, ik_node_t * targ){
    u32 size = ori->Children.size();
    for(u32 i=0;i<size;++i){
        auto & processTarget = ori->Children[i];
        auto it = boneIDMap.find(processTarget);
        if(it!=boneIDMap.end()){//骨骼id存在

            auto new_node = ik_solver->node->create_child(targ  , it->second);//在ik中创建节点
            if(new_node){//递归创建子节点
                ik_nodes[it->second] = new_node;
                buildIKTree(processTarget , new_node);
            }

        }
    }
}

void body::bodyItem::uploadIK(){
    int size = config->ik_nodes.size();
    for(int i=0;i<size;++i){
        auto joint = node->getJointNode(i);
        if(joint){
            auto p = joint->getPosition();
            auto r = joint->getRotation();

            auto ikn = config->ik_nodes[i];

            ikn->position.x = p.X;
            ikn->position.y = p.Y;
            ikn->position.z = p.Z;

            irr::core::quaternion rq(r*core::DEGTORAD);
            ikn->rotation.w = rq.W;
            ikn->rotation.x = rq.X;
            ikn->rotation.y = rq.Y;
            ikn->rotation.z = rq.Z;
        }else{
            printf("error:joint %d no found\n",i);
        }
    }
}

void body::bodyItem::solveIK(){
    for(auto it:ik_effectors){
        try{
            auto iknode = config->ik_nodes.at(it.second);//没有节点在这一步会throw
            config->ik_solver->effector->attach(it.first , iknode);
        }catch(...){
            printf("error:joint %d no found\n",it.second);
        }
    }
    ik.solver.rebuild(config->ik_solver);
    ik.solver.update_distances(config->ik_solver);
    ik.solver.solve(config->ik_solver);
    for(auto it:ik_effectors){
        config->ik_solver->effector->detach(it.first);
    }
}

void body::bodyItem::updateIK(){
    int size = config->ik_nodes.size();
    for(int i=0;i<size;++i){
        auto joint = node->getJointNode(i);
        if(joint){
            auto ikn = config->ik_nodes[i];

            joint->setPosition(
                        vec3(
                            ikn->position.x ,
                            ikn->position.y,
                            ikn->position.z
                            )
                        );
            irr::core::quaternion rq;
            rq.W = ikn->rotation.w;
            rq.X = ikn->rotation.x;
            rq.Y = ikn->rotation.y;
            rq.Z = ikn->rotation.z;
            vec3 reu;
            rq.toEuler(reu);
            reu.X = core::radToDeg(reu.X);
            reu.Y = core::radToDeg(reu.Y);
            reu.Z = core::radToDeg(reu.Z);
            joint->setRotation(reu);
        }else{
            printf("error:joint %d no found\n",i);
        }
    }
}

void body::bodyItem::clearIKEffector(){
    if(!ik_effectors.empty() && config && config->ik_solver){
        for(auto it:ik_effectors){
            config->ik_solver->effector->destroy(it.first);
        }
        ik_effectors.clear();
    }
}

ik_effector_t *body::bodyItem::addIKEffector(u32 boneID){
    auto res = config->ik_solver->effector->create();
    ik_effectors.push_back(std::pair<ik_effector_t*,u32>(res,boneID));
    return res;
}

}
