#include "serverNetwork.h"
#include <sys/types.h>
#include <sys/stat.h>
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

serverNetwork::serverNetwork(){
    
}
serverNetwork::~serverNetwork(){
    
}
void serverNetwork::start(const char * pre,short port,int maxcl){
    char path[PATH_MAX];
    
    lstPoolInit();
    
    
    scriptInit("./script/server.lua");
    
    mkdir(pre,0777);
    
    snprintf(path,sizeof(path),"%s/rmt",pre);
    removeTable::init(path);
    
    snprintf(path,sizeof(path),"%s/gra",pre);
    graphServer::init(path);
    
    snprintf(path,sizeof(path),"%s/sub",pre);
    subsServer::subsInit(path);
    
    snprintf(path,sizeof(path),"%s/user",pre);
    users::usersInit(path);
    
    connection=RakNet::RakPeerInterface::GetInstance();
    if(connection==NULL){
        printf("RakNet::RakPeerInterface::GetInstance() Error!\n");
        return;
    }
    RakNet::SocketDescriptor desc(port, 0);
    connection->Startup( maxcl, &desc, 1 );
    connection->SetMaximumIncomingConnections( maxcl );
}
void serverNetwork::release(){
    subsCache.clear();
    if(connection){
        RakNet::RakPeerInterface::DestroyInstance(connection);
    }
    users::userDestroy();
    subsServer::subsDestroy();
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
    std::string tmpuuid;
    return (addrExist(address) && getUUIDByAddr(tmpuuid,address));
}
void serverNetwork::onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address){
    switch(data->data[0]){
        case MESSAGE_GAME:
            if(data->data[1]==M_ADMIN){
                onMessageAdmin(data,address);
                break;
            }
            if(!loged(address)){
                if(data->data[1]==M_UPDATE_USER && data->data[2]==U_LOGIN){
                    RakNet::BitStream bs(data->data,data->length,false);
                    bs.IgnoreBytes(3);
                    onMessageUpdateUserLogin(&bs,address);
                }
                break;
            }
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
                case M_UPDATE_SUBS:
                    onMessageUpdateSubs(data,address);
                break;
            }
        break;
        case ID_NEW_INCOMING_CONNECTION:
            //登录后才有listener
            //setUserPosition(irr::core::vector3df(0,0,0),address);
            printf("connect\n");
        break;
        case ID_DISCONNECTION_NOTIFICATION:
            logout(address);
            //delListener(address);
            //listener已经被删除
            printf("disconnect\n");
        break;
    }
}

void serverNetwork::onMessageAdmin(RakNet::Packet * data,const RakNet::SystemAddress & address){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    RakNet::RakString name,pwd;
    
    if(!bs.Read(name))return;
    if(!bs.Read(pwd)) return;
    
    if(checkAdminPwd(name.C_String() , pwd.C_String())){
        switch(data->data[2]){
            case A_CREATE_USER:
                onMessageAdminCreateUser(&bs,address);
            break;
        }
    }
}

void serverNetwork::onMessageAdminCreateUser(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int64_t id;
    irr::core::vector3df position;
    RakNet::RakString pwd;
    
    if(!data->Read(id))return;
    if(!data->ReadVector(position.X,position.Y,position.Z))return;
    if(!data->Read(pwd))return;
    
    std::string uuid;
    
    createUser(uuid , pwd.C_String() , position , id);
    
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_ADMIN);
    
    bs.Write((RakNet::MessageID)A_SEND_USER_UUID);
    
    RakNet::RakString u;
    u=uuid.c_str();
    bs.Write(u);
    
    sendMessage(&bs,address);
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
        case U_LOGIN:
            onMessageUpdateUserLogin(&bs,address);
        break;
        
        case U_LOGOUT:
            onMessageUpdateUserLogout(&bs,address);
        break;
        
        case U_CHANGE_PWD:
            onMessageUpdateUserChangePwd(&bs,address);
        break;
    }
}
void serverNetwork::onMessageUpdateSubs(RakNet::Packet * data,const RakNet::SystemAddress & address){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case S_UL_TELEPORT:
            onMessageUpdateSubsTeleport(&bs,address);
        break;
        
        case S_UL_STATUS:
            onMessageUpdateSubsSetSubs(&bs,address);
        break;
        
        case S_UL_CREATE:
            onMessageUpdateSubsCreate(&bs,address);
        break;
        
        case S_UL_ATTACK:
            onMessageUpdateSubsAttack(&bs,address);
        break;
        
        case S_UL_REMOVE:
            onMessageUpdateSubsRemove(&bs,address);
        break;
        
        case S_RQ_CHUNK:
            onMessageUpdateSubsRequestChunk(&bs,address);
        break;
        
        case S_RQ_UUID:
            onMessageUpdateSubsRequestUUID(&bs,address);
        break;
    }
}
void serverNetwork::onMessageUpdateSubsCreate(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int64_t id;
    irr::core::vector3df position,rotation;
    irr::core::vector3df imp,point;
    
    RakNet::RakString config;
    
    if(!data->Read(id))return;
    if(!data->ReadVector(position.X,position.Y,position.Z))return;
    if(!data->ReadVector(rotation.X,rotation.Y,rotation.Z))return;
    if(!data->ReadVector(imp.X,imp.Y,imp.Z))return;
    if(!data->ReadVector(point.X,point.Y,point.Z))return;
    
    data->Read(config);//config can be null
    
    createSubs(id,position,rotation,btVector3(imp.X,imp.Y,imp.Z),btVector3(point.X,point.Y,point.Z),config.C_String(),address);
}
void serverNetwork::onMessageUpdateSubsRemove(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    if(!data->Read(uuid))return;
    
    removeSubs(uuid.C_String());
}
void serverNetwork::onMessageUpdateSubsAttack(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    int32_t dmg;
    if(!data->Read(uuid))return;
    if(!data->Read(dmg))return;
    
    attackSubs(uuid.C_String() , dmg);
}
void serverNetwork::onMessageUpdateSubsRequestChunk(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    int32_t x,y;
    if(!data->Read(x))return;
    if(!data->Read(y))return;
    
    sendSubs(address,x,y);
}
void serverNetwork::onMessageUpdateSubsRequestUUID(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    if(!data->Read(uuid))return;
    
    sendSubs(address,uuid.C_String());
}
void serverNetwork::onMessageUpdateSubsSetSubs(RakNet::BitStream * data,const RakNet::SystemAddress & address){//所有用户不断调用，至于采纳谁的，由服务器决定
    RakNet::RakString uuid;
    irr::core::vector3df position,rotation;
    irr::core::vector3df lin,ang;
    int32_t status;
    
    if(!data->Read(uuid))return;
    if(!data->Read(status))return;
    if(!data->ReadVector(position.X,position.Y,position.Z))return;
    if(!data->ReadVector(rotation.X,rotation.Y,rotation.Z))return;
    if(!data->ReadVector(lin.X,lin.Y,lin.Z))return;
    if(!data->ReadVector(ang.X,ang.Y,ang.Z))return;
    
    setSubs(uuid.C_String(),position,rotation,btVector3(lin.X,lin.Y,lin.Z),btVector3(ang.X,ang.Y,ang.Z),status,address);
}
void serverNetwork::onMessageUpdateSubsTeleport(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    irr::core::vector3df position;
    
    if(!data->Read(uuid))return;
    if(!data->ReadVector(position.X,position.Y,position.Z))return;
    
    teleport(uuid.C_String(),position);
}
void serverNetwork::onMessageUpdateSubsGiveUp(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    if(!data->Read(uuid))return;
    
    giveUpControl(uuid.C_String(),address);
}

void serverNetwork::onMessageUpdateUserLogin(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    RakNet::RakString pwd;
    if(!data->Read(uuid))return;
    if(!data->Read(pwd)) return;
    login(uuid.C_String() , address , pwd.C_String());
}
void serverNetwork::onMessageUpdateUserChangePwd(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    RakNet::RakString uuid;
    RakNet::RakString opwd;
    RakNet::RakString npwd;
    if(!data->Read(uuid))return;
    if(!data->Read(opwd))return;
    if(!data->Read(npwd))return;
    changePwd(uuid.C_String() , opwd.C_String() , npwd.C_String());
}
void serverNetwork::onMessageUpdateUserLogout(RakNet::BitStream * data,const RakNet::SystemAddress & address){
    logout(address);
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
void serverNetwork::sendSubsStatus(
    const std::string & subsuuid,
    long id , 
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& lin_vel,
    const btVector3& ang_vel,
    int status,
    int hp,
    const std::string & useruuid,
    const std::string & config,
    const RakNet::SystemAddress & to
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_DL_STATUS);
    
    RakNet::RakString u,conf;
    
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
    
    conf=config.c_str();
    bs.Write(conf);
    
    sendMessage(&bs,to);
}
void serverNetwork::sendPutSubsFail(const RakNet::SystemAddress & to){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_FAIL);
    
    sendMessageU(&bs,to);
}
void serverNetwork::sendSetUserSubs(const RakNet::SystemAddress & to,const std::string & uuid){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_SET_USER_SUBS);
    
    RakNet::RakString u;
    u=uuid.c_str();
    bs.Write(u);
    
    sendMessage(&bs,to);
}
void serverNetwork::sendUserSubsUUID(const std::string & uuid,const RakNet::SystemAddress & to){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_SUBS_UUID);
    
    RakNet::RakString u;
    u=uuid.c_str();
    bs.Write(u);
    
    sendMessage(&bs,to);
}
void serverNetwork::sendUserSubsUUIDs(const std::list<std::string> & uuids,const RakNet::SystemAddress & to){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_SUBS_UUIDS);
    
    bs.Write((int32_t)uuids.size());
    
    RakNet::RakString u;
    for(auto it:uuids){
        u=it.c_str();
        bs.Write(u);
    }
    
    sendMessage(&bs,to);
}
void serverNetwork::sendChunkRun(int x,int y,const RakNet::SystemAddress & to){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_RUN_CHUNK);
    
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    
    sendMessage(&bs,to);
}

}
