#include "body.h"

//自己拥有的body不接受来自服务器的消息
#define returnInMine \
    if(!myUUID.empty() && myUUID==uuid) \
        return;

#define findBody(x) \
    auto it = bodies.find(uuid); \
    if(it!=bodies.end())

namespace smoothly{

void body::msg_wearing_add(const char* uuid,int d){
    returnInMine;
    findBody(uuid){
        addWearing(it->second , d);
    }
}

void body::msg_wearing_set(const char* uuid,const std::set<int> & t){
    returnInMine;
    findBody(uuid){
        setWearing(it->second , t);
    }
}

void body::msg_wearing_remove(const char* uuid,int d){
    returnInMine;
    findBody(uuid){
        removeWearing(it->second , d);
    }
}

void body::msg_HPInc(const char* uuid,int d){
    returnInMine;
    findBody(uuid){
        it->second->hp = d;
    }
}

void body::msg_setStatus(const char* uuid,int d){
    returnInMine;
    findBody(uuid){
        if(it->second->status_mask!=d){
            it->second->status=d;
            it->second->status_mask=d;
            it->second->updateStatus();
        }else{
            it->second->status=d;
            it->second->status_mask=d;
        }
    }
}

void body::msg_setLookAt(const char* uuid,float x,float y,float z){
    returnInMine;
    findBody(uuid){
        it->second->lookAt.set(x,y,z);
    }
}

void body::msg_setPosition(const char* uuid,float x,float y,float z){
    returnInMine;
    findBody(uuid){
        it->second->m_character.setPosition(vec3(x,y,z));
    }
}

void body::msg_setRotation(const char* uuid,float x,float y,float z){
    returnInMine;
    findBody(uuid){
        it->second->m_character.setRotation(vec3(x,y,z));
    }
}

void body::msg_interactive(const char* uuid,const char*){
    returnInMine;
    findBody(uuid){
        it->second->updateStatus();
    }
}

void body::msg_removeBody(const char* uuid){
    removeBody(uuid);
}

void body::msg_createBody(
        const char* uuid,int id,int hp,int status,const char* owner,
        float px,float py,float pz,
        float rx,float ry,float rz,
        float lx,float ly,float lz){

}

void body::msg_setBody(
        const char* uuid,int id,int hp,int status,const char* owner,
        float px,float py,float pz,
        float rx,float ry,float rz,
        float lx,float ly,float lz,
        const std::set<int> & wearing){

}

void body::msg_setMainControl(const char * uuid){//设置第一人称的body
    mainControl = uuid;
}

}
