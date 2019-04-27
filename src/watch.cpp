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
void watch::setUserPosition(const irr::core::vector3df & position,const RakNet::SystemAddress & addr){
    HBB::vec3 from  (position.X-viewRange , -2000 , position.Z-viewRange);
    HBB::vec3 to    (position.X+viewRange ,  2000 , position.Z+viewRange);
    setListener(from,to,addr);
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
void watch::boardcastSubsCreate(
    const std::string & subsuuid,
    long id , 
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& impulse,
    const btVector3& rel_pos,
    const std::string & useruuid,
    const std::string & config
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_CREATE);
    
    RakNet::RakString u,conf;
    
    bs.Write((int64_t)id);
    
    u=subsuuid.c_str();
    bs.Write(u);
    
    u=useruuid.c_str();
    bs.Write(u);
    
    bs.WriteVector(position.X , position.Y , position.Z);
    bs.WriteVector(rotation.X , rotation.Y , rotation.Z);
    bs.WriteVector(impulse.getX(),impulse.getY(),impulse.getZ());
    bs.WriteVector(rel_pos.getX(),rel_pos.getY(),rel_pos.getZ());
    
    conf=config.c_str();
    bs.Write(conf);
    
    boardcastByPoint(HBB::vec3(position.X,0,position.Z),&bs);
}
void watch::boardcastSubsCreate(
    long id , 
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& impulse,
    const btVector3& rel_pos,
    const std::string & useruuid,
    const std::string & config,
    const RakNet::SystemAddress & ext
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_CREATE_B);
    
    RakNet::RakString u,conf;
    
    bs.Write((int64_t)id);
    
    u=useruuid.c_str();
    bs.Write(u);
    
    bs.WriteVector(position.X , position.Y , position.Z);
    bs.WriteVector(rotation.X , rotation.Y , rotation.Z);
    bs.WriteVector(impulse.getX(),impulse.getY(),impulse.getZ());
    bs.WriteVector(rel_pos.getX(),rel_pos.getY(),rel_pos.getZ());
    
    conf=config.c_str();
    bs.Write(conf);
    
    struct tmpc{
        RakNet::SystemAddress ext;
        RakNet::BitStream * bs;
    }arg;
    
    arg.ext=ext;
    arg.bs=&bs;
    
    eventHBB.fetchByPoint(HBB::vec3(position.X,0,position.Z),[](HBB::AABB * box , void * argp){
        auto arg=(tmpc*)argp;
        auto lst=(listener*)box->data;
        
        if(arg->ext!=lst->address)
            lst->parent->sendMessage(arg->bs,lst->address);
    },&arg);
}
void watch::boardcastSubsStatus(
    const std::string & subsuuid,
    long id , 
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& lin_vel,
    const btVector3& ang_vel,
    int status,
    int hp,
    const std::string & useruuid
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_STATUS);
    
    RakNet::RakString u;
    RakNet::RakString conf="[LOADING]";
    
    bs.Write((int64_t)id);
    
    u=subsuuid.c_str();
    bs.Write(u);
    
    u=useruuid.c_str();
    bs.Write(u);
    
    bs.WriteVector(position.X , position.Y , position.Z);
    bs.WriteVector(rotation.X , rotation.Y , rotation.Z);
    bs.WriteVector(lin_vel.getX(),lin_vel.getY(),lin_vel.getZ());
    bs.WriteVector(ang_vel.getX(),ang_vel.getY(),ang_vel.getZ());
    
    bs.Write((int32_t)status);
    bs.Write((int32_t)hp);
    
    bs.Write(conf);
    
    boardcastByPoint(HBB::vec3(position.X,0,position.Z),&bs);
}

void watch::boardcastSubsRemove(const std::string & subsuuid,const irr::core::vector3df & position){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_REMOVE);
    
    RakNet::RakString u;
    u=subsuuid.c_str();
    bs.Write(u);
    
    boardcastByPoint(HBB::vec3(position.X,0,position.Z),&bs);
}
void watch::boardcastSubsAttack(const std::string & subsuuid,const irr::core::vector3df & position,int hp,int delta){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_ATTACK);
    
    RakNet::RakString u;
    u=subsuuid.c_str();
    bs.Write(u);
    
    bs.Write((int32_t)hp);
    bs.Write((int32_t)delta);
    
    boardcastByPoint(HBB::vec3(position.X,0,position.Z),&bs);
}
void watch::boardcastTeleport(
    const std::string & subsuuid,
    const irr::core::vector3df & position
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_TELEPORT);
    
    RakNet::RakString u;
    u=subsuuid.c_str();
    bs.Write(u);
    
    bs.WriteVector(position.X , position.Y , position.Z);
    
    boardcastByPoint(HBB::vec3(position.X,0,position.Z),&bs);
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