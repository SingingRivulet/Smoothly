#include "connection.h"
namespace smoothly{
namespace server{
/////////////////
void connection::start(unsigned short port,int maxcl){
    con=RakNet::RakPeerInterface::GetInstance();
    if(con==NULL){
        printf(L_RED "[error]" NONE "RakNet::RakPeerInterface::GetInstance() Error!\n");
        return;
    }else
        printf(L_GREEN "[status]" NONE "open raknet at port" L_CYAN " %d" NONE "\n",port);
    RakNet::SocketDescriptor desc(port, 0);
    con->Startup( maxcl, &desc, 1 );
    con->SetMaximumIncomingConnections( maxcl );
}
void connection::release(){
    if(con){
        RakNet::RakPeerInterface::DestroyInstance(con);
    }
}
void connection::linkUUID(const std::string & uuid,const RakNet::SystemAddress & addr){
    std::string rmu;
    bool hrmu=false;
    RakNet::SystemAddress rma;
    bool hrma=false;
    
    {
        auto it = uuids.find(uuid);
        if(it!=uuids.end()){
            rma  = it->second;
            hrma = true;
        }
    }
    {
        auto it = addrs.find(addr);
        if(it!=addrs.end()){
            rmu  = it->second;
            hrmu = true;
        }
    }
    if(hrma)
        addrs.erase(rma);
    if(hrmu)
        uuids.erase(rmu);
    
    addrs[addr]=uuid;
    uuids[uuid]=addr;
}
void connection::getAddrByUUID(const std::string & uuid,RakNet::SystemAddress & addr){
    auto it=uuids.find(uuid);
    if(it==uuids.end()){
        throw std::out_of_range("getAddrByUUID");
    }else{
        addr = it->second;
    }
}
void connection::getUUIDByAddr(const RakNet::SystemAddress & addr,std::string & uuid){
    auto it=addrs.find(addr);
    if(it==addrs.end()){
        throw std::out_of_range("getAddrByUUID");
    }else{
        uuid = it->second;
    }
}

void connection::login(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & pwd){
    std::string value;
    db->Get(leveldb::ReadOptions(), std::string("uPw:")+uuid , &value);
    if(value.empty())
        return;
    if(value!=pwd)
        return;
    linkUUID(uuid,addr);
    //sendMapToUser(uuid);
    //直接发address效率更高
    std::set<ipair> m;
    getUserNodes(uuid,m);//发送用户的节点
    for(auto p:m){
        std::set<std::string> em;
        getNode(p.x , p.y , em);
        for(auto it:em){//先发送body
            sendBodyToAddr(addr,it);
        }
        //发rmt
        std::list<std::pair<int,int> > rmt;
        getRemovedItem(p.x,p.y, rmt);
        sendAddr_removeTable(addr,p.x,p.y, rmt);
    }
    //发送main control
    try{
        auto s = getMainControl(uuid);
        sendAddr_mainControl(addr,s);
    }catch(...){
        logError();
    }
}
void connection::sendBodyToAddr(const RakNet::SystemAddress & addr,const std::string & uuid){
    try{
        int  id       = getId(uuid);
        int  hp       = getHP(uuid);
        int  status   = getStatus(uuid);
        auto owner    = getOwner(uuid);
        auto position = getPosition(uuid);
        auto rotation = getRotation(uuid);
        auto lookAt   = getLookAt  (uuid);
        std::set<int> wearing;
        wearing_get(uuid,wearing);
        sendAddr_body(addr,uuid,id,hp,status,owner,position,rotation,lookAt,wearing);
    }catch(...){
        logError();
    }
}
void connection::logout(const RakNet::SystemAddress & addr){
    auto it = addrs.find(addr);
    if(it!=addrs.end()){
        uuids.erase(it->second);
    }
    addrs.erase(addr);
}
void connection::logout(const std::string & uuid){
    auto it = uuids.find(uuid);
    if(it!=uuids.end()){
        addrs.erase(it->second);
    }
    uuids.erase(uuid);
}
void connection::setPwd(const std::string & uuid,const std::string & pwd){
    db->Put(leveldb::WriteOptions(), std::string("uPw:")+uuid , pwd);
}
void connection::recv(){
    if(con){
        RakNet::Packet* pPacket;
        for(
            pPacket = con->Receive();
            pPacket;
            con->DeallocatePacket( pPacket ),
            pPacket = con->Receive()
        )
            onRecvMessage(pPacket,pPacket->systemAddress);
    }
}
void connection::onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address){
    char addrStr[64];
    address.ToString(true,addrStr);
    switch(data->data[0]){
        case MESSAGE_GAME:
            if(data->data[1]=='+'){
                RakNet::BitStream bs(data->data,data->length,false);
                bs.IgnoreBytes(2);
                RakNet::RakString uuid;
                RakNet::RakString pwd;
                bs.Read(uuid);
                bs.Read(pwd);
                login(address , uuid.C_String() , pwd.C_String());
            }else
            if(data->data[1]=='.'){
                if(data->length<4){
                    break;
                }
                try{
                    std::string uuid;
                    getUUIDByAddr(address,uuid);
                    RakNet::BitStream bs(data->data,data->length,false);
                    bs.IgnoreBytes(4);
                    onMessage(uuid,address,data->data[2],data->data[3],&bs);
                }catch(...){
                    logError();
                }
            }else
            if(data->data[1]=='a'){
                RakNet::BitStream bs(data->data,data->length,false);
                bs.IgnoreBytes(2);
                adminMessage(&bs , address);
            }
        break;
        case ID_NEW_INCOMING_CONNECTION:
            //登录后才有listener
            //setUserPosition(irr::core::vector3df(0,0,0),address);
            printf("[%s client]connect\n",addrStr);
        break;
        case ID_DISCONNECTION_NOTIFICATION:
            logout(address);
            //delListener(address);
            //listener已经被删除
            printf("[%s client]disconnect\n",addrStr);
        break;
    }
}
void connection::send_body(const std::string & to,
    const std::string & uuid,
    int id,int hp,int status,const std::string & owner,
    const vec3 & p,const vec3 & r,const vec3 & l,
    const std::set<int> & wearing){
    try{
        RakNet::SystemAddress addr;
        getAddrByUUID(to , addr);
        sendAddr_body(addr,uuid,id,hp,status,owner,p,r,l,wearing);
    }catch(...){
        logError();
    }
}
/////////////////
}//////server
}//////smoothly
