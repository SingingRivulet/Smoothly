#include "watch.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<watch::listener> lpool;

void watch::setListener(
    const HBB::vec3 & from,
    const HBB::vec3 & to,
    const RakNet::SystemAddress & addr
){
    listener * p;
    auto it=listeners.find(addr);
    if(it==listeners.end()){
        p=createListener();
        listeners[addr]=p;
    }else{
        p=it->second;
        p->box->autodrop();
    }
    p->address=addr;
    p->box=eventHBB.add(from,to,p);
    //printf("setListener:(%f,%f,%f)\n",from.X,from.Y,from.Z);
    //printf("to:(%f,%f,%f)\n",to.X,to.Y,to.Z);
}
void watch::delListener(const RakNet::SystemAddress & addr){
    auto it=listeners.find(addr);
    if(it==listeners.end())
        return;
    it->second->box->autodrop();
    delListener(it->second);
}
void watch::onCreateNode(
    const std::string & uuid,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link,
    long type,
    int hp,
    int x,int y
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    
    bs.Write((RakNet::MessageID)B_CREATE);
    
    RakNet::RakString u;
    u=uuid.c_str();
    bs.Write(u);
    bs.WriteVector(position.X , position.Y , position.Z);
    bs.WriteVector(rotation.X , rotation.Y , rotation.Z);
    bs.Write((int64_t)type);
    bs.Write((int32_t)hp);
    bs.Write((int32_t)(link.size()));
    for(auto it:link){
        u=it.c_str();
        bs.Write(u);
    }
    boardcastByPoint(position,&bs);
}
void watch::onDestroyNode(const std::string & uuid,int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    
    bs.Write((RakNet::MessageID)B_DESTROY);
    
    RakNet::RakString u;
    u=uuid.c_str();
    
    bs.Write(u);
    
    boardcastByPoint(HBB::vec3(x*32+16,0,y*32+16),&bs);
}
void watch::onAttackNode(const std::string & uuid,int lhp,int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    
    bs.Write((RakNet::MessageID)B_ATTACK);
    
    RakNet::RakString u;
    u=uuid.c_str();
    
    bs.Write(u);
    bs.Write((int32_t)lhp);
    
    boardcastByPoint(HBB::vec3(x*32+16,0,y*32+16),&bs);
}
void watch::onRemoveTerrain(int x,int y,long itemid,int mapid){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    
    bs.Write((RakNet::MessageID)T_SEND_ONE);
    
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    bs.Write((int64_t)itemid);
    bs.Write((int32_t)mapid);
    
    boardcastByPoint(HBB::vec3(x*32+16,0,y*32+16),&bs);
}

void watch::boardcastByPoint(const HBB::vec3 & point,RakNet::BitStream * bs){
    //printf("boardcast:(%f,%f,%f)\n",point.X,point.Y,point.Z);
    eventHBB.fetchByPoint(point,[](HBB::AABB * box , void * argp){
        auto bs=(RakNet::BitStream*)argp;
        auto lst=(listener*)box->data;
        lst->parent->sendMessage(bs,lst->address);
        //printf("boardcast\n");
    },bs);
}

watch::listener * watch::createListener(){
    auto pool=(lpool*)listenerPool;
    auto p=pool->get();
    p->parent=this;
    p->box=NULL;
    return p;
}
void watch::delListener(listener * p){
    auto pool=(lpool*)listenerPool;
    pool->del(p);
}
void watch::lstPoolInit(){
    listenerPool=new lpool;
}
void watch::lstPoolFree(){
    delete (lpool*)listenerPool;
}

}