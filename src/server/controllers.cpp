#include "controllers.h"
#include <unordered_set>

namespace smoothly{
namespace server{
/////////////////

controllers::controllers(int thnum):handlers(thnum){

}

void controllers::onMessage(const std::string & uuid,const RakNet::SystemAddress & addr,char c,char a,RakNet::BitStream * data){
    techLoop;
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
                case 'U':
                    ctl_useTool(uuid,addr,data);
                break;
                case 'R':
                    ctl_reloadTool(uuid,addr,data);
                break;
            }
        break;
        case 'p':
            switch (a) {
                case 'p':
                    ctl_putPackage(uuid,addr,data);
                break;
                case 'u':
                    ctl_pickupPackage(uuid,addr,data);
                break;
            }
        break;
        case 't':
            switch (a) {
                case 't':
                    ctl_setTechTarget(uuid,addr,data);
                break;
            }
        break;
        case 'M':
            switch (a) {
                case 'm':
                    ctl_making(uuid,addr,data);
                break;
            }
        break;
    }
}
//===========================================================================================================
void controllers::ctl_addRemovedItem(const std::string & ,const RakNet::SystemAddress &,RakNet::BitStream * data){
    int32_t cx,cy,id,index;
    float x,y;
    data->Read(cx);
    data->Read(cy);
    data->Read(x);
    data->Read(y);
    data->Read(id);
    data->Read(index);
    if(addRemovedItem(cx,cy,x,y,id,index)){
        boardcast_addRemovedItem(cx,cy,id,index);
    }
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
    sendAddr_chunkPackage(addr,x,y);//发送地面散落的物体
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

void controllers::ctl_useTool(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    RakNet::RakString u,t;
    data->Read(u);
    data->Read(t);
    if(u.IsEmpty())
        return;
    try{

        auto ow = getOwner(u.C_String());
        if(uuid!=ow)
            return;

        useTool(addr,u.C_String(),t.C_String());
    }catch(...){}
}

void controllers::ctl_reloadTool(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    RakNet::RakString u,t;
    data->Read(u);
    data->Read(t);
    if(u.IsEmpty())
        return;
    if(t.IsEmpty())
        return;
    try{

        auto ow = getOwner(u.C_String());
        if(uuid!=ow)
            return;

        reloadTool(addr,u.C_String(),t.C_String());
    }catch(...){}
}

void controllers::ctl_putPackage(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    bool isResource;
    int32_t skin;
    float x,y,z;
    RakNet::RakString b;
    if(data->Read(isResource) && data->Read(skin) && data->ReadVector(x,y,z) && data->Read(b)){

        auto ow = getOwner(b.C_String());
        if(uuid!=ow)
            return;

        if(isResource){
            int32_t id,num;
            if(data->Read(id) && data->Read(num)){
                putPackage_resource(addr,b.C_String(),skin,vec3(x,y,z),id,num);
            }
        }else{
            RakNet::RakString t;
            if(data->Read(t)){
                putPackage_tool(addr,b.C_String(),skin,vec3(x,y,z),t.C_String());
            }
        }

    }
}

void controllers::ctl_pickupPackage(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    int32_t x,y;
    RakNet::RakString b,t;
    data->Read(b);
    data->Read(x);
    data->Read(y);
    data->Read(t);
    auto ow = getOwner(b.C_String());
    if(uuid!=ow)
        return;
    pickupPackage(addr,x,y,t.C_String(),b.C_String());
}

void controllers::ctl_setTechTarget(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    int32_t target;
    if(data->Read(target)){
        setTechTarget(addr,uuid,target);
    }
}

void controllers::ctl_making(const std::string & uuid, const RakNet::SystemAddress & addr, RakNet::BitStream * data){
    int32_t id;
    float x,y,z;
    RakNet::RakString bag;
    if(data->Read(id) && data->ReadVector(x,y,z) && data->Read(bag)){
        auto ow = getOwner(bag.C_String());
        if(uuid!=ow)
            return;//检查所有权

        bool status = make(addr,uuid,bag.C_String(),id,vec3(x,y,z));

        makeHeader('M','s');
        bs.Write(id);
        bs.Write(status);
        sendMessage(&bs,addr);
    }
}
/////////////////
}//////server
}//////smoothly
