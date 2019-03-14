#include "building.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<buildings::building> cbpool;

buildings::building * buildings::createCBuild(){
    if(!cBuildPool)
        return NULL;
    auto p=((cbpool*)cBuildPool)->get();
    p->node=NULL;
    p->data=NULL;
    return p;
}

void buildings::delCBuilding(buildings::building * p){
    if(!cBuildPool)
        return;
    ((cbpool*)cBuildPool)->del(p);
}

buildings::buildings(){
    cBuildPool=new cbpool;
    meshSize=2.0f;
}
buildings::~buildings(){
    delete (cbpool*)cBuildPool;
}
bool buildings::collisionWithTerrain(irr::scene::IMeshSceneNode * n,long type){
    auto it=m->buildings.find(type);
    if(it==m->buildings.end())
        return false;
    irr::core::aabbox3d<irr::f32> box=it->second->BB;
    n->getAbsoluteTransformation().transformBoxEx(box);
    
    float b=box.MinEdge.Y;
    
    float fx=box.MinEdge.X;
    float fy=box.MinEdge.Z;
    float tx=box.MaxEdge.X;
    float ty=box.MaxEdge.Z;
    
    if(tx-fx<=meshSize && ty-fy<=meshSize){
        if(getRealHight(fx,fy)>=b)
            return true;
    }else{
        for(float i=fx;i<=tx;i+=meshSize){
            for(float j=fy;j<=ty;j+=meshSize){
                if(getRealHight(i,j)>=b)
                    return true;
            }
        }
    }
    return false;
}
void buildings::onGenChunk(terrain::chunk *){
    
}
void buildings::onFreeChunk(terrain::chunk *){
    
}

void buildings::onFreeTerrainItem(terrain::item * i){
    auto it=allBuildings.find(i);
    if(it==allBuildings.end())
        return;
    it->second->node->autodrop();
    delCBuilding(it->second);
    allBuildings.erase(it);
}

void buildings::onGenTerrainItem(terrain::item * i){
    if(allBuildings.find(i)!=allBuildings.end())
        return;
    auto mit=m->items.find(i->id);
    if(mit==m->items.end())
        return;
    if(!mit->second->haveBB)
        return;
    if(!i->node)
        return;
    auto posi=i->node->getPosition();
    auto p=createCBuild();
    p->type=building::BUILDING_TERRAIN;
    p->data=i;
    allBuildings[i]=p;
    p->node=buildingHBB.add(mit->second->BB.MinEdge+posi , mit->second->BB.MaxEdge+posi , p);
}

buildings::building * buildings::collisionWithBuildings(const irr::core::line3d<irr::f32> & ray,irr::core::line3d<irr::f32> &normal){
    struct cwbtmp{
        float lenSq;
        irr::core::vector3df position;
        irr::core::triangle3df triangle;
        bool havevalue;
        buildings * self;
        irr::core::line3d<irr::f32> ray;
        building * node;
    }arg;
    arg.havevalue=false;
    arg.self=this;
    arg.ray=ray;
    arg.node=NULL;
    buildingHBB.rayTest(ray,[](HBB::AABB * box , void * argp){
        auto bd=(building*)(box->data);
        if(bd->type==building::BUILDING_TERRAIN)
            return;
        
        auto arg=(cwbtmp*)argp;
        auto self=arg->self;
        
        //use selector to get collision point
        auto pitem=(remoteGraph::item*)bd->data;
        
        auto selector=pitem->node->getTriangleSelector();
        
        if(pitem->node==NULL)
            return;
        
        irr::core::triangle3df t;
        irr::core::vector3df position;
        irr::scene::ISceneNode * outNode;
        
        if(!self->scene->getSceneCollisionManager()->getCollisionPoint(
            arg->ray,
            selector,
            position,
            t,
            outNode
        ))return;
        
        auto sq=getLenSq(arg->ray.start , position);
        if(arg->havevalue==false || sq < arg->lenSq){
            arg->havevalue=true;
            arg->lenSq=sq;
            arg->triangle=t;
            arg->position=position;
            arg->node=bd;
        }
        
    },&arg);
    if(arg.havevalue==false)
        return NULL;
    //get normal
    irr::core::vector3df n;
    getTriangleNormal(arg.triangle , ray.start , n);
    normal.start=arg.position;
    normal.end=arg.position+n;
    return arg.node;
}

void buildings::onGenBuilding(remoteGraph::item * i){
    auto it=m->buildings.find(i->type);
    if(it==m->buildings.end()){
        i->node=NULL;
        i->rigidBody=NULL;
        return;
    }
    i->node=scene->addMeshSceneNode(it->second->mesh);
    i->node->setPosition(i->position);
    i->node->setRotation(i->rotation);
    i->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    
    irr::core::aabbox3d<irr::f32> box=it->second->BB;
    i->node->updateAbsolutePosition();
    i->node->getAbsoluteTransformation().transformBoxEx(box);
    
    if(it->second->bodyShape){
        i->bodyState=setMotionState(i->node->getAbsoluteTransformation().pointer());
        i->rigidBody=createBody(it->second->bodyShape,i->bodyState);
        i->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        dynamicsWorld->addRigidBody(i->rigidBody);
    }else
        i->rigidBody=NULL;
    
    auto sel=scene->createOctreeTriangleSelector(it->second->mesh,i->node);
    i->node->setTriangleSelector(sel);
    sel->drop();
    
    auto p=createCBuild();
    p->type=building::BUILDING_ITEM;
    p->data=i;
    
    allBuildings[i]=p;
    
    p->node=buildingHBB.add(box.MinEdge , box.MaxEdge , p);
    //i->node->remove();
}

void buildings::onFreeBuilding(remoteGraph::item * i){
    if(i->node)
        i->node->remove();
        
    if(i->rigidBody){
        dynamicsWorld->removeRigidBody(i->rigidBody);
        delete i->rigidBody;
        delete i->bodyState;
    }
    auto it=allBuildings.find(i);
    if(it==allBuildings.end())
        return;
    it->second->node->autodrop();
    delCBuilding(it->second);
    allBuildings.erase(it);
}

void buildings::onDestroyBuilding(remoteGraph::item * i){
    onFreeBuilding(i);
}
void buildings::onCreateBuilding(remoteGraph::item * i){
    //printf("createBuilding:%ld\n",i->type);
    onGenBuilding(i);
}
bool buildings::collisionWithBuildings(irr::scene::IMeshSceneNode * n,long type,std::list<building*> & b){
    b.clear();
    auto it=m->buildings.find(type);
    if(it==m->buildings.end())
        return false;
    
    irr::core::aabbox3d<irr::f32> box=it->second->BB;
    n->getAbsoluteTransformation().transformBoxEx(box);
    
    struct cwbtmp{
        std::list<building*> * b;
    }arg;
    
    arg.b=&b;
    
    HBB::AABB bb;
    bb.from=box.MinEdge;
    bb.to=box.MaxEdge;
    
    buildingHBB.collisionTest(&bb,[](HBB::AABB * box , void * argp){
        
        auto bd=(building*)(box->data);
        
        auto arg=(cwbtmp*)argp;
        
        arg->b->push_back(bd);
        
    },&arg);
    
    return !b.empty();
}

bool buildings::getNormalByRay(const irr::core::line3d<irr::f32> & ray,irr::core::line3d<irr::f32> &normal){
    irr::core::vector3df outCollisionPoint;
    irr::core::triangle3df outTriangle;
    irr::scene::ISceneNode* outNode;
    
    if(selectPointM(ray,outCollisionPoint,outTriangle,outNode)){
        irr::core::vector3df n1;
        irr::core::line3d<irr::f32> l1;
        if(collisionWithBuildings(ray,l1)){
            if(getLenSq(ray.start,l1.start)<getLenSq(ray.start,outCollisionPoint)){
                normal=l1;
                return true;
            }else{
                getTriangleNormal(outTriangle , ray.start , n1);
                normal.start=outCollisionPoint;
                normal.end=outCollisionPoint+n1;
                return true;
            }
        }else{
            getTriangleNormal(outTriangle , ray.start , n1);
            normal.start=outCollisionPoint;
            normal.end=outCollisionPoint+n1;
            return true;
        }
    }else
        return collisionWithBuildings(ray,normal);
}

void buildings::transformByNormal(irr::scene::IMeshSceneNode * n,const irr::core::line3d<irr::f32> & normal){
    //normal:某一平面的法向量
    //将物体n变换后贴到该平面表面
    //auto v1=irr::core::vector3df(0,1,0);//朝上的坐标
    //auto vn=normal.end-normal.start;//求法线向量
    auto vc=cameraRay.end-cameraRay.start;
    vc.normalize();
    auto ang=vc.getHorizontalAngle();
    ang.X=0;
    
    //irr::core::vector3df eu;
    //irr::core::quaternion qu;//用于保存旋转的四元数
    
    //qu.rotationFromTo(v1,vn);//求从(0,1,0)旋转到法线的四元数
    
    //qu.toEuler(eu);//转换四元数为欧拉角
    
    n->setPosition(normal.start);//更新n的坐标
    n->setRotation(ang);//更新n的旋转欧拉角
    n->updateAbsolutePosition();//更新
}

bool buildings::canBuild(irr::scene::IMeshSceneNode * n,long type){
    std::list<building*> l;
    if(collisionWithBuildings(n,type,l)){
        if(l.size()>4)
            return false;
        else
            return true;
    }else
    if(collisionWithTerrain(n,type))
        return true;
    else
        return false;
}

void buildings::doBuildBegin(long type){
    if(buildingMode)
        doBuildEnd();
    buildingMode=true;
    buildingType=type;
    auto mit=m->buildings.find(type);
    if(mit==m->buildings.end() || mit->second==NULL){
        buildingConfig=NULL;
        buildingNode=NULL;
        return;
    }
    buildingConfig=mit->second;
    buildingNode=scene->addMeshSceneNode(
        buildingConfig->mesh,
        0,-1
    );
    buildingNode->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    buildingNode->setVisible(false);
}
void buildings::doBuildEnd(){
    if(!buildingMode)
        return;
    buildingMode=false;
    buildingNode->remove();
    buildingNode=NULL;
    buildingConfig=NULL;
    buildingType=0;
}
void buildings::doBuildUpdate(const irr::core::line3d<irr::f32> & ray){
    if(!buildingMode)
        return;
    auto nr=ray.end-ray.start;
    cameraRay.start=ray.start;
    cameraRay.end=cameraRay.start+nr.normalize()*5.0f;
    
    irr::core::line3d<irr::f32> buildingNormal;
    if(getNormalByRay(cameraRay,buildingNormal)){
        transformByNormal(buildingNode,buildingNormal);
        buildingNode->setVisible(true);
        allowBuild=true;
    }else{
        buildingNode->setVisible(false);
        allowBuild=false;
    }
}
void buildings::doBuildApplay(){
    if(!buildingMode || !allowBuild)
        return;
    std::list<building*> l;
    std::list<std::string> sislist;
    if(collisionWithBuildings(buildingNode,buildingType,l)){
        if(l.size()>4)
            return;
        else{
            if(collisionWithTerrain(buildingNode,buildingType))
                buildOnFloor(
                    buildingType,
                    buildingNode->getPosition(),
                    buildingNode->getRotation()
                );
            else{
                for(auto it:l){
                    if(it && it->type==building::BUILDING_ITEM){
                        auto pitem=(remoteGraph::item*)it->data;
                        sislist.push_back(pitem->uuid);
                    }
                }
                if(sislist.empty())
                    return;
                buildOn(
                    buildingType,
                    buildingNode->getPosition(),
                    buildingNode->getRotation(),
                    sislist
                );
            }
        }
    }else
    if(collisionWithTerrain(buildingNode,buildingType)){
        buildOnFloor(
            buildingType,
            buildingNode->getPosition(),
            buildingNode->getRotation()
        );
    }
}
bool buildings::collisionWithObject(irr::scene::IMeshSceneNode * n){
    return false;
}

void buildings::attackByRay(const irr::core::line3d<irr::f32> & ray){
    auto p=selectByRay(ray);
    if(p){
        auto pitem=(remoteGraph::item*)p->data;
        attackNode(pitem->uuid,buildingDamage);
    }
}

buildings::building * buildings::selectByRay(const irr::core::line3d<irr::f32> & ray){
    irr::core::line3d<irr::f32> l;
    return collisionWithBuildings(ray,l);
}

}//namespace smoothly