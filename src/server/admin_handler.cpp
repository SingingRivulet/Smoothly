#include "admin.h"
#include <unordered_map>
namespace smoothly{
namespace server{

#define makeAdminHeader() \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)connection::MESSAGE_GAME); \
    bs.Write((RakNet::MessageID)'a');

#define sendOK(u,act) \
    makeAdminHeader(); \
    bs.Write(RakNet::RakString("OK")); \
    bs.Write(RakNet::RakString(act)); \
    bs.Write(RakNet::RakString(u)); \
    self->sendMessage(&bs,addr);

//statics
static bool inited = false;
static std::unordered_map<std::string,void(*)(admin*, RakNet::BitStream *data, const RakNet::SystemAddress & addr)> adminMap;
static std::mutex adminMapLocker;

////////////////////////////////////////////////////
//functions
static void setMainControl(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString user,uuid;
    data->Read(user);
    data->Read(uuid);
    self->setMainControl(user.C_String() , uuid.C_String());
    sendOK(user,"setMainControl");
}
static void setData(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString key,val;
    data->Read(key);
    data->Read(val);
    self->db->Put(leveldb::WriteOptions(),key.C_String(),val.C_String());
    sendOK(key,"setData");
}
static void getData(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString key;
    data->Read(key);

    std::string val;
    self->db->Get(leveldb::ReadOptions() , key.C_String() , &val);

    makeAdminHeader();
    bs.Write(RakNet::RakString("dbRes"));
    bs.Write(key);
    bs.Write(RakNet::RakString(val.c_str()));
    self->sendMessage(&bs,addr);
}

static void setPosition(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString uuid;
    float x,y,z;
    data->Read(uuid);
    data->ReadVector(x,y,z);

    try{
        self->setPosition(uuid.C_String(),vec3(x,y,z));
    }catch(...){}

    sendOK(uuid,"setPosition");
}

static void addCharacter(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString owner;
    float x,y,z;
    int32_t id;
    data->Read(owner);
    data->Read(id);
    data->ReadVector(x,y,z);
    auto res = self->addCharacter(owner.C_String(),id,vec3(x,y,z));

    makeAdminHeader();
    bs.Write(RakNet::RakString("addCharacter"));
    bs.Write(RakNet::RakString(res.c_str()));
    self->sendMessage(&bs,addr);
}

static void removeCharacter(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString uuid;
    data->Read(uuid);
    self->removeCharacter(uuid.C_String());
    sendOK(uuid,"removeCharacter");
}

static void setPwd(admin * self, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    RakNet::RakString uuid,pwd;
    data->Read(uuid);
    data->Read(pwd);
    self->setPwd(uuid.C_String(),pwd.C_String());
    sendOK(uuid,"setPwd");
}

////////////////////////////////////////////////////

void admin::adminMap_init(){
    printf(L_GREEN "[status]" NONE "load admin map\n" );
    if(!inited){
        adminMap.clear();

        //function list
        adminMap["setMainControl"]  = smoothly::server::setMainControl;
        adminMap["setData"]         = smoothly::server::setData;
        adminMap["getData"]         = smoothly::server::getData;
        adminMap["setPosition"]     = smoothly::server::setPosition;
        adminMap["addCharacter"]    = smoothly::server::addCharacter;
        adminMap["removeCharacter"] = smoothly::server::removeCharacter;
        adminMap["setPwd"]          = smoothly::server::setPwd;

    }
    inited = true;
}
void admin::admin_handler(const char * act, RakNet::BitStream *data, const RakNet::SystemAddress & addr){
    adminMapLocker.lock();
    auto it = adminMap.find(act);
    if(it==adminMap.end()){
        adminMapLocker.unlock();
        char addrStr[64];
        addr.ToString(true,addrStr);
        logger->error("fail to call:{} call by {}" , act , addrStr);
        return;
    }else {
        adminMapLocker.unlock();
        it->second(this,data,addr);
    }
}

}//////server
}//////smoothly
