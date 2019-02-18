#include "building.h"
namespace smoothly{


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

bool buildings::collisionWithBuildings(const irr::core::line3d<irr::f32> & ray,irr::core::line3d<irr::f32> &normal){
    struct cwbtmp{
        float lenSq;
        irr::core::vector3df position;
        irr::core::triangle3df triangle;
        bool havevalue;
        buildings * self;
        irr::core::line3d<irr::f32> ray;
    }arg;
    arg.havevalue=false;
    arg.self=this;
    arg.ray=ray;
    
    buildingHBB.rayTest(ray,[](HBB::AABB * box , void * argp){
        
        auto bd=(building*)(box->data);
        if(bd->type==building::BUILDING_TERRAIN)
            return;
        
        auto arg=(cwbtmp*)argp;
        auto self=arg->self;
        
        //use selector to get collision point
        auto pitem=(remoteGraph::item*)bd->data;
        
        auto it=self->m->buildings.find(pitem->type);
        if(it==self->m->buildings.end())
            return;
        
        auto selector=it->second->selector;
        
        if(pitem->node==NULL)
            return;
        
        irr::core::triangle3df t;
        int num=0;
        auto mat=pitem->node->getAbsoluteTransformation();
        selector->getTriangles(&t,1,num,arg->ray,&mat);
        
        if(num<=0)
            return;
        
        irr::core::vector3df position;
        intersectionTriangle(arg->ray,t,position);
        
        auto sq=getLenSq(arg->ray.start , position);
        if(arg->havevalue==false || sq < arg->lenSq){
            arg->havevalue=true;
            arg->lenSq=sq;
            arg->triangle=t;
            arg->position=position;
        }
        
    },&arg);
    if(arg.havevalue==false)
        return false;
    //get normal
    irr::core::vector3df n;
    getTriangleNormal(arg.triangle , ray.start , n);
    normal.start=arg.position;
    normal.end=arg.position+n;
    return true;
}

void buildings::onGenBuilding(remoteGraph::item * i){
    auto it=m->buildings.find(i->type);
    if(it==m->buildings.end()){
        i->node=NULL;
        i->rigidBody=NULL;
        return;
    }
    i->node=scene->addMeshSceneNode(
        it->second->mesh,
        0,-1,
        i->position,
        i->rotation
    );
    
    i->node->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    
    irr::core::aabbox3d<irr::f32> box=it->second->BB;
    i->node->getAbsoluteTransformation().transformBoxEx(box);
    
    i->rigidBody=makeBulletMeshFromIrrlichtNode(i->node);
    i->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
    dynamicsWorld->addRigidBody(i->rigidBody);
    
    auto p=createCBuild();
    p->type=building::BUILDING_ITEM;
    p->data=i;
    
    allBuildings[i]=p;
    
    p->node=buildingHBB.add(box.MinEdge , box.MaxEdge , p);
}

void buildings::onFreeBuilding(remoteGraph::item * i){
    if(i->node)
        i->node->remove();
    if(i->rigidBody){
        dynamicsWorld->removeRigidBody(i->rigidBody);
        delete i->rigidBody;
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
    auto v1=irr::core::vector3df(0,1,0);
    auto vn=normal.end-normal.start;
    vn.normalize();
    
    float dot=v1.dotProduct(vn);
    float theta=acos(dot);//v1，vn是单位向量，所以不用除
    
    theta=theta/2;
    auto ct=cos(theta);
    auto st=sin(theta);
    
    auto u=v1*vn;
    u.normalize();
    
    irr::core::vector3df eu;
    irr::core::quaternion qu(ct , st*u.X , st*u.Y , st*u.Z);
    qu.toEuler(eu);
    
    n->setPosition(normal.start);
    n->setRotation(eu);
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
    }else
        buildingNode->setVisible(false);
}
void buildings::doBuildApplay(){
    if(!buildingMode)
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

}//namespace smoothly