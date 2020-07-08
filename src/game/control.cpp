#include "control.h"

namespace smoothly{

control::control(){
    addEventRecv();
    menu_window = gui->addImage(driver->getTexture("../../res/icon/menu.png"),irr::core::vector2d<s32>(width/2-128 , height/2-64));

    menu_exit   = gui->addButton(irr::core::rect<s32>(16,16,230,48),menu_window,-1,L"exit");
    menu_exit->setOverrideColor(video::SColor(255,0,0,0));

    menu_cmd_line = gui->addEditBox(L"",irr::core::rect<s32>(16,60,230,80),true,menu_window);

    menu_cmd    = gui->addButton(irr::core::rect<s32>(128,82,230,112),menu_window,-1,L"commond");
    menu_cmd->setOverrideColor(video::SColor(255,0,0,0));

    menu_window->setVisible(false);
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
    commond cmd;
    if(parent->camera->isInputReceiverEnabled()){
        switch(event.EventType){

            case irr::EET_KEY_INPUT_EVENT:

#define setStatusEvent(k) \
    if(event.KeyInput.PressedDown) \
    parent->status.k=true; \
    else \
    parent->status.k=false;


#define walkEvent \
            case irr::KEY_KEY_W: \
                cmd.data_int = BM_WALK_F;\
                if(event.KeyInput.PressedDown)\
                    cmd.cmd = CMD_STATUS_ADD;\
                else\
                    cmd.cmd = CMD_STATUS_REMOVE;\
                parent->pushCommond(cmd);\
                break;\
            case irr::KEY_KEY_A:\
                cmd.data_int = BM_WALK_L;\
                if(event.KeyInput.PressedDown)\
                    cmd.cmd = CMD_STATUS_ADD;\
                else\
                    cmd.cmd = CMD_STATUS_REMOVE;\
                parent->pushCommond(cmd);\
                break;\
            case irr::KEY_KEY_S:\
                cmd.data_int = BM_WALK_B;\
                if(event.KeyInput.PressedDown)\
                    cmd.cmd = CMD_STATUS_ADD;\
                else\
                    cmd.cmd = CMD_STATUS_REMOVE;\
                parent->pushCommond(cmd);\
                break;\
            case irr::KEY_KEY_D:\
                cmd.data_int = BM_WALK_R;\
                if(event.KeyInput.PressedDown)\
                    cmd.cmd = CMD_STATUS_ADD;\
                else\
                    cmd.cmd = CMD_STATUS_REMOVE;\
                parent->pushCommond(cmd);\
                break;\
            case irr::KEY_LCONTROL:\
                break;\
            case irr::KEY_SPACE:\
                if(event.KeyInput.PressedDown){\
                    cmd.cmd = CMD_JUMP;\
                    cmd.data_vec.set(0,1,0);\
                    parent->pushCommond(cmd);\
                }else{\
                }\
                break;

                switch(event.KeyInput.Key){
                    walkEvent;
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
                            parent->submitShowingMissions = true;
                        }else{
                            parent->autoPickup = false;
                            parent->submitShowingMissions = false;
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
                    case irr::KEY_KEY_M:
                        if(!event.KeyInput.PressedDown){
                            parent->setFullMapMode_auto();
                            parent->setGUIMode(true);
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
                    case irr::KEY_KEY_P:
                        if(!event.KeyInput.PressedDown){
                            parent->getChunkMission();
                        }
                        break;
                    case irr::KEY_KEY_N:
                        if(!event.KeyInput.PressedDown){
                            {
                                parent->showMails = true;
                                parent->showMailText = false;
                                //parent->getMail();
                                parent->setGUIMode(true);
                            }
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
                        if(!event.KeyInput.PressedDown){
                            {
                                if(!parent->menu_window->isVisible()){
                                    bool mode = !parent->menu_window->isVisible();
                                    parent->menu_window->setVisible(mode);
                                    parent->setGUIMode(mode);
                                }
                            }
                        }
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
    }else if(parent->tech_tree_view->isVisible()){//科技树模式
        switch(event.EventType){

            case irr::EET_KEY_INPUT_EVENT:
                switch(event.KeyInput.Key){
                    walkEvent;
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
                    case irr::gui::EGET_BUTTON_CLICKED:
                        if(event.GUIEvent.Caller && event.GUIEvent.Caller==parent->tech_button){
                            parent->techActive();
                        }
                        break;
                    default:break;
                }
                break;
            default:break;
        }
    }else if(parent->body_bag_resource->isVisible()){//背包模式

        switch(event.EventType){

            case irr::EET_KEY_INPUT_EVENT:
                switch(event.KeyInput.Key){
                    walkEvent;
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
    }else if(parent->fullmap_gui->isVisible()){//地图、任务模式

        switch(event.EventType){

            case irr::EET_KEY_INPUT_EVENT:
                switch(event.KeyInput.Key){
                    walkEvent;
                    case irr::KEY_KEY_M:
                        if(!event.KeyInput.PressedDown){
                            parent->setFullMapMode_auto();
                            parent->setGUIMode(false);
                        }
                        return true;
                    case irr::KEY_KEY_L:
                        if(!event.KeyInput.PressedDown){
                            parent->controlSelectedBody();
                        }
                        break;
                    default:break;
                    case irr::KEY_F10:
                        if(!event.KeyInput.PressedDown){
                            parent->addMissionWindow();
                        }
                        break;
                    case irr::KEY_F9:
                        if(!event.KeyInput.PressedDown){
                            parent->occupy();
                        }
                        break;
                    case irr::KEY_KEY_E:
                        if(event.KeyInput.PressedDown){
                            parent->submitShowingMissions = true;
                        }else{
                            parent->submitShowingMissions = false;
                        }
                        break;
                    case irr::KEY_KEY_P:
                        if(!event.KeyInput.PressedDown){
                            parent->getChunkMission();
                        }
                        break;
                }
                break;

            case irr::EET_MOUSE_INPUT_EVENT:
                switch(event.MouseInput.Event){
                    case irr::EMIE_MOUSE_WHEEL:
                        if(event.MouseInput.Wheel>0){
                            parent->mapCamera_height+=16;
                        }else if(event.MouseInput.Wheel<0){
                            parent->mapCamera_height-=16;
                        }
                        if(parent->mapCamera_height<128)
                            parent->mapCamera_height = 128;
                        else if(parent->mapCamera_height>1024)
                            parent->mapCamera_height = 1024;
                        return true;
                    default:break;
                }
                break;
            case irr::EET_GUI_EVENT:
                switch (event.GUIEvent.EventType) {
                    case irr::gui::EGET_BUTTON_CLICKED:
                        if(event.GUIEvent.Caller==parent->terrmapacl_save){
                            parent->uploadChunkACL();
                        }else if(event.GUIEvent.Caller==parent->button_mission_giveup){
                            parent->cmd_giveUpMission();
                        }
                        break;
                    default:break;
                }
                break;
            default:break;
        }
    }else if(parent->menu_window->isVisible()){

        switch(event.EventType){

            case irr::EET_KEY_INPUT_EVENT:
                switch(event.KeyInput.Key){
                    case irr::KEY_ESCAPE:
                        if(!event.KeyInput.PressedDown){
                            {
                                bool mode = !parent->menu_window->isVisible();
                                parent->menu_window->setVisible(mode);
                                parent->setGUIMode(mode);
                            }
                        }
                        break;
                    default:break;
                }
                break;

            case irr::EET_MOUSE_INPUT_EVENT:
                switch(event.MouseInput.Event){
                    default:break;
                }
                break;
            case irr::EET_GUI_EVENT:
                switch (event.GUIEvent.EventType) {
                    case irr::gui::EGET_BUTTON_CLICKED:
                        if(event.GUIEvent.Caller==parent->menu_cmd){
                            parent->processCmd();
                        }else if(event.GUIEvent.Caller==parent->menu_exit){
                            parent->running = false;
                        }
                        break;
                    default:break;
                }
                break;
            default:break;
        }
    }else if(parent->showMails){

        switch(event.EventType){

            case irr::EET_MOUSE_INPUT_EVENT:
                if(!parent->showMailText){
                    parent->setMailFocusByScreen(event.MouseInput.X,event.MouseInput.Y);
                    switch(event.MouseInput.Event){
                        case irr::EMIE_LMOUSE_PRESSED_DOWN:
                            if(parent->mailLastPage.isPointInside(irr::core::vector2d<irr::s32>(event.MouseInput.X,event.MouseInput.Y))){
                                parent->mail_page-=1;
                                if(parent->mail_page<1)
                                    parent->mail_page = 1;
                            }else if(parent->mailNextPage.isPointInside(irr::core::vector2d<irr::s32>(event.MouseInput.X,event.MouseInput.Y))){
                                parent->mail_page+=1;
                            }else if(parent->mail_focus!=-1){
                                parent->mailPackagePickingup = false;
                                parent->showMailText = true;
                            }
                            break;
                        default:break;
                    }
                }else{
                    switch(event.MouseInput.Event){
                        case irr::EMIE_LMOUSE_PRESSED_DOWN:
                            if(event.MouseInput.X<64 || event.MouseInput.Y<64){
                                parent->showMailText = false;
                            }else if(parent->mailPackage_button>0 &&
                                     event.MouseInput.Y>parent->mailPackage_button &&
                                     event.MouseInput.Y<parent->mailPackage_button+20){
                                parent->pickupFocusMailPackage();
                            }
                            break;
                        default:break;
                    }
                }
                break;

            case irr::EET_KEY_INPUT_EVENT:
                switch(event.KeyInput.Key){
                    walkEvent;
                    case irr::KEY_KEY_N:
                        if(!event.KeyInput.PressedDown){
                            {
                                parent->showMails = false;
                                parent->showMailText = false;
                                parent->setGUIMode(false);
                            }
                        }
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
