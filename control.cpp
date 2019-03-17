#include "control.h"
namespace smoothly{
void clientNetwork::move(const irr::core::vector3df & mto){
    //printf("camrea:(%f,%f,%f)\n",mto.X,mto.Y,mto.Z);
    int cx=mto.X/32;
    int cy=mto.Z/32;

    visualChunkUpdate(cx,cy,false);
    updateBuildingChunks(cx,cy,7);
    setUserPosition(mto);
}
control::control(){
    walkSpeed=0.01;
    flyDir=true;
}
control::~control(){
    
}
void control::relativePositionApply(){
    irr::core::vector3df dt= relativePosition + cameraPosition;
    clientNetwork::move(dt);
    camera->setPosition(dt);
}
void control::setRelativePosition(const irr::core::vector3df & delta){
    relativePosition+=(delta * this->deltaTime)*walkSpeed;
}
void control::setCameraPosition(const irr::core::vector3df & p){
    clientNetwork::move(p);
    camera->setPosition(p);
}
/*
 * rotate:
 *          |cos(a)  sin(a) |
 * (x,y)  * |               | = ( x*cos(a)-y*sin(a) , x*sin(a)+y*cos(a) )
 *          |-sin(a) cos(a) |
*/
static void rotate2d(irr::core::vector2df & v,float a){
    auto cosa=cos(a);
    auto sina=sin(a);
    auto x=v.X*cosa - v.Y*sina;
    auto y=v.X*sina + v.Y*cosa;
    v.X=x;
    v.Y=y;
    //v.normalize();
}

void control::moveFront(){
    irr::core::vector3df p(moveTo.X , 0 , moveTo.Y);
    setRelativePosition(p);
}
void control::moveBack(){
    irr::core::vector3df p(-moveTo.X , 0 , -moveTo.Y);
    setRelativePosition(p);
}
void control::moveLeft(){
    irr::core::vector2df p2d=moveTo;
    rotate2d(p2d, 3.1415926/2);
    p2d.normalize();
    irr::core::vector3df p(p2d.X , 0 , p2d.Y);
    setRelativePosition(p);
}
void control::moveRight(){
    irr::core::vector2df p2d=moveTo;
    rotate2d(p2d, -3.1415926/2);
    p2d.normalize();
    irr::core::vector3df p(p2d.X , 0 , p2d.Y);
    setRelativePosition(p);
}
void control::moveUp(){
    if(canFly){
        irr::core::vector3df p(0 , 1 , 0);
        setRelativePosition(p);
    }
}
void control::moveDown(){
    if(canFly){
        irr::core::vector3df p(0 , -1 , 0);
        setRelativePosition(p);
    }
}
void control::setCameraDirect(const irr::core::vector3df & delta){
    moveTo.set(delta.X,delta.Z);
    moveTo.normalize();
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
    buildingApplay();
    deltaTimeUpdate();
    irr::core::line3d<irr::f32> line;
    cameraPosition           = camera->getPosition();
    line.start               = cameraPosition;
    irr::core::vector3df dir = camera->getTarget()-line.start;
    line.end                 = line.start+dir.normalize()*32.0f;
    setCameraDirect(dir);
    doBuildUpdate(line);
    dir.normalize();
    relativePosition.set(0,0,0);
    
    if(status.moveFront && !status.moveBack){
        if(flyDir){
            setRelativePosition(dir);
        }else
            moveFront();
    }else
    if(!status.moveFront && status.moveBack){
        if(flyDir){
            setRelativePosition(-dir);
        }else
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
                doBuildApplay();
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