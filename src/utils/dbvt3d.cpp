#include "dbvt3d.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<dbvt3d::AABB> apool;

void dbvt3d::AABB::autoclean(){
    if(left==NULL && right==NULL && !isDataNode()){
        if(parent){
            if(parent->left==this){
                parent->left=NULL;
            }
            if(parent->right==this){
                parent->right=NULL;
            }
            parent->autoclean();
            box->delAABB(this);
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
        box->delAABB(tmp);
        parent->autoclean();
    }
}

void dbvt3d::AABB::add(AABB * in){
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
    auto nnode=box->createAABB();
    
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


void dbvt3d::poolInit(){
    pool=new apool;
}
void dbvt3d::poolDestroy(){
    if(pool)
        delete (apool*)pool;
}

dbvt3d::AABB * dbvt3d::createAABB(){
    if(pool){
        auto p=((apool*)pool)->get();
        p->construct();
        p->box=this;
        return p;
    }
    return NULL;
}
void dbvt3d::delAABB(dbvt3d::AABB * p){
    if(pool){
        ((apool*)pool)->del(p);
    }
}

void dbvt3d::AABB::remove(){
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

void dbvt3d::AABB::drop(){
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
    box->delAABB(this);
}

void dbvt3d::AABB::autodrop(){
    auto p=parent;
    this->drop();
    if(p)
        p->autoclean();
}

void dbvt3d::add(dbvt3d::AABB * in){
    root->add(in);
}

void dbvt3d::remove(dbvt3d::AABB * in){
    in->remove();
}

dbvt3d::AABB * dbvt3d::add(const dbvt3d::vec3 & from,const dbvt3d::vec3 & to,void * data){
    auto p=createAABB();
    p->from=from;
    p->to=to;
    p->data=data;
    root->add(p);
    return p;
}

void dbvt3d::AABB::rayTest(
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
void dbvt3d::AABB::collisionTest(
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
void dbvt3d::AABB::fetchByPoint(
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
dbvt3d::dbvt3d(){
    poolInit();
    root=createAABB();
}

dbvt3d::~dbvt3d(){
    if(root)
        root->drop();
}

void dbvt3d::makeID(dbvt3d::AABB * p){
    p->id = ++tmpid;
    onMakeID(p);
    if(p->left)
        makeID(p->left);
    if(p->right)
        makeID(p->right);
}

}//namespace smoothly
