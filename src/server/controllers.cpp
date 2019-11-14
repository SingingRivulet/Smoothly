#include "controllers.h"
namespace smoothly{
namespace server{
/////////////////

void controllers::onMessage(const std::string & uuid,const RakNet::SystemAddress & addr,char c,char a,RakNet::BitStream * data){
    switch(c){
        case 'R':
            switch(a){
                case '+':
                    ctl_addRemovedItem(uuid,addr,data);
                break;
            }
        break;
        case 'B':
            switch(a){
                case 'A':
                    ctl_wearing_add(uuid,addr,data);
                break;
                case 'R':
                    ctl_wearing_remove(uuid,addr,data);
                break;
                case 'G':
                    ctl_wearing_get(uuid,addr,data);
                break;
                case 'H':
                    ctl_HPInc(uuid,addr,data);
                break;
                case 'S':
                    ctl_setStatus(uuid,addr,data);
                break;
                case 'l':
                    ctl_setLookAt(uuid,addr,data);
                break;
                case 'p':
                    ctl_setPosition(uuid,addr,data);
                break;
                case 'r':
                    ctl_setRotation(uuid,addr,data);
                break;
                case 'i':
                    ctl_interactive(uuid,addr,data);
                break;
                case '-':
                    
                break;
                case '+':
                    
                break;
                case '=':
                    
                break;
            }
        break;
    }
}
//===========================================================================================================
void controllers::ctl_addRemovedItem(const std::string & ,const RakNet::SystemAddress &,RakNet::BitStream * data){
    int32_t x,y,id,index;
    data->Read(x);
    data->Read(y);
    data->Read(id);
    data->Read(index);
    addRemovedItem(x,y,id,index);
    boardcast_addRemovedItem(x,y,id,index);
}

void controllers::ctl_wearing_add(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t d;
    data->Read(u);
    data->Read(d);
    wearing_add(uuid , u.C_String() , d);
}
void controllers::ctl_wearing_remove(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t d;
    data->Read(u);
    data->Read(d);
    wearing_remove(uuid , u.C_String() , d);
}
void controllers::ctl_wearing_get(const std::string & ,const RakNet::SystemAddress & addr,RakNet::BitStream * data){
    RakNet::RakString u;
    data->Read(u);
    std::set<int> wearing;
    wearing_get(u.C_String() , wearing);
    
    makeHeader('B','G');
    bs.Write(u);
    bs.Write((int32_t)wearing.size());
    for(auto it:wearing){
        bs.Write((int32_t)it);
    }
    sendMessage(&bs,addr);
}
void controllers::ctl_setLookAt(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    vec3 v;
    data->Read(u);
    data->ReadVector(v.X ,v.Y ,v.Z);
    setLookAt(uuid , u.C_String() , v);
}
void controllers::ctl_setStatus(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t d;
    data->Read(u);
    data->Read(d);
    setStatus(uuid , u.C_String() , d);
}
void controllers::ctl_setPosition(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    vec3 v;
    data->Read(u);
    data->ReadVector(v.X ,v.Y ,v.Z);
    setPosition(uuid , u.C_String() , v);
}
void controllers::ctl_setRotation(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    vec3 v;
    data->Read(u);
    data->ReadVector(v.X ,v.Y ,v.Z);
    setRotation(uuid , u.C_String() , v);
}
void controllers::ctl_interactive(const std::string & uuid,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    RakNet::RakString s;
    data->Read(u);
    data->Read(s);
    interactive(uuid , u.C_String() , s.C_String());
}
void controllers::ctl_HPInc(const std::string & ,const RakNet::SystemAddress &,RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t d;
    data->Read(u);
    data->Read(d);
    HPInc(u.C_String() , d);
}
void controllers::ctl_getBody(const std::string & ,const RakNet::SystemAddress & addr,RakNet::BitStream * data){
    RakNet::RakString u;
    data->Read(u);
    sendBodyToAddr(addr , u.C_String());
}
/////////////////
}//////server
}//////smoothly
