#include "control.h"
namespace smoothly{

control::control(){
    
}
control::~control(){
    
}
void control::relativePositionApply(){
    
    int cx=mainControlPosition.X/32;
    int cy=mainControlPosition.Z/32;
    
    //update chunks
    visualChunkUpdate(cx,cy,false);
    updateBuildingChunks(cx,cy,7);
    setUserPosition(mainControlPosition);
    
    //get direction
    irr::core::vector3df dir = camera->getTarget() - camera->getPosition();
    dir.normalize();
    
    irr::core::vector3df dt= mainControlPosition + dir*deltaCamera;
    camera->setPosition(dt);
}

void control::moveFront(){
    
}
void control::moveBack(){
    
}
void control::moveLeft(){
    
}
void control::moveRight(){
    
}
void control::moveUp(){
    
}
void control::moveDown(){
    
}
void control::addCamera(){
    camera=scene->addCameraSceneNodeFPS();
}
bool control::eventRecv::OnEvent(const irr::SEvent &event){
    parent->camera->OnEvent(event);
    //处理按键
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
    if(!ok())
        return;
    recv();
    buildingApply();
    deltaTimeUpdate();
    irr::core::line3d<irr::f32> line;
    auto cameraPosition      = camera->getPosition();
    line.start               = cameraPosition;
    irr::core::vector3df dir = camera->getTarget()-line.start;
    line.end                 = line.start+dir.normalize()*32.0f;
    doBuildUpdate(line);
    
    if(status.moveFront && !status.moveBack){
        moveFront();
    }else
    if(!status.moveFront && status.moveBack){
        moveBack();
    }
    
    if(status.moveLeft && !status.moveRight){
        moveLeft();
    }else
    if(!status.moveLeft && status.moveRight){
        moveRight();
    }
    
    if(status.moveUp && !status.moveDown){
        moveUp();
    }else
    if(!status.moveUp && status.moveDown){
        moveDown();
    }
    
    relativePositionApply();
    
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