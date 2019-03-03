#include "clientNetwork.h"
#include <string.h>
namespace smoothly{
void clientNetwork::init(const char * addr,short port){
    connection=RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd;
    connection->Startup(1,&sd,1);
    this->addr=addr;
    this->port=port;
    connect();
}
void clientNetwork::connect(){
    connection->Connect(addr.c_str(),port,0,0);
}
void clientNetwork::sendMessage(RakNet::BitStream * data){
    connection->Send( data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );  
}
void clientNetwork::sendMessageU(RakNet::BitStream * data){
    connection->Send( data, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );  
}
void clientNetwork::shutdown(){
    connection->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(connection);
    terrain::destroy();
    remoteGraph::destroy();
}
void clientNetwork::recv(){
    if(!connection)
        return;
    auto pk=connection->Receive();
    if(pk)
        onRecvMessage(pk);
}
void clientNetwork::onRecvMessage(RakNet::Packet * data){
    switch(data->data[0]){
        case MESSAGE_GAME:
            //printf("recv message\n");
            switch(data->data[1]){
                case M_UPDATE_BUILDING:
                    onMessageUpdateBuilding(data);
                break;
                case M_UPDATE_TERRAIN:
                    onMessageUpdateTerrain(data);
                break;
                case M_UPDATE_OBJECT:
                
                break;
            }
        break;
    }
}
void clientNetwork::onMessageUpdateBuilding(RakNet::Packet * data){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case B_ATTACK:
            onMessageUpdateBuildingAttack(&bs);
        break;
        case B_CREATE:
            onMessageUpdateBuildingCreate(&bs);
        break;
        case B_DESTROY:
            onMessageUpdateBuildingDestroy(&bs);
        break;
        case B_GENER:
            onMessageUpdateBuildingGen(&bs);
        break;
    }
}
void clientNetwork::onMessageUpdateTerrain(RakNet::Packet * data){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case T_SEND_ONE:
            onMessageUpdateTerrainRemove(&bs);
        break;
        case T_SEND_TABLE:
            onMessageUpdateTerrainGetRMTable(&bs);
        break;
        case T_APPLAY:
            onMessageUpdateTerrainRMTApply(&bs);
        break;
    }
}
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
    printf("createBuilding:(%f,%f,%f)\n",position.X,position.Y,position.Z);
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
void clientNetwork::onMessageUpdateTerrainRMTApply(RakNet::BitStream * data){
    int32_t x,y;
    if(!data->Read(x))return;
    if(!data->Read(y))return;
    removeTableApplay(x,y);
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
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    bs.Write((RakNet::MessageID)B_ATTACK_UPLOAD);
    bs.Write(uuid.c_str());
    bs.Write((int32_t)hurt);
    sendMessage(&bs);
}
void clientNetwork::downloadBuilding(int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    bs.Write((RakNet::MessageID)B_DOWNLOAD_CHUNK);
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    sendMessageU(&bs);
}
remoteGraph::item * clientNetwork::downloadBuilding(const std::string & uuid){
    
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
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    bs.Write((RakNet::MessageID)B_CREATE_UPLOAD);
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
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    bs.Write((RakNet::MessageID)B_CREATE_UPLOAD);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.Write((int64_t)type);
    bs.Write((int32_t)0);
    //printf("buildOnFloor:(%f,%f,%f)\n",position.X,position.Y,position.Z);
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
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    bs.Write((RakNet::MessageID)B_CREATE_UPLOAD);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.Write((int64_t)type);
    int32_t len=link.size();
    bs.Write(len);
    for(auto it:link){
        bs.Write(it.c_str());
    }
    //printf("buildOn:(%f,%f,%f)\n",position.X,position.Y,position.Z);
    sendMessage(&bs);
}

void clientNetwork::requestRemoveItem(const mapid & mid){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    bs.Write((RakNet::MessageID)T_REMOVE);
    bs.Write((int32_t)mid.x);
    bs.Write((int32_t)mid.y);
    bs.Write((int64_t)mid.itemId);
    bs.Write((int32_t)mid.mapId);
    sendMessageU(&bs);
}
void clientNetwork::requestUpdateTerrain(int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    bs.Write((RakNet::MessageID)T_DOWNLOAD);
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    sendMessageU(&bs);
}
void clientNetwork::setUserPosition(const irr::core::vector3df & p){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_USER);
    bs.Write((RakNet::MessageID)U_SET_POSITION);
    bs.WriteVector(p.X , p.Y , p.Z);
    sendMessage(&bs);
}
void clientNetwork::setUserRotation(const irr::core::vector3df & r){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)U_SET_ROTATION);
    bs.WriteVector(r.X , r.Y , r.Z);
    sendMessage(&bs);
}

}