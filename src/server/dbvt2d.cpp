#include "dbvt2d.h"
#include "mempool.h"
namespace smoothly {
namespace server {

typedef mempool<dbvt2d::AABB> apool;

void dbvt2d::AABB::autoclean(){
    if(left==NULL && right==NULL && !isDataNode()){
        if(parent){
            if(parent->left==this){
                parent->left=NULL;
            }
            if(parent->right==this){
                parent->right=NULL;
            }
            parent->autoclean();
            bb->delAABB(this);
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
        bb->delAABB(tmp);
        parent->autoclean();
    }
}

void dbvt2d::AABB::add(AABB * in){
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
    auto nnode=bb->createAABB();

    //nnode->parent=this;

    #define insertLeft \
        in->merge(left,nnode); \
        nnode->setLeft(left); \
        nnode->setRight(in); \
        this->setLeft(nnode);

    #define insertRight \
        in->merge(right,nnode); \
        nnode->setLeft(right); \
        nnode->setRight(in); \
        this->setRight(nnode);

    if(ls<rs){
        insertLeft;
    }else
    if(ls>rs){
        insertRight;
    }else{
        //等于，轮流在两边插入
        if(insertFlag){
            insertLeft;
        }else {
            insertRight;
        }
        insertFlag = !insertFlag;
    }
    //debug
    //printf(
    //    "create:(%f,%f,%f)->(%f,%f,%f)\n",
    //    nnode->from.X,nnode->from.Y,nnode->from.Z,
    //    nnode->to.X,nnode->to.Y,nnode->to.Z
    //);
}

void dbvt2d::poolInit(){
    pool=new apool;
}
void dbvt2d::poolDestroy(){
    if(pool)
        delete (apool*)pool;
}

dbvt2d::AABB * dbvt2d::createAABB(){
    if(pool){
        auto p=((apool*)pool)->get();
        p->construct();
        p->bb=this;
        return p;
    }
    return NULL;
}
void dbvt2d::delAABB(dbvt2d::AABB * p){
    if(pool){
        ((apool*)pool)->del(p);
    }
}

void dbvt2d::AABB::remove(){
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

void dbvt2d::AABB::drop(){
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
    bb->delAABB(this);
}

void dbvt2d::AABB::autodrop(){
    auto p=parent;
    this->drop();
    if(p)
        p->autoclean();
}

void dbvt2d::add(dbvt2d::AABB * in){
    root->add(in);
}

void dbvt2d::remove(dbvt2d::AABB * in){
    in->remove();
}

dbvt2d::AABB * dbvt2d::add(const dbvt2d::vec & from,const dbvt2d::vec & to,void * data){
    auto p=createAABB();
    p->from=from;
    p->to=to;
    p->data=data;
    root->add(p);
    return p;
}

void dbvt2d::AABB::collisionTest(
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
void dbvt2d::AABB::fetchByPoint(
    const vec & point,
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
dbvt2d::dbvt2d(){
    poolInit();
    root=createAABB();
}

dbvt2d::~dbvt2d(){
    if(root)
        root->drop();
}

}//server
}//smoothly
