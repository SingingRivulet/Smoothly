#include "body.h"

namespace smoothly{

void body::updateBagUI(){
    body_bag_resource->clear();
    body_bag_using->clear();
    body_bag_using->setVisible(false);
    int bagStartAt = bagPage*8;
    int index=0;
    int keyIndex=0;
    wchar_t buf[256];
    bool selectTool = false;
    handItems.clear();
    if(mainControlBody){
        for(auto it:mainControlBody->tools){
            if(index>=bagStartAt && index<=(bagStartAt+8)){
                auto t = tools.find(it);
                if(t!=tools.end()){
                    if(keyIndex>9)
                        break;
                    handItems.push_back(hand_t(it));
                    std::string pad;
                    if(t->second.id==fastUseTool[0])
                        pad += "(1)";
                    if(t->second.id==fastUseTool[1])
                        pad += "(2)";
                    if(t->second.id==fastUseTool[2])
                        pad += "(3)";
                    if(t->second.id==fastUseTool[3])
                        pad += "(4)";
                    swprintf(buf,256,L"\nUUID:%s %s\nDP:%d\n",it.c_str(),pad.c_str(),t->second.dur);
                    ++keyIndex;
                    int id;
                    auto icit = bag_tool_icons_mapping.find(t->second.id);
                    if(icit==bag_tool_icons_mapping.end()){
                        id = body_bag_resource->addItem(buf);
                    }else{
                        id = body_bag_resource->addItem(buf,icit->second);
                    }
                    if(!selectTool && mainControlBody->usingTool==it){//正在使用
                        body_bag_resource->setSelected(id);
                        selectTool = true;
                    }
                    body_bag_resource->setItemOverrideColor(id,irr::video::SColor(255,255,255,255));
                }
            }
            ++index;
        }
        std::vector<int> rms;
        for(auto it:mainControlBody->resources){
            if(it.second<=0){
                rms.push_back(it.first);
                continue;
            }
            if(index>=bagStartAt && index<=(bagStartAt+8)){
                if(keyIndex>9)
                    break;
                handItems.push_back(hand_t(it.first));
                swprintf(buf,256,L"\n×%d\n",it.second);
                int id;
                auto icit = bag_res_icons_mapping.find(it.first);
                if(icit==bag_res_icons_mapping.end()){
                    id = body_bag_resource->addItem(buf);
                }else{
                    id = body_bag_resource->addItem(buf,icit->second);
                }
                if(!selectTool && usingResource){
                    if(usingResource_id == it.first){
                        body_bag_resource->setSelected(id);
                        selectTool = true;
                    }
                }
                body_bag_resource->setItemOverrideColor(id,irr::video::SColor(255,255,255,255));
            }
            ++index;
        }
        for(auto it:rms){
            mainControlBody->resources.erase(it);
        }
        if(body_bag_page)
            body_bag_page->remove();
        swprintf(buf,256,L"%d\n",bagPage+1);
        body_bag_page = gui->addStaticText(buf,irr::core::rect<irr::s32>(380,256,400,272),false,true,body_bag_resource);
        body_bag_page->setOverrideColor(irr::video::SColor(255,255,255,255));
        body_bag_page->setOverrideFont(font);
    }
    if(!mainControlBody->usingTool.empty()){
        int bullets = 0;

        auto cit = fire_costs.find(mainControlBody->fireId);
        if(cit!=fire_costs.end()){
            if(cit->second.cost_num!=0){//需要弹药
                auto bgr = mainControlBody->resources.find(cit->second.cost_id);//定位资源
                if(bgr!=mainControlBody->resources.end()){
                    bullets = bgr->second;
                }
            }
        }

        auto t = tools.find(mainControlBody->usingTool);
        if(t!=tools.end()){

            swprintf(buf,256,L"\n%d/%d %d\n",t->second.pwr,bullets,t->second.dur);

            int usingId;

            auto icit = bag_tool_icons_mapping.find(t->second.id);
            if(icit==bag_tool_icons_mapping.end()){
                usingId = body_bag_using->addItem(buf);
            }else{
                usingId = body_bag_using->addItem(buf,icit->second);
            }
            body_bag_using->setItemOverrideColor(usingId,irr::video::SColor(128,105,218,213));
            body_bag_using->setVisible(true);
        }
    }else if(usingResource){
        int usingId;
        auto icit = bag_res_icons_mapping.find(usingResource_id);
        if(icit!=bag_res_icons_mapping.end()){

            auto urit = mainControlBody->resources.find(usingResource_id);
            if(urit!=mainControlBody->resources.end() && urit->second>0){

                swprintf(buf,256,L"\n×%d",urit->second);
                usingId = body_bag_using->addItem(buf,icit->second);
                body_bag_using->setItemOverrideColor(usingId,irr::video::SColor(128,255,255,255));
                body_bag_using->setVisible(true);

            }
        }
    }
    needUpdateUI = false;
}

void body::dropHand(){
    if(mainControlBody==NULL)
        return;
    try{
        hand_t & h = handItems.at(bag_selectId);
        auto posi = mainControlBody->node->getPosition();
        if(h.isTool){
            cmd_putPackage(0,posi.X,posi.Y,posi.Z,mainControl.c_str(),h.toolUUID.c_str());
        }else{
            cmd_putPackage(0,posi.X,posi.Y,posi.Z,mainControl.c_str(),h.resourceId,1);
        }
    }catch(...){

    }
}

void body::useTool(int id){
    if(mainControlBody==NULL)
        return;
    usingResource = false;
    try{
        hand_t & h = handItems.at(id);
        if(h.isTool){
            cmd_useBagTool(mainControl.c_str(),h.toolUUID.c_str());
        }else{
            cmd_useBagTool(mainControl.c_str(),"");
            usingResource = true;
            usingResource_id = h.resourceId;
        }
    }catch(...){
        cmd_useBagTool(mainControl.c_str(),"");
    }
}

void body::bag_selectLast(){
    --bag_selectId;
    if(bag_selectId<-1)
        bag_selectId = -1;
    useTool(bag_selectId);
}

void body::bag_selectNext(){
    ++bag_selectId;
    auto max = handItems.size();
    if(bag_selectId>max){
        bag_selectId = max;
    }
    useTool(bag_selectId);
}

void body::pickupPackageToBag(int x, int y, const std::string & uuid){
    if(mainControlBody){
        cmd_pickupPackage(mainControl.c_str(),x,y,uuid.c_str());
    }
}

void body::tool::loadStr(const char * str){
    auto json = cJSON_Parse(str);
    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"id")==0){
                    id = c->valueint;
                }else if(strcmp(c->string,"durability")==0){
                    dur = c->valueint;
                }else if(strcmp(c->string,"power")==0){
                    pwr = c->valueint;
                }
            }
            c=c->next;
        }

        cJSON_Delete(json);
    }
}

}
