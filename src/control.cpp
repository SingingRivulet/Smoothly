#include "control.h"
namespace smoothly{

control::control(){
    
}
control::~control(){
    
}
void control::activeApply(){
    
    int cx=mainControlPosition.X/32;
    int cy=mainControlPosition.Z/32;
    
    //update chunks
    visualChunkUpdate(cx,cy,false);
    updateBuildingChunks(cx,cy,7);
    setUserPosition(mainControlPosition);
    
    //get direction
    //irr::core::vector3df dir = camera->getTarget() - camera->getPosition();
    //dir.normalize();
    
    irr::core::vector3df dt= mainControlPosition;
    camera->setPosition(dt);
}

void control::addCamera(){
    printf("[view]add camera\n");
    camera=scene->addCameraSceneNodeFPS();
}
bool control::eventRecv::OnEvent(const irr::SEvent &event){
    parent->camera->OnEvent(event);
    //处理按键
    //return true;
    switch(event.EventType){
        
        case irr::EET_KEY_INPUT_EVENT:
        
            #define setStatusEvent(k) \
                if(event.KeyInput.PressedDown) \
                    parent->status.k=true; \
                else \
                    parent->status.k=false;
            
            switch(event.KeyInput.Key){
                case irr::KEY_KEY_W:
                    setStatusEvent(moveFront);
                break;
                case irr::KEY_KEY_A:
                    setStatusEvent(moveLeft);
                break;
                case irr::KEY_KEY_S:
                    setStatusEvent(moveBack);
                break;
                case irr::KEY_KEY_D:
                    setStatusEvent(moveRight);
                break;
                case irr::KEY_LCONTROL:
                    setStatusEvent(moveDown);
                break;
                case irr::KEY_SPACE:
                    setStatusEvent(moveUp);
                break;
                
                case irr::KEY_KEY_Z:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,0));
                break;
                case irr::KEY_KEY_X:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,1));
                break;
                case irr::KEY_KEY_C:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,2));
                break;
                case irr::KEY_KEY_V:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,3));
                break;
                case irr::KEY_KEY_B:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,4));
                break;
                case irr::KEY_KEY_N:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,5));
                break;
                case irr::KEY_KEY_M:
                    if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,6));
                break;
                case irr::KEY_TAB:
                    if(event.KeyInput.PressedDown){
                        parent->flyMode=!parent->flyMode;
                        if(parent->flyMode==false){
                            {
                                if(!parent->mainControlUUID.empty()){//发送命令，关闭飞行模式
                                    subsCommond cmd;
                                    cmd.uuid=parent->mainControlUUID;
                                    cmd.method=subsCommond::FLY_STOP;
                                    parent->pushSubsCommond(cmd);//该函数是线程安全的，所以可以这样用
                                }
                            }
                        }else{
                            {
                                if(!parent->mainControlUUID.empty()){//发送命令，关闭行走模式
                                    subsCommond cmd;
                                    cmd.uuid=parent->mainControlUUID;
                                    cmd.method=subsCommond::WALK_STOP;
                                    parent->pushSubsCommond(cmd);//该函数是线程安全的，所以可以这样用
                                }
                            }
                        }
                    }
                break;
            }
            #undef setStatusEvent
            
        break;
        
        case irr::EET_MOUSE_INPUT_EVENT:
            switch(event.MouseInput.Event){
                case irr::EMIE_LMOUSE_PRESSED_DOWN:
                
                break;
                case irr::EMIE_RMOUSE_PRESSED_DOWN:
                    parent->eventQueue.push(gemeEPair(START_BUILDING_MODE,0));
                break;
                case irr::EMIE_MMOUSE_PRESSED_DOWN:
                
                break;
                case irr::EMIE_LMOUSE_LEFT_UP:
                
                break;
                case irr::EMIE_RMOUSE_LEFT_UP:
                    parent->eventQueue.push(gemeEPair(BUILDING_APPLY,0));
                break;
                case irr::EMIE_MMOUSE_LEFT_UP:
                
                break;
            }
        break;
    }
}
void control::addEventRecv(){
    auto old=device->getEventReceiver();
    receiver.parent=this;
    device->setEventReceiver(&receiver);
}
void control::loop(){
    if(!ok())//engine is not running
        return;
    
    recv();
    
    if(mainControlUUID.empty()){//don't know who am I
        //printf("[debug]don't know who an I\n");
        return;
    }
    
    buildingApply();
    deltaTimeUpdate();
    irr::core::line3d<irr::f32> line;
    auto cameraPosition      = camera->getPosition();
    line.start               = cameraPosition;
    irr::core::vector3df dir = camera->getTarget()-line.start;
    line.end                 = line.start+dir.normalize()*32.0f;
    doBuildUpdate(line);
    
    subsCommond cmd;
    cmd.uuid=mainControlUUID;
    
    //把摄像机的旋转发送给substance
    cmd.method=subsCommond::DIRECT;
    cmd.vec=dir;
    pushSubsCommond(cmd);
    
    if(flyMode){
        if(status.moveFront && !status.moveBack){
            
            cmd.method=subsCommond::FLY;
            if(status.moveUp){
                cmd.flying  =true;
                cmd.lifting =true;
            }else{
                cmd.flying  =true;
                cmd.lifting =false;
            }
            pushSubsCommond(cmd);
            
        }else{
            cmd.method=subsCommond::FLY_STOP;
            pushSubsCommond(cmd);
        }
    }else{
        
        cmd.method=subsCommond::WALK;
        
        //前后
        if(status.moveFront && !status.moveBack){
            cmd.walkForward=1;
        }else
        if(!status.moveFront && status.moveBack){
            cmd.walkForward=-1;
        }else{
            cmd.walkForward=0;
        }
        
        if(status.moveLeft && !status.moveRight){
            cmd.walkLeftOrRight=1;
        }else
        if(!status.moveLeft && status.moveRight){
            cmd.walkLeftOrRight=-1;
        }else{
            cmd.walkLeftOrRight=0;
        }
        
        pushSubsCommond(cmd);
        
        if(status.moveUp && !status.moveDown){//处理跳跃
            status.moveUp=false;//关掉状态，直到下次按键才会重新启动
            
            cmd.method=subsCommond::JUMP;
            irr::core::vector3df dirj=dir;
            dirj.Y=0;
            dirj.normalize();
            
            cmd.vec.set(0,1,0);
            
            if(status.moveFront && !status.moveBack){
                cmd.vec+=dirj;
            }else
            if(!status.moveFront && status.moveBack){
                cmd.vec-=dirj;
            }
        
            pushSubsCommond(cmd);
            
        }else
        if(!status.moveUp && status.moveDown){
            
        }
    }
    
    
    activeApply();
    
    while(!eventQueue.empty()){
        auto ele=eventQueue.front();
        switch(ele.first){
            case SET_BUILDING_MODE:
                buildingSelected=ele.second;
            break;
            case START_BUILDING_MODE:
                buildingModeBegin();
            break;
            case BUILDING_APPLY:
                doBuildApply();
                doBuildEnd();
            break;
        }
        eventQueue.pop();
    }
    
    worldLoop();
    sceneLoop();
    terrainLoop();
}

}