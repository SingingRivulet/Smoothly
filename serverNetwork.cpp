#include "serverNetwork.h"
namespace smoothly{
void serverNetwork::removeTableApply(const RakNet::SystemAddress & to,int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    
    bs.Write((RakNet::MessageID)T_APPLY);
    
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    
    sendMessage(&bs,to);
}

void serverNetwork::sendRemoveTable(
    const RakNet::SystemAddress & to,
    const std::list<std::pair<long,int> > & l,
    int x,int y
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    
    bs.Write((RakNet::MessageID)T_SEND_TABLE);
    
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    
    bs.Write((int32_t)l.size());
    for(auto it:l){
        bs.Write((int64_t)it.first);
        bs.Write((int32_t)it.second);
    }
    
    sendMessage(&bs,to);
}
void serverNetwork::onSendNode(
    const std::string & uuid,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link,
    const std::set<std::string> & linkTo,
    long type,
    int hp,
    int x,int y,
    const RakNet::SystemAddress & to
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_BUILDING);
    
    bs.Write((RakNet::MessageID)B_GENER);
    
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
    
    bs.Write((int32_t)(linkTo.size()));
    for(auto it:linkTo){
        u=it.c_str();
        bs.Write(u);
    }
    
    sendMessage(&bs,to);
}

serverNetwork::serverNetwork(const char * pathGra,const char * pathRMT,const char * modpath,short port,int maxcl){
    lstPoolInit();
    scriptInit(modpath);
    removeTable::init(pathGra);
    graphServer::init(pathRMT);
    connection=RakNet::RakPeerInterface::GetInstance();
    if(connection==NULL){
        printf("RakNet::RakPeerInterface::GetInstance() Error!\n");
        return;
    }
    RakNet::SocketDescriptor desc(port, 0);
    connection->Startup( maxcl, &desc, 1 );
    connection->SetMaximumIncomingConnections( maxcl );
}
serverNetwork::~serverNetwork(){
    if(connection){
        RakNet::RakPeerInterface::DestroyInstance(connection);
    }
    removeTable::destroy();
    graphServer::destroy();
    scriptDestroy();
    lstPoolFree();
}
void serverNetwork::recv(){
    if(connection){
        RakNet::Packet* pPacket;
        for(
            pPacket = connection->Receive();
            pPacket;
            connection->DeallocatePacket( pPacket ),
            pPacket = connection->Receive()
        )
            onRecvMessage(pPacket,pPacket->systemAddress);
    }
}
bool serverNetwork::loged(const RakNet::SystemAddress & address){
    return addrExist(address);
}
void serverNetwork::onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address){
    switch(data->data[0]){
        case MESSAGE_GAME:
            if(!loged(address))
                break;
            switch(data->data[1]){
                case M_UPDATE_BUILDING:
                    onMessageUpdateBuilding(data,address);
                break;
                case M_UPDATE_TERRAIN:
                    onMessageUpdateTerrain(data,address);
                break;
                case M_UPDATE_USER:
                    onMessageUpdateUser(data,address);
                break;
            }
        break;
        case ID_NEW_INCOMING_CONNECTION:
            //下一版本修改
            setUserPosition(irr::core::vector3df(0,0,0),address);
            printf("connect\n");
        break;
        case ID_DISCONNECTION_NOTIFICATION:
            delListener(address);
            printf("disconnect\n");
        break;
    }
}

void serverNetwork::onMessageUpdateBuilding(RakNet::Packet * data,const RakNet::SystemAddress & address){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case B_CREATE_UPLOAD:
            onMessageUpdateBuildingCreate(&bs,address);
        break;
        case B_ATTACK_UPLOAD:
            onMessageUpdateBuildingAttack(&bs,address);
        break;
        case B_DOWNLOAD_CHUNK:
            onMessageUpdateBuildingDownload(&bs,address);
        break;
    }
}
void serverNetwork::onMessageUpdateTerrain(RakNet::Packet * data,const RakNet::SystemAddress & address){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case T_REMOVE:
            onMessageUpdateTerrainRemove(&bs,address);
        break;
        case T_DOWNLOAD:
            onMessageUpdateTerrainGetRMT(&bs,address);
        break;
    }
}
void serverNetwork::onMessageUpdateUser(RakNet::Packet * data,const RakNet::SystemAddress & address){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case U_SET_POSITION:
            onMessageUpdateUserPosition(&bs,address);
        break;
        case U_SET_ROTATION:
            onMessageUpdateUserRotation(&bs,address);
        break;
    }
}
void serverNetwork::onMessageUpdateUserPosition(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    //暂定
    irr::core::vector3df position;
    data->ReadVector(position.X,position.Y,position.Z);
    //printf("setPosition:(%f,%f,%f)\n",position.X,position.Y,position.Z);
    setUserPosition(position,address);
}
void serverNetwork::onMessageUpdateUserRotation(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    //暂定
}
void serverNetwork::onMessageUpdateBuildingDownload(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int32_t x,y;
    data->Read(x);
    if(!data->Read(y))
        return;
    getAllNode(x,y,address);
}
void serverNetwork::onMessageUpdateBuildingAttack(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    int32_t dmg;
    data->Read(uuid);
    if(!data->Read(dmg))
        return;
    attack(uuid.C_String(),dmg);
}
void serverNetwork::onMessageUpdateBuildingCreate(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    int64_t type;
    int32_t len;
    std::set<std::string> link;
    irr::core::vector3df position,rotation;
    data->ReadVector(position.X,position.Y,position.Z);
    data->ReadVector(rotation.X,rotation.Y,rotation.Z);
    //printf("createBuilding:(%f,%f,%f)\n",position.X,position.Y,position.Z);
    if(!data->Read(type))
        return;
    link.clear();
    if(data->Read(len)){
        for(int i=0;i<len;i++){
            if(!data->Read(uuid))
                break;
            link.insert(uuid.C_String());
        }
    }
    createNode(type,position,rotation,link);
}

void serverNetwork::onMessageUpdateTerrainGetRMT(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int32_t x,y;
    data->Read(x);
    if(!data->Read(y))
        return;
    getRemoveTable(address,x,y);
}
void serverNetwork::onMessageUpdateTerrainRemove(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int32_t x,y,mapid;
    int64_t itemid;
    data->Read(x);
    data->Read(y);
    data->Read(itemid);
    if(!data->Read(mapid))
        return;
    removeTerrain(x,y,itemid,mapid);
}

}