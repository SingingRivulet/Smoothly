#include "connection.h"
namespace smoothly{
namespace server{
/////////////////
connection::connection(int thnum):building(thnum){
    lastAutoKickTime = 0;
    dbvtTimeStep = 0;
}
void connection::start(unsigned short port, int maxcl, int vf){
    con=RakNet::RakPeerInterface::GetInstance();
    if(con==NULL){
        printf(L_RED "[error]" NONE "RakNet::RakPeerInterface::GetInstance() Error!\n");
        return;
    }else
        printf(L_GREEN "[status]" NONE "open raknet at port" L_CYAN " %d" NONE "\n",port);
    RakNet::SocketDescriptor desc(port, 0);
    con->Startup( maxcl, &desc, 1 );
    con->SetMaximumIncomingConnections( maxcl );

    printf(L_GREEN"[status]" NONE "visualField = " L_CYAN "%d" NONE "\n" , vf);
    printf(L_GREEN"[status]" NONE "maxConnect = " L_CYAN "%d" NONE "\n" , maxcl);

    visualField = vf;
}
void connection::release(){
    building::release();
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
    if(addrs.find(addr)!=addrs.end())//登录过
        return;
    if(uuids.find(uuid)!=uuids.end())//登录过
        return;
    std::string value;
    db->Get(leveldb::ReadOptions(), std::string("uPw:")+uuid , &value);
    if(value.empty())
        return;
    if(value!=pwd)
        return;
    linkUUID(uuid,addr);
    sendAddr_visualrange(addr);
    //sendMapToUser(uuid);
    //直接发address效率更高
    std::set<ipair> m;
    getUserNodes(uuid,m,[&](const std::string & bodyuuid,int x,int y){
        addToDBVT(addr,bodyuuid,uuid,x,y);
    });//发送用户的节点

    std::unordered_set<std::string> buuids;
    for(auto p:m){
        getNode(p.x , p.y , [&buuids](const std::string & u){
            buuids.insert(u);
        });
    }
    //发送main control
    try{
        auto s = getMainControl(uuid);
        sendAddr_mainControl(addr,s);
    }catch(...){
        logError();
    }
    for(auto it:buuids){//先发送body
        sendBodyToAddr(addr,it);
    }
    for(auto p:m){
        //发rmt
        std::list<std::pair<int,int> > rmt;
        getRemovedItem(p.x,p.y, rmt);
        sendAddr_removeTable(addr,p.x,p.y, rmt);
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
        removeUserBox(it->second);
        addrs.erase(it);
    }
}
void connection::logout(const std::string & uuid){
    auto it = uuids.find(uuid);
    if(it!=uuids.end()){
        addrs.erase(it->second);
        uuids.erase(it);
        removeUserBox(uuid);
    }
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
        ){
            onRecvMessage(pPacket,pPacket->systemAddress);
        }
        int ntm = time(0);
        if(ntm-lastAutoKickTime>1){
            lastAutoKickTime = ntm;
            hb.removeExpire([&](const RakNet::SystemAddress & addr){
                char addrStr[64];
                addr.ToString(true,addrStr,':');
                printf(YELLOW "[timeout %s]" NONE "\n",addrStr);
                logout(addr);
                con->CloseConnection(addr,true,0);
            });
        }
        loop();
    }
}

void connection::sendAddr_visualrange(const RakNet::SystemAddress & addr){
    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)MESSAGE_GAME);
    bs.Write((RakNet::MessageID)'.');
    bs.Write((RakNet::MessageID)'=');
    bs.Write((RakNet::MessageID)'r');
    bs.Write((int32_t)visualField);
    sendMessage(&bs,addr);
}
extern std::atomic<bool> running;
void connection::onRecvMessage(RakNet::Packet * data,const RakNet::SystemAddress & address){
    char addrStr[64];
    address.ToString(true,addrStr,':');
    switch(data->data[0]){
        case MESSAGE_GAME:
            hb.markNode(address);
            if(data->length<2){
                break;
            }
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
            printf(YELLOW "[client %s]" NONE "connect\n",addrStr);
            hb.markNode(address);
        break;
        case ID_DISCONNECTION_NOTIFICATION:
            hb.eraseNode(address);
            logout(address);
            //delListener(address);
            //listener已经被删除
            //running = false;
            printf(YELLOW "[client %s]" NONE "disconnect\n",addrStr);
        break;
    }
}

void connection::addToDBVT(const RakNet::SystemAddress &addr, const std::string &uuid, const std::string &owner, int x, int y){
    charBB * p = updateDBVT(uuid,owner,x,y);
    p->address = addr;
}

void connection::updateChunkDBVT(const std::string & uuid, const std::string & owner, int x, int y){
    updateDBVT(uuid,owner,x,y,false);
}

void connection::removeFromDBVT(const std::string & uuid){
    removeDBVT(uuid);
}

void connection::removeDBVT(const std::string &uuid){
    auto it = charBBs.find(uuid);
    charBB * bb;
    if(it!=charBBs.end()){
        bb = it->second;
        charBBs.erase(it);
        auto cit = charOwners.find(bb->owner);
        if(cit!=charOwners.end()){
            userSet * c = cit->second;
            c->owned.erase(bb);
            if(c->owned.empty()){//空了，删掉索引
                delete c;
                charOwners.erase(cit);
            }
        }
        bb->box->autodrop();
        delete bb;
    }
}

void connection::removeUserBox(const std::string &owner){
    auto it = charOwners.find(owner);
    if(it!=charOwners.end()){
        userSet * us = it->second;
        for(auto bb:us->owned){
            charBBs.erase(bb->uuid);
            bb->box->autodrop();
        }
        delete us;
        charOwners.erase(it);
    }
}

connection::userSet * connection::seekUserSet(const std::string &owner){
    auto cit = charOwners.find(owner);
    if(cit!=charOwners.end()){
        userSet * c = cit->second;
        return c;
    }else{
        auto c = new userSet;
        c->timeStep = dbvtTimeStep;
        charOwners[owner]=c;
        return c;
    }
}

void connection::releaseDBVT(){
    for(auto it:charBBs){
        it.second->box->autodrop();
        delete it.second;
    }
    for(auto it:charOwners){
        delete it.second;
    }
    charBBs.clear();
    charOwners.clear();
}

void connection::fetchByDBVT(int x, int y, std::function<void (connection::charBB *)> callback){
    viewDBVT.fetchByPoint(dbvt2d::vec(x,y) , [](dbvt2d::AABB * box , void * s){
        auto callback = (std::function<void (connection::charBB *)>*)s;
        if(box->data){
            (*callback)((charBB*)box->data);
        }
    },&callback);
}

void connection::fetchUserByDBVT(int x, int y, std::function<void (const std::string &,const RakNet::SystemAddress &)> callback){
    ++dbvtTimeStep;
    fetchByDBVT(x,y,[&](connection::charBB * bb){
        if(bb->userp->timeStep==dbvtTimeStep)
            return;
        bb->userp->timeStep=dbvtTimeStep;
        callback(bb->owner,bb->address);
    });
}

connection::charBB *connection::updateDBVT(const std::string &uuid, const std::string &owner, int x, int y, bool create){
    auto it = charBBs.find(uuid);
    charBB * bb;
    if(it==charBBs.end()){//创建
        if(!create)
            return NULL;
        bb = new charBB;
        bb->owner = owner;
        bb->uuid = uuid;
        bb->x=x;
        bb->y=y;
        bb->timeStep = 0;
        dbvt2d::vec f(x-visualField,y-visualField);
        dbvt2d::vec t(x+visualField,y+visualField);
        bb->box=viewDBVT.add(f,t,bb);
        charBBs[uuid]=bb;
        auto p = seekUserSet(owner);
        bb->userp = p;
        p->owned.insert(bb);
    }else{
        bb = it->second;
        if(bb->x!=x || bb->y!=y){
            bb->x=x;
            bb->y=y;
            bb->box->autodrop();
            dbvt2d::vec f(x-visualField,y-visualField);
            dbvt2d::vec t(x+visualField,y+visualField);
            bb->box=viewDBVT.add(f,t,bb);
        }
    }
    return bb;
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
