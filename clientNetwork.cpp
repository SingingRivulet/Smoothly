#include "clientNetwork.h"
#include <string.h>
namespace smoothly{

void clientNetwork::onMessageUpdateBuildingGen(RakNet::BitStream * data){
    irr::core::vector3df position;
    irr::core::vector3df rotation;
    RakNet::RakString uuid;
    int64_t type;
    int32_t hp;
    int32_t linkNum;
    RakNet::RakString linkuuid;
    std::set<std::string> link,linkTo;
    
    data->Read(uuid);
    data->ReadVector(position.X , position.Y , position.Z);
    data->ReadVector(rotation.X , rotation.Y , rotation.Z);
    data->Read(type);
    if(!data->Read(hp))
        return;
    if(data->Read(linkNum)){
        for(int i=0;i<linkNum;i++){
            if(!data->Read(linkuuid))
                break;
            const char * str=linkuuid.C_String();
            if(strlen(str)==0)
                break;
            link.insert(str);
        }
    }
    
    if(data->Read(linkNum)){
        for(int i=0;i<linkNum;i++){
            if(!data->Read(linkuuid))
                break;
            const char * str=linkuuid.C_String();
            if(strlen(str)==0)
                break;
            linkTo.insert(str);
        }
    }
    
    onMessageGen(uuid.C_String(),position,rotation,link,linkTo,hp,type);
}
void clientNetwork::onMessageUpdateBuildingAttack(RakNet::BitStream * data){
    RakNet::RakString uuid;
    int32_t hp;
    data->Read(uuid);
    if(!data->Read(hp))
        return;
    setBuildingHP(uuid.C_String(),hp);
}
void clientNetwork::onMessageUpdateBuildingCreate(RakNet::BitStream * data){
    irr::core::vector3df position;
    irr::core::vector3df rotation;
    RakNet::RakString uuid;
    int64_t type;
    int32_t hp;
    int32_t linkNum;
    RakNet::RakString linkuuid;
    std::set<std::string> link;
    
    data->Read(uuid);
    data->ReadVector(position.X , position.Y , position.Z);
    data->ReadVector(rotation.X , rotation.Y , rotation.Z);
    data->Read(type);
    if(!data->Read(hp))
        return;
    if(data->Read(linkNum)){
        for(int i=0;i<linkNum;i++){
            if(!data->Read(linkuuid))
                break;
            const char * str=linkuuid.C_String();
            if(strlen(str)==0)
                break;
            link.insert(str);
        }
    }
    
    onMessageCreate(uuid.C_String(),position,rotation,link,hp,type);
}
void clientNetwork::onMessageUpdateBuildingDestroy(RakNet::BitStream * data){
    RakNet::RakString uuid;
    if(!data->Read(uuid))
        return;
    onMessageDestroy(uuid.C_String());
}
void clientNetwork::onMessageUpdateTerrainGetRMTable(RakNet::BitStream * data){
    int32_t num,x,y;
    int64_t type;
    int32_t mapid;
    std::list<std::pair<long,int> > t;
    data->Read(x);
    data->Read(y);
    if(!data->Read(num))
        return;
    for(int i=0;i<num;i++){
        if(!data->Read(type))   break;
        if(!data->Read(mapid))  break;
        t.push_back(std::pair<long,int>(type,mapid));
    }
    setRemoveTable(x,y,t);
}
void clientNetwork::onMessageUpdateTerrainRemove(RakNet::BitStream * data){
    mapid mid;
    int32_t x,y,mapId;
    int64_t itemId;
    data->Read(mid);
    if(!data->Read(x))      return;
    if(!data->Read(y))      return;
    if(!data->Read(itemId)) return;
    if(!data->Read(mapId))  return;
    mid.x=x;
    mid.y=y;
    mid.itemId=itemId;
    mid.mapId=mapId;
    terrain::remove(mid);
}

void clientNetwork::uploadAttack(const std::string & uuid , int hurt){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_ATTACK_UPLOAD);
    bs.Write(uuid.c_str());
    bs.Write((int32_t)hurt);
    sendMessage(&bs);
}
void clientNetwork::downloadBuilding(int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_DOWNLOAD_CHUNK);
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    sendMessage(&bs);
}
remoteGraph::item * clientNetwork::downloadBuilding(const std::string & uuid){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_DOWNLOAD_UUID);
    bs.Write(uuid.c_str());
    sendMessage(&bs);
}
void clientNetwork::addNode(//添加节点
    const irr::core::vector3df & position,//位置
    const irr::core::vector3df & rotation,//无用，给子类
    const std::set<std::string> & link,//表示修建在什么节点上
    int hp,//服务器端没定义hp，所以这是无用变量
    long type
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_CREATE_UPLOAD);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.Write((int64_t)type);
    int32_t len=link.size();
    bs.Write(len);
    for(auto it:link){
        bs.Write(it.c_str());
    }
    sendMessage(&bs);
}
void clientNetwork::buildOnFloor(
    long type,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_CREATE_UPLOAD);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.Write((int64_t)type);
    bs.Write((int32_t)0);
    sendMessage(&bs);
}
void clientNetwork::buildOn(
    long type,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::list<std::string> & link
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((char)B_CREATE_UPLOAD);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.Write((int64_t)type);
    int32_t len=link.size();
    bs.Write(len);
    for(auto it:link){
        bs.Write(it.c_str());
    }
    sendMessage(&bs);
}

void clientNetwork::requestRemoveItem(const mapid & mid){
    
}
void clientNetwork::requestUpdateTerrain(int x,int y){
    
}

}