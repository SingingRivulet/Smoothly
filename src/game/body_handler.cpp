#include "body.h"

//自己拥有的body不接受来自服务器的消息
#define returnInMine \
    if(!myUUID.empty() && myUUID==it->second->owner) \
        return;

#define findBody(x) \
    auto it = bodies.find(uuid); \
    if(it==bodies.end()){ \
        cmd_getBody(uuid); \
    }else

namespace smoothly{

void body::msg_wearing_add(const char* uuid,int d){
    findBody(uuid){
        //returnInMine;
        addWearing(it->second , d);
    }
}

void body::msg_wearing_set(const char* uuid,const std::set<int> & t){
    findBody(uuid){
        //returnInMine;
        setWearing(it->second , t);
    }
}

void body::msg_wearing_remove(const char* uuid,int d){
    findBody(uuid){
        //returnInMine;
        removeWearing(it->second , d);
    }
}

void body::msg_HPInc(const char* uuid,int d){
    findBody(uuid){
        //returnInMine;
        it->second->hp = d;
    }
}

void body::msg_setStatus(const char* uuid,int d){
    findBody(uuid){
        returnInMine;
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
    findBody(uuid){
        returnInMine;
        it->second->lookAt.set(x,y,z);
    }
}

void body::msg_setPosition(const char* uuid,float x,float y,float z){
    findBody(uuid){
        returnInMine;
        setBodyPosition(uuid,vec3(x,y,z));
    }
}

void body::msg_setRotation(const char* uuid,float x,float y,float z){
    findBody(uuid){
        returnInMine;
        it->second->m_character.setRotation(vec3(x,y,z));
    }
}

void body::msg_interactive(const char* uuid,const char * s){
    findBody(uuid){
        returnInMine;
        it->second->interactive(s);
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
    addBody(uuid,id,hp,status,owner,
            vec3(px,py,pz),
            vec3(rx,ry,rz),
            vec3(lx,ly,lz));
}

void body::msg_setBody(
        const char* uuid,int id,int hp,int status,const char* owner,
        float px,float py,float pz,
        float rx,float ry,float rz,
        float lx,float ly,float lz,
        const std::set<int> & wearing){
    addBody(uuid,id,hp,status,owner,
            vec3(px,py,pz),
            vec3(rx,ry,rz),
            vec3(lx,ly,lz));
    findBody(uuid){
        setWearing(it->second , wearing);
    }
}

void body::msg_setMainControl(const char * uuid){//设置第一人称的body
    defaultMainControl = uuid;
    setMainControl(uuid);
}

void body::setMainControl(const std::string & uuid){
    mainControl = uuid;
    findBody(uuid){
        mainControlBody = it->second;
        needUpdateUI = true;
    }
}

void body::msg_setBag(const char * uuid, const char * text){
    findBody(uuid){
        it->second->loadBag(text);
        if(it->second==mainControlBody)
            //updateBagUI();
            needUpdateUI = true;
    }
}

void body::msg_setBagResource(const char * uuid, int id, int num){
    findBody(uuid){
        it->second->resources[id] = num;
        if(it->second==mainControlBody)
            //updateBagUI();
            needUpdateUI = true;
    }
}

void body::msg_setBagTool(const char * uuid, const char * str){
    tools[uuid].loadStr(str);
    //updateBagUI();
    needUpdateUI = true;
}

void body::msg_setBagToolDur(const char * uuid, int dur,int pwr){
    auto it = tools.find(uuid);
    if(it!=tools.end()){
        it->second.dur = dur;
        it->second.pwr = pwr;
        //updateBagUI();
        needUpdateUI = true;
    }
}

void body::msg_bag_tool_add(const char * uuid, const char * tuuid){
    findBody(uuid){
        it->second->tools.insert(tuuid);
        cmd_getBagTool(tuuid);
    }
}

void body::msg_bag_tool_remove(const char * uuid, const char * tuuid){
    findBody(uuid){
        it->second->tools.erase(tuuid);
        tools.erase(tuuid);
        if(it->second->usingTool == tuuid)
            it->second->usingTool.clear();
        if(it->second==mainControlBody)
            needUpdateUI = true;
    }
}

void body::msg_bag_tool_use(const char * uuid, const char * tuuid){
    findBody(uuid){
        it->second->usingTool = tuuid;
        if(it->second==mainControlBody)
            needUpdateUI = true;
    }
}

std::string body::getMissionAccepter(){
    return mainControl;
}

}
