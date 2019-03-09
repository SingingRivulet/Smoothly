#include "mods.h"
namespace smoothly{
void mods::init(const char * path){
    defaultBuildingList[0]=1;
    defaultBuildingList[1]=2;
    defaultBuildingList[2]=3;
    defaultBuildingList[3]=-1;
    defaultBuildingList[4]=-1;
    defaultBuildingList[5]=-1;
    defaultBuildingList[6]=-1;
}
void mods::loadMesh(){
    building * b;
    auto creator=scene->getGeometryCreator();
    
    b           = new building;
    b->mesh     = creator->createCubeMesh(irr::core::vector3df(5.0f , 0.1f , 5.0f));
    b->BB       = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[1]= b;
    
    b       = new building;
    b->mesh = creator->createCubeMesh(irr::core::vector3df(5.0f , 5.0f , 0.1f));
    b->BB   = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[2]=b;
    
    b       = new building;
    b->mesh = creator->createCubeMesh(irr::core::vector3df(1.0f , 5.0f , 1.0f));
    b->BB   = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[3]=b;
}
void mods::destroy(){
    for(auto it:items){
        it.second->mesh->drop();
        delete it.second;
    }
    items.clear();
    for(auto it:buildings){
        it.second->mesh->drop();
        delete it.second;
        if(it.second->bodyMesh) delete it.second->bodyMesh;
        if(it.second->bodyShape)delete it.second->bodyShape;
    }
    buildings.clear();
}

}