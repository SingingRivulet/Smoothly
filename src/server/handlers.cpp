#include "handlers.h"
namespace smoothly{
namespace server{
/////////////////

handlers::handlers(int thnum):admin(thnum){

}

void handlers::boardcast_wearing_add(const std::string & uuid,int x,int y,int d){
    makeHeader('B','A');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.Write((int32_t)d);
    boardcast(x,y,&bs);
}
void handlers::boardcast_wearing_remove(const std::string & uuid,int x,int y,int d){
    makeHeader('B','R');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.Write((int32_t)d);
    boardcast(x,y,&bs);
}
void handlers::boardcast_hp(const std::string & uuid,int x,int y,int hp){
    makeHeader('B','H');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.Write((int32_t)hp);
    boardcast(x,y,&bs);
}
void handlers::boardcast_setStatus(const std::string & uuid,int x,int y,int s){
    makeHeader('B','S');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.Write((int32_t)s);
    boardcast(x,y,&bs);
}
void handlers::boardcast_setLookAt(const std::string & uuid,int x,int y,const vec3 & v){
    makeHeader('B','l');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.WriteVector(v.X ,v.Y ,v.Z);
    boardcast(x,y,&bs);
}
void handlers::boardcast_setPosition(const std::string & uuid,int x,int y,const vec3 & v){
    makeHeader('B','p');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.WriteVector(v.X ,v.Y ,v.Z);
    boardcast(x,y,&bs);
}
void handlers::boardcast_setRotation(const std::string & uuid,int x,int y,const vec3 & v){
    makeHeader('B','r');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.WriteVector(v.X ,v.Y ,v.Z);
    boardcast(x,y,&bs);
}
void handlers::boardcast_interactive(const std::string & uuid,int x,int y,const std::string & s){
    makeHeader('B','i');
    RakNet::RakString u=uuid.c_str();
    RakNet::RakString ss=s.c_str();
    bs.Write(u);
    bs.Write(ss);
    boardcast(x,y,&bs);
}

void handlers::boardcast_bodyRemove(const std::string & uuid,int x,int y){
    makeHeader('B','-');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    boardcast(x,y,&bs);
}

void handlers::boardcast_createBody(const std::string & uuid,int x,int y,
    int id,int hp,int status,const std::string & owner,
    const vec3 & p,const vec3 & r,const vec3 & l){
    makeHeader('B','+');
    
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    
    bs.Write((int32_t)id);
    bs.Write((int32_t)hp);
    bs.Write((int32_t)status);
    
    RakNet::RakString o=owner.c_str();
    bs.Write(o);
    
    bs.WriteVector(p.X ,p.Y ,p.Z);
    bs.WriteVector(r.X ,r.Y ,r.Z);
    bs.WriteVector(l.X ,l.Y ,l.Z);
    
    boardcast(x,y,&bs);
}
void handlers::sendAddr_mainControl(const RakNet::SystemAddress & addr,const std::string & uuid){
    makeHeader('B','/');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    sendMessage(&bs,addr);
}
void handlers::sendAddr_body(const RakNet::SystemAddress & addr,
    const std::string & uuid,
    int id,int hp,int status,const std::string & owner,
    const vec3 & p,const vec3 & r,const vec3 & l,
    const std::set<int> & wearing){
    
    makeHeader('B','=');
    
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    
    bs.Write((int32_t)id);
    bs.Write((int32_t)hp);
    bs.Write((int32_t)status);
    
    RakNet::RakString o=owner.c_str();
    bs.Write(o);
    
    bs.WriteVector(p.X ,p.Y ,p.Z);
    bs.WriteVector(r.X ,r.Y ,r.Z);
    bs.WriteVector(l.X ,l.Y ,l.Z);
    
    bs.Write((int32_t)wearing.size());
    for(auto it:wearing){
        bs.Write((int32_t)it);
    }
    
    sendMessage(&bs,addr);
}
void handlers::sendRemoveTable(const ipair & p , const std::string & to){
    try{
        std::list<std::pair<int,int> > rmt;
        getRemovedItem(p.x,p.y, rmt);
        RakNet::SystemAddress addr;
        getAddrByUUID(to,addr);
        sendAddr_removeTable(addr,p.x,p.y, rmt);
        //sendAddr_chunkACL(addr,p,cache_chunkACL[p]);
    }catch(...){
        logError();
    }
}
void handlers::sendAddr_removeTable(const RakNet::SystemAddress & addr,
    int x,int y,
    const std::list<std::pair<int,int> > & rmt){
    
    makeHeader('R','=');
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    bs.Write((int32_t)rmt.size());
    for(auto it:rmt){
        bs.Write((int32_t)it.first);
        bs.Write((int32_t)it.second);
    }
    sendMessage(&bs,addr);
    
}

void handlers::sendAddr_chunkACL(const RakNet::SystemAddress & addr,const ipair & posi, map::chunkACL_t & acl){
    makeHeader('G','a');
    bs.Write((int32_t)posi.x);
    bs.Write((int32_t)posi.y);
    bs.Write(acl.allowBuildingWrite);
    bs.Write(acl.allowCharacterDamage);
    bs.Write(acl.allowTerrainItemWrite);
    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & text){
    makeHeader('P','=');

    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    t = text.c_str();
    bs.Write(t);

    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag_resourceNum(const RakNet::SystemAddress & addr, const std::string & uuid, int id, int num){
    makeHeader('P','R');

    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    bs.Write((int32_t)id);
    bs.Write((int32_t)num);

    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag_toolDur(const RakNet::SystemAddress & addr, const std::string & uuid, int dur, int pwr){
    makeHeader('P','D');
    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    bs.Write((int32_t)dur);
    bs.Write((int32_t)pwr);
    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag_tool_add(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID){
    makeHeader('P','+');

    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    t = toolUUID.c_str();
    bs.Write(t);

    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag_tool_remove(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID){
    makeHeader('P','-');

    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    t = toolUUID.c_str();
    bs.Write(t);

    sendMessage(&bs,addr);
}

void handlers::sendAddr_bag_tool_use(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID){
    makeHeader('P','<');

    RakNet::RakString t = uuid.c_str();
    bs.Write(t);
    t = toolUUID.c_str();
    bs.Write(t);

    sendMessage(&bs,addr);
}

void handlers::boardcast_addRemovedItem(int x,int y,int id,int index){
    makeHeader('R','+');
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    bs.Write((int32_t)id);
    bs.Write((int32_t)index);
    boardcast(x,y,&bs);
}

void handlers::boardcast_shoot(const std::string & user,int id,const vec3 & f,const vec3 & d){
    makeHeader('S','A');
    RakNet::RakString u=user.c_str();
    bs.Write(u);
    bs.Write((int32_t)id);
    bs.WriteVector(f.X ,f.Y ,f.Z);
    bs.WriteVector(d.X ,d.Y ,d.Z);
    boardcast(floor(f.X/32) , floor(f.Z/32) , &bs);
}

void handlers::boardcast_buildingRemove(const std::string & uuid, int x, int y){
    makeHeader('T','-');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    boardcast(x,y,&bs);
}

void handlers::boardcast_buildingAdd(const std::string & uuid, int id, const vec3 & p, const vec3 & r, int x, int y){
    makeHeader('T','+');
    RakNet::RakString u=uuid.c_str();
    bs.Write(u);
    bs.Write((int32_t)id);
    bs.WriteVector(p.X ,p.Y ,p.Z);
    bs.WriteVector(r.X ,r.Y ,r.Z);
    boardcast(x,y,&bs);
}

void handlers::sendBuildingChunk(int32_t x, int32_t y, const RakNet::SystemAddress & addr){
    getBuildingChunk(x,y,[&](const std::string & uuid, const vec3 & p, const vec3 & r,int id){
        makeHeader('T','+');
        RakNet::RakString u=uuid.c_str();
        bs.Write(u);
        bs.Write((int32_t)id);
        bs.WriteVector(p.X ,p.Y ,p.Z);
        bs.WriteVector(r.X ,r.Y ,r.Z);
        sendMessage(&bs,addr);
    });
    makeHeader('T','~');
    bs.Write(x);
    bs.Write(y);
    sendMessage(&bs,addr);
}

void handlers::boardcast_packageRemove(int x, int y, const std::string & uuid){
    makeHeader('p','-');
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    RakNet::RakString t=uuid.c_str();
    bs.Write(t);
    boardcast(x,y,&bs);
}

#define makePackageAdd \
    makeHeader('p','+');\
    bs.Write((int32_t)x);\
    bs.Write((int32_t)y);\
    RakNet::RakString t=uuid.c_str();\
    bs.Write(t);\
    t=text.c_str();\
    bs.Write(t);

void handlers::boardcast_packageAdd(int x, int y, const std::string & uuid, const std::string & text){
    makePackageAdd;
    boardcast(x,y,&bs);
}

void handlers::sendAddr_packageAdd(const RakNet::SystemAddress & addr, int x, int y, const std::string & uuid, const std::string & text){
    makePackageAdd;
    sendMessage(&bs,addr);
}

void handlers::sendAddr_unlockTech(const RakNet::SystemAddress & addr, bool newTech, int id){
    makeHeader('t','u');
    bs.Write(newTech);
    bs.Write((int32_t)id);
    sendMessage(&bs,addr);
}

void handlers::sendAddr_techTarget(const RakNet::SystemAddress & addr, bool newTarget, int id){
    makeHeader('t','t');
    bs.Write(newTarget);
    bs.Write((int32_t)id);
    sendMessage(&bs,addr);
}

void handlers::sendAddr_missionList(const RakNet::SystemAddress & addr, const std::vector<std::string> & s){
    makeHeader('I','L');
    bs.Write((int32_t)s.size());
    for(auto it:s){
        bs.Write(RakNet::RakString(it.c_str()));
    }
    sendMessage(&bs,addr);
}

void handlers::sendAddr_missionText(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & text){
    makeHeader('I','t');
    bs.Write(RakNet::RakString(uuid.c_str()));
    bs.Write(RakNet::RakString(text.c_str()));
    sendMessage(&bs,addr);
}

void handlers::boardcast(int x,int y,RakNet::BitStream * data){
    fetchUserByDBVT(x,y,[&](const std::string &,const RakNet::SystemAddress &addr){
        sendMessage(data,addr);
    });
}
/////////////////
}//////server
}//////smoothly
