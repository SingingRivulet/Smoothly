#include "remoteGraph.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<remoteGraph::item> bpool;
typedef mempool<remoteGraph::chunk> chpool;

void remoteGraph::item::destroy(){
    destroy(parent->maxdeep);
}

void remoteGraph::item::destroy(int deep){
    if(destroyed)
        return;
    if(deep=0)
        return;
    destroyed=true;
    parent->removelist.insert(this);
    
    if(!link.empty()){
        for(auto it:link){
            auto p=parent->seekNode(it,false);
            if(p)
                p->remove(this);
        }
    }
    
    if(!linkTo.empty()){
        for(auto it:linkTo){
            auto p=parent->seekNode(it,false);
            if(p)
                p->attack(this,deep-1);
        }
    }
}

void remoteGraph::item::attack(remoteGraph::item * p,int deep){
    if(deep=0)
        return;
    this->link.erase(p->uuid);
    if(destroyed)
        return;
    if(link.empty())
        this->destroy(deep);
}

void remoteGraph::item::remove(remoteGraph::item * p){
    if(destroyed)
        return;
    this->linkTo.erase(p->uuid);
}

remoteGraph::item * remoteGraph::genNode(
    const irr::core::vector3df & position,//位置
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link,//表示修建在什么节点上
    const std::set<std::string> & linkTo,
    const std::string & uuid,
    int hp,
    long type,
    bool createmode
){
    auto p=seekNode(uuid,false);
    if(p==NULL){
        p=createNode();
        p->uuid=uuid;
        setChunk(p , position);
    }else
        return NULL;//bug:重复调用onGenBuilding导致内存泄露
    
    p->position=position;
    p->rotation=rotation;
    p->hp=hp;
    p->type=type;
    
    if(link.empty()){
        p->isRoot=true;
    }else{
        p->isRoot=false;
        for(auto it:link){
            p->link.insert(it);
            //auto par=seekNode(it,false);
            //if(par){
            //    par->linkTo.insert(p->uuid);
            //}
        }
    }
    p->linkTo=linkTo;
    if(createmode)
        createlist.insert(p);
    else
        genlist.insert(p);
    return p;
}

void remoteGraph::removeNode(
    const std::string & uuid
){
    auto p=seekNode(uuid,false);
    if(p){
        p->destroy();
        
    }
}

bool remoteGraph::inRange(remoteGraph::item * p){
    return p->inChunk!=NULL;
}

void remoteGraph::removeApplay(){
    for(auto it:removelist){
        onDestroyBuilding(it);
        removeFromChunk(it);
        delNode(it);
    }
    removelist.clear();
}

void remoteGraph::createListFilter(){
    for(auto it:genlist){
        if(!inRange(it)){
            removeFromChunk(it);
            delNode(it);
        }else{
            onGenBuilding(it);
        }
    }
    for(auto it:createlist){
        if(!inRange(it)){
            removeFromChunk(it);
            delNode(it);
        }else{
            this->onCreateBuilding(it);
        }
    }
    createlist.clear();
    genlist.clear();
}

void remoteGraph::onMessageGen(
    const std::string & uuid,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link,
    const std::set<std::string> & linkTo,
    int hp,
    long type
){
    genNode(position,rotation,link,linkTo,uuid,hp,type,false);
}

void remoteGraph::onMessageCreate(
    const std::string & uuid,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link,
    int hp,
    long type
){
    std::set<std::string> tmp;
    genNode(position,rotation,link,tmp,uuid,hp,type,true);
}

void remoteGraph::onMessageDestroy(const std::string & uuid){
    removeNode(uuid);
}

void remoteGraph::setBuildingHP(
    const std::string & uuid,
    int hp
){
    auto p=seekNode(uuid,false);
    if(p){
        //uploadAttack(uuid,hurt);
        p->hp=hp;
        if(hp<=0)
            p->destroy();
    }
}

void remoteGraph::attackNode(
    const std::string & uuid,
    int hurt
){
    uploadAttack(uuid,hurt);
}

void remoteGraph::onMessageAttack(const std::string & uuid , int hurt){
    auto p=seekNode(uuid,false);
    if(p){
        p->hp-=hurt;
        if((p->hp)<=0)
            p->destroy();
    }
}

void remoteGraph::setChunk(remoteGraph::item * p,const irr::core::vector3df & position){
    p->position=position;
    int x,y;
    getChunkPosition(position.X , position.Z , x , y);
    auto it=chunks.find(ipair(x,y));
    if(it!=chunks.end()){
        p->inChunk=it->second;
        it->second->items.insert(p);
        auto it2=items.find(p->uuid);
        if(it2!=items.end()){
            //debug:uuid has existed
            //free old value
            removeFromChunk(it2->second);
            delNode(it2->second);
        }
        items[p->uuid]=p;
    }else{
        p->inChunk=NULL;
    }
}

void remoteGraph::createChunk(int x,int y){
    auto it=chunks.find(ipair(x,y));
    if(it!=chunks.end())
        return;
    auto p=createChunk();
    p->x=x;
    p->y=y;
    chunks[ipair(x,y)]=p;
}

void remoteGraph::removeFromChunk(remoteGraph::item * p){
    if(p->inChunk)
        p->inChunk->items.erase(p);
    if(p->parent)
        p->parent->items.erase(p->uuid);
}

void remoteGraph::clearNodes(){
    for(auto it:items){
        //printf("free:%s\n",it.second->uuid.c_str());
        onFreeBuilding(it.second);
        //removeFromChunk(it.second);
        //bug:removeFromChunk导致迭代器失效无法遍历所有item，导致内存泄漏
        if(it.second->inChunk)
            it.second->inChunk->items.erase(it.second);
        delNode(it.second);
    }
    for(auto it:chunks){
        it.second->items.clear();
        delChunk(it.second);
    }
    chunks.clear();
    items.clear();
}

void remoteGraph::updateBuildingChunks(int x,int y,int range){
    int fx=x-range;
    int tx=x+range;
    int fy=y-range;
    int ty=y+range;
    std::map<ipair,chunk*> rm=chunks;
    for(int i=fx;i<=tx;i++){
        for(int j=fy;j<=ty;j++){
            createChunk(i,j);
            downloadBuilding(i,j);
            rm.erase(ipair(i,j));
        }
    }
    for(auto it:rm){
        delChunk(it.second);
        chunks.erase(it.first);
    }
}

void remoteGraph::poolInit(){
    buildingPool=new bpool;
    buildingChunkPool=new chpool;
}

void remoteGraph::poolFree(){
    if(buildingPool)delete (bpool*)buildingPool;
    if(buildingChunkPool)delete (chpool*)buildingChunkPool;
}


remoteGraph::chunk * remoteGraph::createChunk(){
    auto p=((chpool*)buildingChunkPool)->get();
    p->parent=this;
    p->construct();
    return p;
}

void remoteGraph::delChunk(remoteGraph::chunk *p){
    p->destruct();
    ((chpool*)buildingChunkPool)->del(p);
}

remoteGraph::item * remoteGraph::createNode(){
    auto p=((bpool*)buildingPool)->get();
    p->construct();
    p->parent=this;
    
    return p;
}

void remoteGraph::chunk::clear(){
    for(auto it:items){
        if(it){
            parent->onFreeBuilding(it);
            parent->removeFromChunk(it);
            parent->delNode(it);
        }
    }
    items.clear();
}

void remoteGraph::delNode(remoteGraph::item * p){
    p->destruct();
    ((bpool*)buildingPool)->del(p);
}

remoteGraph::item * remoteGraph::seekNode(const std::string & uuid,bool download){
    auto it=items.find(uuid);
    if(it==items.end()){
        if(download)
            return this->downloadBuilding(uuid);
        else
            return NULL;
    }else
        return it->second;
}

remoteGraph::remoteGraph(){
    removelist.clear();
    createlist.clear();
    genlist.clear();
    maxdeep=64;
    items.clear();
    chunks.clear();
    poolInit();
}

void remoteGraph::destroy(){
    buildingApplay();
    clearNodes();
    poolFree();
}

remoteGraph::~remoteGraph(){
}

}//namespace smoothly