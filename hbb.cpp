#include "hbb.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<HBB::AABB> apool;

void HBB::AABB::autoclean(){
    if(left==NULL && right==NULL && !isDataNode()){
        if(parent){
            if(parent->left==this){
                parent->left=NULL;
            }
            if(parent->right==this){
                parent->right=NULL;
            }
            parent->autoclean();
            hbb->delAABB(this);
        }
    }else
    if(parent && parent->parent){
        if(parent->left && parent->right==NULL)
            parent->left=NULL;
        else
        if(parent->left==NULL && parent->right)
            parent->right=NULL;
        else
            return;
        
        if(parent->parent->left==parent){
            parent->parent->left=this;
        }else{
            parent->parent->right=this;
        }
        
        auto tmp=parent;
        parent=parent->parent;
        hbb->delAABB(tmp);
        parent->autoclean();
    }
}

void HBB::AABB::add(AABB * in){
    if(left){
        if(!left->isDataNode() && in->inBox(left)){
            left->add(in);
            return;
        }else
        if(right==NULL){
            setRight(in);
            return;
        }
    }
    if(right){
        if(!right->isDataNode() && in->inBox(right)){
            right->add(in);
            return;
        }else
        if(left==NULL){
            setLeft(in);
            return;
        }
    }
    if(right==NULL && left==NULL){
        setLeft(in);
        return;
    }
    
    auto ls=left->getMergeSizeSq(in);
    auto rs=right->getMergeSizeSq(in);
    auto nnode=hbb->createAABB();
    
    //nnode->parent=this;
    
    if(ls<rs){
        in->merge(left,nnode);
        nnode->setLeft(left);
        nnode->setRight(in);
        this->setLeft(nnode);
    }else{
        in->merge(right,nnode);
        nnode->setLeft(right);
        nnode->setRight(in);
        this->setRight(nnode);
    }
    //debug
    //printf(
    //    "create:(%f,%f,%f)->(%f,%f,%f)\n",
    //    nnode->from.X,nnode->from.Y,nnode->from.Z,
    //    nnode->to.X,nnode->to.Y,nnode->to.Z
    //);
}


void HBB::poolInit(){
    pool=new apool;
}
void HBB::poolDestroy(){
    if(pool)
        delete (apool*)pool;
}

HBB::AABB * HBB::createAABB(){
    if(pool){
        auto p=((apool*)pool)->get();
        p->construct();
        p->hbb=this;
        return p;
    }
}
void HBB::delAABB(HBB::AABB * p){
    if(pool){
        ((apool*)pool)->del(p);
    }
}

void HBB::AABB::remove(){
    if(parent){
        if(parent->left==this){
            parent->left=NULL;
        }
        if(parent->right==this){
            parent->right=NULL;
        }
        parent->autoclean();
        parent=NULL;
    }
}

void HBB::AABB::drop(){
    if(left){
        left->drop();
        left=NULL;
    }
    if(right){
        right->drop();
        right=NULL;
    }
    if(parent){
        if(parent->left==this){
            parent->left=NULL;
        }
        if(parent->right==this){
            parent->right=NULL;
        }
        parent=NULL;
    }
    hbb->delAABB(this);
}

void HBB::AABB::autodrop(){
    auto p=parent;
    this->drop();
    if(p)
        p->autoclean();
}

void HBB::add(HBB::AABB * in){
    root->add(in);
}

void HBB::remove(HBB::AABB * in){
    in->remove();
}

HBB::AABB * HBB::add(const HBB::vec3 & from,const HBB::vec3 & to,void * data){
    auto p=createAABB();
    p->from=from;
    p->to=to;
    p->data=data;
    root->add(p);
    return p;
}

void HBB::AABB::rayTest(
    const irr::core::line3d<irr::f32> & ray,
    void(*callback)(AABB *,void *),
    void * arg
){
    
    if(left && left->intersects(ray)){
        if(left->isDataNode())
            callback(left,arg);
        else
            left->rayTest(ray,callback,arg);
    }
    
    if(right && right->intersects(ray)){
        if(right->isDataNode())
            callback(right,arg);
        else
            right->rayTest(ray,callback,arg);
    }
}
void HBB::AABB::collisionTest(
    const AABB * in,
    void(*callback)(AABB *,void *),
    void * arg
){
    if(left && left->intersects(in)){
        if(left->isDataNode())
            callback(left,arg);
        else
            left->collisionTest(in,callback,arg);
    }
    
    if(right && right->intersects(in)){
        if(right->isDataNode())
            callback(right,arg);
        else
            right->collisionTest(in,callback,arg);
    }
}
void HBB::AABB::fetchByPoint(
    const vec3 & point,
    void(*callback)(AABB *,void *),
    void * arg
){
    if(left && left->inBox(point)){
        if(left->isDataNode())
            callback(left,arg);
        else
            left->fetchByPoint(point,callback,arg);
    }
    if(right && right->inBox(point)){
        if(right->isDataNode())
            callback(right,arg);
        else
            right->fetchByPoint(point,callback,arg);
    }
}
HBB::HBB(){
    poolInit();
    root=createAABB();
}

HBB::~HBB(){
    if(root)
        root->drop();
}

}//namespace smoothly
