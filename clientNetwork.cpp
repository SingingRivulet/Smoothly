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
    canConnect=true;
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
    terrain::destroyTexture();
    remoteGraph::destroy();
}
void clientNetwork::recv(){
    if(!connection)
        return;
    
    long t=timer->getRealTime();
    
    while(1){
        if(fabs(timer->getRealTime()-t)>80)
            break;
        auto pk=connection->Receive();
        if(pk)
            onRecvMessage(pk);
        else
            break;
    }
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
                case M_UPDATE_SUBS:
                    onMessageUpdateSubs(data);
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
             //printf("updateBuilding:attack\n");
             onMessageUpdateBuildingAttack(&bs);
        break;
        case B_CREATE:
             //printf("updateBuilding:create\n");
             onMessageUpdateBuildingCreate(&bs);
        break;
        case B_DESTROY:
             //printf("updateBuilding:destroy\n");
             onMessageUpdateBuildingDestroy(&bs);
        break;
        case B_GENER:
             //printf("updateBuilding:gen\n");
             onMessageUpdateBuildingGen(&bs);
        break;
        default:
             //printf("updateBuilding\n");
        break;
    }
}
void clientNetwork::onMessageUpdateSubs(RakNet::Packet * data){
    RakNet::BitStream bs(data->data,data->length,false);
    bs.IgnoreBytes(3);
    switch(data->data[2]){
        case S_DL_TELEPORT:
            onMessageUpdateSubsTeleport(&bs);
        break;
        case S_DL_STATUS:
            onMessageUpdateSubsSetStatus(&bs);
        break;
        case S_DL_CREATE:
            onMessageUpdateSubsCreate(&bs);
        break;
        case S_DL_CREATE_B:
            onMessageUpdateSubsCreateBrief(&bs);
        break;
        case S_DL_ATTACK:
            onMessageUpdateSubsAttack(&bs);
        break;
        case S_DL_REMOVE:
            onMessageUpdateSubsRemove(&bs);
        break;
        case S_FAIL:
            onMessageUpdateSubsFail(&bs);
        break;
        case S_SET_USER_SUBS:
            onMessageUpdateSubsSetUserSubs(&bs);
        break;
        case S_SUBS_UUID:
            onMessageUpdateSubsAddOneSubs(&bs);
        break;
        case S_SUBS_UUIDS:
            onMessageUpdateSubsAddSubs(&bs);
        break;
        case S_RUN_CHUNK:
            onMessageUpdateSubsRunChunk(&bs);
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
        case T_APPLY:
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
    //printf("genBuilding:(%f,%f,%f)\n",position.X,position.Y,position.Z);
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
    //printf("createBuilding:(%f,%f,%f)\n",position.X,position.Y,position.Z);
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
    removeTableApply(x,y);
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
    if(!canConnect)
        return;
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_TERRAIN);
    bs.Write((RakNet::MessageID)T_DOWNLOAD);
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    sendMessageU(&bs);
}
void clientNetwork::onMessageUpdateSubsTeleport(RakNet::BitStream * data){
    RakNet::RakString u;
    irr::core::vector3df position;
    
    if(!data->Read(u))return;
    if(!data->ReadVector(position.X , position.Y , position.Z))return;
    
    //call
    auto p=seekSubs(u.C_String());
    if(p)
        p->teleport(position);
}
void clientNetwork::onMessageUpdateSubsSetStatus(RakNet::BitStream * data){
    RakNet::RakString useruuid,subsuuid;
    int32_t status,hp;
    int64_t id;
    irr::core::vector3df position,rotation,lin_vel,ang_vel;
    
    if(!data->Read(id))return;
    if(!data->Read(subsuuid))return;
    if(!data->Read(useruuid))return;
    
    if(!data->ReadVector(position.X , position.Y , position.Z))return;
    if(!data->ReadVector(rotation.X , rotation.Y , rotation.Z))return;
    if(!data->ReadVector(lin_vel.X  , lin_vel.Y  , lin_vel.Z))return;
    if(!data->ReadVector(ang_vel.X  , ang_vel.Y  , ang_vel.Z))return;
    
    if(!data->Read(status))return;
    if(!data->Read(hp))return;
    
    //call
    updateSubs(
        id,
        subsuuid.C_String(),
        useruuid.C_String(),
        position,
        rotation,
        btVector3(lin_vel.X  , lin_vel.Y  , lin_vel.Z),
        btVector3(ang_vel.X  , ang_vel.Y  , ang_vel.Z),
        hp,status
    );
}
void clientNetwork::onMessageUpdateSubsCreate(RakNet::BitStream * data){
    RakNet::RakString useruuid,subsuuid;
    int64_t id;
    irr::core::vector3df position,rotation,impulse,rel_pos;
    
    if(!data->Read(id))return;
    if(!data->Read(subsuuid))return;
    if(!data->Read(useruuid))return;
    
    if(!data->ReadVector(position.X , position.Y , position.Z))return;
    if(!data->ReadVector(rotation.X , rotation.Y , rotation.Z))return;
    if(!data->ReadVector(impulse.X  , impulse.Y  , impulse.Z))return;
    if(!data->ReadVector(rel_pos.X  , rel_pos.Y  , rel_pos.Z))return;
    
    genSubs(
        subsuuid.C_String(),
        useruuid.C_String(),
        id,
        position,
        rotation,
        btVector3(impulse.X  , impulse.Y  , impulse.Z),
        btVector3(rel_pos.X  , rel_pos.Y  , rel_pos.Z)
    );
}
void clientNetwork::onMessageUpdateSubsCreateBrief(RakNet::BitStream * data){
    RakNet::RakString useruuid;
    int64_t id;
    irr::core::vector3df position,rotation,impulse,rel_pos;
    
    if(!data->Read(id))return;
    if(!data->Read(useruuid))return;
    
    if(!data->ReadVector(position.X , position.Y , position.Z))return;
    if(!data->ReadVector(rotation.X , rotation.Y , rotation.Z))return;
    if(!data->ReadVector(impulse.X  , impulse.Y  , impulse.Z))return;
    if(!data->ReadVector(rel_pos.X  , rel_pos.Y  , rel_pos.Z))return;
    
    genSubs(
        id,
        useruuid.C_String(),
        position,
        rotation,
        btVector3(impulse.X  , impulse.Y  , impulse.Z),
        btVector3(rel_pos.X  , rel_pos.Y  , rel_pos.Z)
    );
}
void clientNetwork::onMessageUpdateSubsAttack(RakNet::BitStream * data){
    RakNet::RakString subsuuid;
    int32_t hp,delta;
    
    if(!data->Read(subsuuid))return;
    if(!data->Read(hp))return;
    if(!data->Read(delta))return;
    
    attackLocalSubs(subsuuid.C_String(),hp,delta);
}
void clientNetwork::onMessageUpdateSubsRemove(RakNet::BitStream * data){
    RakNet::RakString subsuuid;
    if(!data->Read(subsuuid))return;
    
    removeLocalSubs(subsuuid.C_String());
}
void clientNetwork::onMessageUpdateSubsFail(RakNet::BitStream * data){
    //no data
}
void clientNetwork::onMessageUpdateSubsSetUserSubs(RakNet::BitStream * data){
    RakNet::RakString subsuuid;
    if(!data->Read(subsuuid))return;
    
    mainControlUUID=subsuuid.C_String();
}
void clientNetwork::onMessageUpdateSubsAddOneSubs(RakNet::BitStream * data){
    RakNet::RakString subsuuid;
    if(!data->Read(subsuuid))return;
    
    mySubs.insert(subsuuid.C_String());
}
void clientNetwork::onMessageUpdateSubsAddSubs(RakNet::BitStream * data){
    RakNet::RakString subsuuid;
    int32_t length;
    std::list<std::string> subs;
    if(!data->Read(length))return;
    for(int i=0;i<length;i++){
        if(data->Read(subsuuid)){
            mySubs.insert(subsuuid.C_String());
        }else
            break;
    }
    
}
void clientNetwork::onMessageUpdateSubsRunChunk(RakNet::BitStream * data){
    int32_t x,y;
    if(!data->Read(x))return;
    if(!data->Read(y))return;
    
    unlockChunk(x,y);
}

void clientNetwork::uploadBodyStatus(//上传持久型物体状态
    const std::string & u , 
    int status,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& lin ,
    const btVector3& ang
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_UL_STATUS);
    
    RakNet::RakString uuid=u.c_str();
    
    bs.Write(uuid);
    bs.Write((int32_t)status);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.WriteVector(lin.getX(),lin.getY(),lin.getZ());
    bs.WriteVector(ang.getX(),ang.getY(),ang.getZ());
    
    sendMessage(&bs);
}

void clientNetwork::requestCreateSubs(//请求创建物体
    long id,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_UL_CREATE);
    
    bs.Write((int64_t)id);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.WriteVector(rotation.X,rotation.Y,rotation.Z);
    bs.WriteVector(impulse.getX(),impulse.getY(),impulse.getZ());
    bs.WriteVector(rel_pos.getX(),rel_pos.getY(),rel_pos.getZ());
    
    sendMessage(&bs);
}

void clientNetwork::requestCreateBriefSubs(//请求创建物体（非持久）
    long id,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    requestCreateSubs(id,p,r,impulse,rel_pos);
}

void clientNetwork::requestCreateLastingSubs(//请求创建物体（持久）
    long id,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r, 
    const btVector3& impulse,
    const btVector3& rel_pos
){
    requestCreateSubs(id,p,r,impulse,rel_pos);
}

void clientNetwork::requestDownloadSubstanceChunk(int x,int y){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_RQ_CHUNK);
    
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    
    sendMessage(&bs);
}

void clientNetwork::requestRemoveSubs(const std::string & u){//请求删除持久物体（非持久不需要删除）
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_UL_REMOVE);
    
    RakNet::RakString uuid=u.c_str();
    bs.Write(uuid);
    
    sendMessage(&bs);
}
void clientNetwork::requestTeleport(const std::string & u,const irr::core::vector3df & position){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_UL_TELEPORT);
    
    RakNet::RakString uuid=u.c_str();
    bs.Write(uuid);
    bs.WriteVector(position.X,position.Y,position.Z);
    
    sendMessage(&bs);
}
void clientNetwork::requestAttackSubs(const std::string & u,int dmg){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_SUBS);
    
    bs.Write((RakNet::MessageID)S_UL_ATTACK);
    
    RakNet::RakString uuid=u.c_str();
    bs.Write(uuid);
    bs.Write((int32_t)dmg);
    
    sendMessage(&bs);
}

void clientNetwork::login(const std::string & name,const std::string & p){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)M_UPDATE_USER);
    
    bs.Write((RakNet::MessageID)U_LOGIN);
    
    RakNet::RakString uuid=name.c_str();
    RakNet::RakString pwd=p.c_str();
    
    bs.Write(uuid);
    bs.Write(pwd);
    
    sendMessage(&bs);
}

void clientNetwork::setUserPosition(const irr::core::vector3df & p){
    
}
void clientNetwork::setUserRotation(const irr::core::vector3df & r){
    
}

}