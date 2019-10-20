#include "handlers.h"
namespace smoothly{
namespace server{
/////////////////

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
    
    boardcast(p.X/32 , p.Z/32 , &bs);
}
void handlers::sendRemoveTable(const ipair & p , const std::string & to){
    try{
        std::list<std::pair<int,int> > rmt;
        getRemovedItem(p.x,p.y, rmt);
        RakNet::SystemAddress addr;
        getAddrByUUID(to,addr);
        sendAddr_removeTable(addr,p.x,p.y, rmt);
    }catch(...){
        logError();
    }
}
void handlers::sendAddr_removeTable(const RakNet::SystemAddress & addr,
    int x,int y,
    const std::list<std::pair<int,int> > & rmt){
    
    makeHeader('R','=');
    bs.Write((int32_t)rmt.size());
    for(auto it:rmt){
        bs.Write((int32_t)it.first);
        bs.Write((int32_t)it.second);
    }
    boardcast(x,y,&bs);
    
}

void handlers::boardcast_addRemovedItem(int x,int y,int id,int index){
    makeHeader('R','+');
    bs.Write((int32_t)x);
    bs.Write((int32_t)y);
    bs.Write((int32_t)id);
    bs.Write((int32_t)index);
    boardcast(x,y,&bs);
}

void handlers::boardcast(int x,int y,RakNet::BitStream * data){
    std::set<std::string> o;
    getUsers(x,y,o);
    for(auto it:o){
        try{
            RakNet::SystemAddress addr;
            getAddrByUUID(it,addr);
            sendMessage(data,addr);
        }catch(...){
            logError();
        }
    }
}
/////////////////
}//////server
}//////smoothly
