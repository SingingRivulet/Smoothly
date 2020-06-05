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

void control::setGUIMode(bool m)
{
    device->getCursorControl()->setVisible(m);
    camera->setInputReceiverEnabled(!m);
    if(!m){
        device->getCursorControl()->setPosition(0.5f,0.5f);
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
    if(parent->camera->isInputReceiverEnabled()){
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
                    case irr::KEY_KEY_Y://设置移动目标
                        if(event.KeyInput.PressedDown){
                            if(parent->pathFindingMode){

                            }else{
                                parent->pathFindingMode = true;
                                parent->pathFindingMode_startTime = parent->timer->getTime();
                            }
                        }else{
                            if(parent->pathFindingMode){
                                if(parent->timer->getTime() - parent->pathFindingMode_startTime > 2000){
                                    parent->pathFindingMode = false;
                                    parent->followMainControl();
                                }else{
                                    parent->pathFindingMode = false;
                                    parent->findPathByRay();
                                }
                            }
                        }
                        break;
                    case irr::KEY_KEY_T://建造
                        if(event.KeyInput.PressedDown){
                            parent->buildingStart();
                        }else{
                            parent->buildingEnd();
                        }
                        break;
                    case irr::KEY_KEY_G://建造
                        if(event.KeyInput.PressedDown){
                        }else{
                            parent->dropHand();
                        }
                        break;
                    case irr::KEY_KEY_E://拾取
                        if(event.KeyInput.PressedDown){
                            parent->autoPickup = true;
                        }else{
                            parent->autoPickup = false;
                        }
                        break;
                    case irr::KEY_KEY_L://手动模式
                        if(!event.KeyInput.PressedDown){
                            parent->controlSelectedBody();
                        }
                        break;
                    case irr::KEY_KEY_5://切换建筑
                        if(event.KeyInput.PressedDown){
                            parent->switchBuilding();
                        }
                        break;
                    case irr::KEY_KEY_6://开关自动寻路
                        if(!event.KeyInput.PressedDown){
                            parent->useAIPathingFinding =! parent->useAIPathingFinding;
                        }
                        break;
                    case irr::KEY_KEY_R://装弹
                        if(event.KeyInput.PressedDown){
                            cmd.cmd = CMD_TOOL_RELOAD_START;
                        }else{
                            cmd.cmd = CMD_TOOL_RELOAD_END;
                        }
                        parent->pushCommond(cmd);
                        break;
                    case irr::KEY_KEY_H:
                        if(event.KeyInput.PressedDown){
                            parent->selectBodyStart();
                        }else{
                            parent->selectBodyEnd();
                        }
                        break;
                    case irr::KEY_OEM_3://取消
                        parent->cancle();
                        break;
                    case irr::KEY_TAB:
                        if(event.KeyInput.PressedDown){
                            parent->body_bag_resource->setVisible(true);
                            parent->setGUIMode(true);
                        }else{
                            parent->body_bag_resource->setVisible(false);
                            parent->setGUIMode(false);
                        }
                        break;
                    case irr::KEY_OEM_1:
                        if(!event.KeyInput.PressedDown){
                            {
                                if(!parent->body_bag_resource->isVisible()){
                                    bool mode = !parent->tech_tree_view->isVisible();
                                    parent->tech_tree_view->setVisible(mode);
                                    parent->setGUIMode(mode);
                                }
                            }
                        }
                        break;
                    case irr::KEY_PRIOR:
                        if(event.KeyInput.PressedDown){
                            if(parent->body_bag_resource->isVisible()){
                                --parent->bagPage;
                                if(parent->bagPage<0)
                                    parent->bagPage=0;
                                parent->needUpdateUI = true;
                            }
                        }
                        break;
                    case irr::KEY_NEXT:
                        if(event.KeyInput.PressedDown){
                            if(parent->body_bag_resource->isVisible()){
                                ++parent->bagPage;
                                parent->needUpdateUI = true;
                            }
                        }
                        break;
                    case irr::KEY_ESCAPE:
                        parent->running = false;
                        break;
                    case irr::KEY_F11:
                        if(!event.KeyInput.PressedDown){
                            parent->showVoxelsByCamera();
                        }
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
                        cmd.cmd = CMD_FIRE_END;
                        parent->pushCommond(cmd);
                        break;
                    case irr::EMIE_RMOUSE_LEFT_UP:
                        //parent->eventQueue.push(gemeEPair(BUILDING_APPLY,0));

                        break;
                    case irr::EMIE_MMOUSE_LEFT_UP:

                        break;
                    case irr::EMIE_MOUSE_WHEEL:
                        if(event.MouseInput.Wheel>0){
                            parent->bag_selectLast();
                        }else if(event.MouseInput.Wheel<0){
                            parent->bag_selectNext();
                        }
                        break;
                    default:break;
                }
                break;
            default:break;
        }
        parent->camera->OnEvent(event);
        return false;
    }else{

        if(parent->tech_tree_view->isVisible()){//科技树模式
            switch(event.EventType){

                case irr::EET_KEY_INPUT_EVENT:
                    switch(event.KeyInput.Key){
                        case irr::KEY_OEM_1:
                            if(!event.KeyInput.PressedDown){
                                {
                                    bool mode = !parent->tech_tree_view->isVisible();
                                    parent->tech_tree_view->setVisible(mode);
                                    parent->setGUIMode(mode);
                                }
                            }
                            break;
                        default:break;
                    }
                    break;
                case irr::EET_GUI_EVENT:
                    switch (event.GUIEvent.EventType) {
                        case irr::gui::EGET_TREEVIEW_NODE_SELECT:
                            if(event.GUIEvent.Caller && event.GUIEvent.Caller->getType()==irr::gui::EGUIET_TREE_VIEW){
                                auto elm = (irr::gui::IGUITreeView*)event.GUIEvent.Caller;
                                auto node = elm->getSelected();
                                if(node){
                                    parent->selectTech(node);
                                }
                            }
                            break;
                        default:break;
                    }
                    break;
                default:break;
            }
        }else
        if(parent->body_bag_resource->isVisible()){//背包模式

            switch(event.EventType){

                case irr::EET_KEY_INPUT_EVENT:
                    switch(event.KeyInput.Key){
                        case irr::KEY_TAB:
                            if(event.KeyInput.PressedDown){
                                parent->body_bag_resource->setVisible(true);
                                parent->setGUIMode(true);
                            }else{
                                parent->body_bag_resource->setVisible(false);
                                parent->setGUIMode(false);
                            }
                            return true;

                        case irr::KEY_PRIOR:
                            if(event.KeyInput.PressedDown){
                                if(parent->body_bag_resource->isVisible()){
                                    --parent->bagPage;
                                    if(parent->bagPage<0)
                                        parent->bagPage=0;
                                    parent->needUpdateUI = true;
                                }
                            }
                            return true;
                        case irr::KEY_NEXT:
                            if(event.KeyInput.PressedDown){
                                if(parent->body_bag_resource->isVisible()){
                                    ++parent->bagPage;
                                    parent->needUpdateUI = true;
                                }
                            }
                            return true;
                        default:break;
                    }
                    break;

                case irr::EET_MOUSE_INPUT_EVENT:
                    switch(event.MouseInput.Event){
                        case irr::EMIE_MOUSE_WHEEL:
                            if(event.MouseInput.Wheel>0){
                                parent->bag_selectLast();
                            }else if(event.MouseInput.Wheel<0){
                                parent->bag_selectNext();
                            }
                            return true;
                        default:break;
                    }
                    break;
                case irr::EET_GUI_EVENT:
                    switch (event.GUIEvent.EventType) {
                        case irr::gui::EGET_LISTBOX_SELECTED_AGAIN:
                            parent->useTool(parent->body_bag_resource->getSelected());
                            break;
                        default:break;
                    }
                    break;
                default:break;
            }
        }
        return false;
    }
}

}
