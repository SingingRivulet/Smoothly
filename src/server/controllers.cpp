#include "controllers.h"
#include <unordered_set>

namespace smoothly{
namespace server{
/////////////////

controllers::controllers(int thnum):handlers(thnum){

}

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
        case 'S':
            switch(a){
                case 'A':
                    ctl_fire(uuid,addr,data);
                break;
            }
        break;
        case 'T':
            switch (a) {
                case '+':
                    ctl_addBuilding(uuid,addr,data);
                break;
                case '-':
                    ctl_damageBuilding(uuid,addr,data);
                break;
                case 'G':
                    ctl_getBuilding(uuid,addr,data);
                break;
            }
        break;
        case 'P':
            switch (a) {
                case 'G':
                    ctl_getTool(uuid,addr,data);
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

void controllers::ctl_fire(const std::string & uuid,const RakNet::SystemAddress & addr,RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t id;
    vec3 f,d;
    data->Read(u);
    data->Read(id);
    data->ReadVector(f.X ,f.Y ,f.Z);
    data->ReadVector(d.X ,d.Y ,d.Z);

    std::string ob = u.C_String();
    if(getOwner(ob)==uuid && !uuid.empty())//验证权限
        shoot(addr , ob , id , f , d);
}

void controllers::ctl_addBuilding(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    vec3 p,r;
    int32_t id;
    data->Read(id);
    data->ReadVector(p.X ,p.Y ,p.Z);
    data->ReadVector(r.X ,r.Y ,r.Z);
    RakNet::RakString u;
    std::unordered_set<std::string> s;
    for(int i=0;i<8;i++){
        if(!data->Read(u))
            break;
        if(u.IsEmpty())
            break;
        if(u.GetLength()>64)
            break;
        s.insert(u.C_String());
    }
    std::list<std::string> l;
    for(auto it:s){
        l.push_back(it);
    }
    createBuilding(p,r,l,id,uuid,addr);
}

void controllers::ctl_damageBuilding(const std::string & /*uuid*/, const RakNet::SystemAddress &, RakNet::BitStream * data){
    RakNet::RakString u;
    int32_t dmg;
    data->Read(u);
    data->Read(dmg);
    damageBuilding(u.C_String() , dmg);
}

void controllers::ctl_getBuilding(const std::string & /*uuid*/, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    int32_t x,y;
    data->Read(x);
    data->Read(y);
    sendBuildingChunk(x,y,addr);
}

void controllers::ctl_getTool(const std::string & /*uuid*/, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    RakNet::RakString u;
    data->Read(u);
    if(u.IsEmpty())
        return;
    try{
        bag_tool & t = cache_tools[u.C_String()];
        makeHeader('P','G');
        bs.Write(u);
        std::string str;
        t.toString(str);
        u = str.c_str();
        bs.Write(u);
        sendMessage(&bs,addr);
    }catch(...){}
}
/////////////////
}//////server
}//////smoothly
