#include "building.h"

namespace smoothly{

static int getBuildingNumInGhost(btPairCachingGhostObject * ghost){
    int count=0;
    int num = ghost->getNumOverlappingObjects();
    for (int i = 0; i < num; i++){
        btCollisionObject *btco = ghost->getOverlappingObject(i);
        auto info = (building::bodyInfo*)(btco->getUserPointer());
        if(info){
            if(info->type==building::BODY_BUILDING)
                ++count;
        }
    }
    return count;
}

void building::buildingBody::initCollTable(bool removeMode){
    auto box = node[0]->getBoundingBox();
    auto deltaV = box.MaxEdge-box.MinEdge;

    int nx = floor(deltaV.X/2);
    int ny = floor(deltaV.Y/2);
    int nz = floor(deltaV.Z/2);

    int bx = floor(box.MinEdge.X/2);
    int by = floor(box.MinEdge.Y/2);
    int bz = floor(box.MinEdge.Z/2);

    ghostSearch gs(btVector3(1,1,1),parent);
    btTransform t;

    for(int i = 0;i<nx;++i){
        for(int j=0;j<ny;++j){
            for(int k=0;k<nz;++k){

                int rx = bx+i;
                int ry = by+j;
                int rz = bz+k;

                blockPosition rv(rx,ry,rz);

                auto it = parent->collTable.find(rv);
                if(removeMode){
                    if(it!=parent->collTable.end()){
                        btVector3 position(//真实位置
                                           rx*2+1,
                                           ry*2+1,
                                           rz*2+1);
                        //开始碰撞检测
                        t.setOrigin(position);
                        gs.ghost.setWorldTransform(t);

                        parent->dynamicsWorld->addCollisionObject(&gs.ghost);//加入世界
                        if(getBuildingNumInGhost(&gs.ghost)==0){
                            parent->collTable.erase(it);
                        }
                        parent->dynamicsWorld->removeCollisionObject(&gs.ghost);
                    }
                }else{
                    if(it==parent->collTable.end() || it->second==false){
                        btVector3 position(//真实位置
                                           rx*2+1,
                                           ry*2+1,
                                           rz*2+1);
                        //开始碰撞检测
                        t.setOrigin(position);
                        gs.ghost.setWorldTransform(t);

                        parent->dynamicsWorld->addCollisionObject(&gs.ghost);//加入世界
                        if(getBuildingNumInGhost(&gs.ghost)!=0)
                            parent->collTable[rv] = true;
                        parent->dynamicsWorld->removeCollisionObject(&gs.ghost);
                    }
                }
            }
        }
    }
}

void building::buildingBody::buildAffectArea(std::queue<blockPosition> & t){
    auto box = node[0]->getBoundingBox();
    auto deltaV = box.MaxEdge-box.MinEdge;

    int nx = floor(deltaV.X/2);
    int ny = floor(deltaV.Y/2);
    int nz = floor(deltaV.Z/2);

    int bx = floor(box.MinEdge.X/2);
    int by = floor(box.MinEdge.Y/2);
    int bz = floor(box.MinEdge.Z/2);


    for(int i = 0;i<nx;++i){
        for(int j=0;j<ny;++j){
            for(int k=0;k<nz;++k){

                int rx = bx+i;
                int ry = by+j;
                int rz = bz+k;

                t.push(blockPosition(rx,ry,rz));

            }
        }
    }
}
void building::processCheckingRemove(){
    if(checkingRemoveList.empty())
        return;

    ghostSearch gs(btVector3(1,1,1),this);
    btTransform t;

    clock_t starts,ends;
    starts=clock();
    while(!checkingRemoveList.empty()){
        auto & rv = checkingRemoveList.front();
        auto it = collTable.find(rv);
        if(it!=collTable.end()){
            btVector3 position(//真实位置
                               rv.x*2+1,
                               rv.y*2+1,
                               rv.z*2+1);
            //开始碰撞检测
            t.setOrigin(position);
            gs.ghost.setWorldTransform(t);

            dynamicsWorld->addCollisionObject(&gs.ghost);//加入世界
            if(getBuildingNumInGhost(&gs.ghost)==0){//没发生碰撞
                collTable.erase(it);
            }
            dynamicsWorld->removeCollisionObject(&gs.ghost);
        }
        checkingRemoveList.pop();

        ends = clock();
        auto dtm = (ends-starts)/(CLOCKS_PER_SEC/1000);
        if(dtm>2)
            break;
    }
}

}
