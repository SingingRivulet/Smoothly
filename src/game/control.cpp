#include "control.h"

namespace smoothly{

control::control(){
    addEventRecv();
}

void control::processControl(){
    //set lookAt
    if(mainControlBody){
        irr::core::line3d<irr::f32> line;
        auto cameraPosition      = mainControlBody->node->getPosition();
        camera->setPosition(cameraPosition);
        line.start               = cameraPosition;
        vec3 dir = camera->getTarget()-line.start;
        line.end                 = line.start+dir.normalize()*32.0f;
        commond cmd;

        cmd.cmd = CMD_SET_LOOKAT;
        cmd.data_vec = dir;
        pushCommond(cmd);

        //cmd.cmd = CMD_SET_ROTATION;
        //cmd.data_vec = dir.getHorizontalAngle();
        //cmd.data_vec.X = 0;
        //cmd.data_vec.Z = 0;
        //pushCommond(cmd);
    }
}

void control::addEventRecv(){
    //auto old=device->getEventReceiver();
    receiver.parent=this;
    device->setEventReceiver(&receiver);
}
bool control::eventRecv::OnEvent(const irr::SEvent &event){
    //处理按键
    //return true;
    commond cmd;
    switch(event.EventType){

        case irr::EET_KEY_INPUT_EVENT:

            #define setStatusEvent(k) \
                if(event.KeyInput.PressedDown) \
                    parent->status.k=true; \
                else \
                    parent->status.k=false;

            switch(event.KeyInput.Key){
                case irr::KEY_KEY_W:
                    cmd.data_int = BM_WALK_F;
                    if(event.KeyInput.PressedDown)
                        cmd.cmd = CMD_STATUS_ADD;
                    else
                        cmd.cmd = CMD_STATUS_REMOVE;
                    parent->pushCommond(cmd);
                break;
                case irr::KEY_KEY_A:
                    cmd.data_int = BM_WALK_L;
                    if(event.KeyInput.PressedDown)
                        cmd.cmd = CMD_STATUS_ADD;
                    else
                        cmd.cmd = CMD_STATUS_REMOVE;
                    parent->pushCommond(cmd);
                break;
                case irr::KEY_KEY_S:
                    cmd.data_int = BM_WALK_B;
                    if(event.KeyInput.PressedDown)
                        cmd.cmd = CMD_STATUS_ADD;
                    else
                        cmd.cmd = CMD_STATUS_REMOVE;
                    parent->pushCommond(cmd);
                break;
                case irr::KEY_KEY_D:
                    cmd.data_int = BM_WALK_R;
                    if(event.KeyInput.PressedDown)
                        cmd.cmd = CMD_STATUS_ADD;
                    else
                        cmd.cmd = CMD_STATUS_REMOVE;
                    parent->pushCommond(cmd);
                break;
                case irr::KEY_LCONTROL:
                    //蹲
                break;
                case irr::KEY_SPACE:
                    //跳
                    if(event.KeyInput.PressedDown){
                        cmd.cmd = CMD_JUMP;
                        cmd.data_vec.set(0,1,0);
                        parent->pushCommond(cmd);
                    }else{
                    }
                break;

                case irr::KEY_KEY_Z:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,0));
                break;
                case irr::KEY_KEY_X:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,1));
                break;
                case irr::KEY_KEY_C:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,2));
                break;
                case irr::KEY_KEY_V:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,3));
                break;
                case irr::KEY_KEY_B:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,4));
                break;
                case irr::KEY_KEY_N:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,5));
                break;
                case irr::KEY_KEY_M:
                    //if(event.KeyInput.PressedDown)parent->eventQueue.push(gemeEPair(SET_BUILDING_MODE,6));
                break;
                case irr::KEY_KEY_Y:
                    if(event.KeyInput.PressedDown){
                        parent->pathFindingMode = true;
                    }else{
                        if(parent->pathFindingMode){
                            parent->pathFindingMode = false;
                            parent->findPathByRay();
                        }
                    }
                break;
                case irr::KEY_KEY_T:
                    if(event.KeyInput.PressedDown){
                        parent->buildingStart();
                    }else{
                        parent->buildingEnd();
                    }
                break;
                case irr::KEY_KEY_5:
                    if(event.KeyInput.PressedDown){
                        parent->switchBuilding();
                    }
                break;
                case irr::KEY_OEM_3:
                    parent->cancle();
                break;
                case irr::KEY_TAB:
                    /*
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
                    */
                break;
                case irr::KEY_ESCAPE:
                    parent->running = false;
                break;
                default:
                break;
            }
            #undef setStatusEvent

        break;

        case irr::EET_MOUSE_INPUT_EVENT:
            switch(event.MouseInput.Event){
                case irr::EMIE_LMOUSE_PRESSED_DOWN:
                    cmd.cmd = CMD_FIRE_BEGIN;
                    parent->pushCommond(cmd);
                break;
                case irr::EMIE_RMOUSE_PRESSED_DOWN:
                    //parent->eventQueue.push(gemeEPair(START_BUILDING_MODE,0));

                break;
                case irr::EMIE_MMOUSE_PRESSED_DOWN:

                break;
                case irr::EMIE_LMOUSE_LEFT_UP:
                    cmd.cmd = CMD_FIRE_BEGIN;
                    parent->pushCommond(cmd);
                break;
                case irr::EMIE_RMOUSE_LEFT_UP:
                    //parent->eventQueue.push(gemeEPair(BUILDING_APPLY,0));

                break;
                case irr::EMIE_MMOUSE_LEFT_UP:

                break;
                default:break;
            }
        break;
        default:break;
    }
    parent->camera->OnEvent(event);
    return true;
}

}
