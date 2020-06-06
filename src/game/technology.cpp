#include "technology.h"
#include <QFile>

namespace smoothly{

technology::technology(){
    int cx = width/2;
    int cy = height/2;
    tech_tree_view = gui->addTreeView(irr::core::rect<irr::s32>(cx-300,cy-200,cx+300,cy+200),0,-1,true,true,true);
    tech_tree_view->setVisible(false);
    tech_tree_img = gui->createImageList(driver->getTexture("../../res/tech.png"),irr::core::dimension2d<irr::s32>(128,16),true);
    //高16
    //使用pinta绘制图标时，请在16*index+5处点击，字号9
    tech_tree_view->setImageList(tech_tree_img);
    tech_tree_view->setIconFont(font);
    tech_status = NULL;
    tech_button = NULL;
    tech_root = std::make_shared<tech_node>();
    tech_root->iconId = 0;
    tech_root->relatedName.clear();
    tech_root->relatedId = 0;
    tech_root->type = TECH_NONE;
    tech_root->unlocked = true;
    tech_root->techId = -1;
    tech_root->description = NULL;
    tech_index[-1] = tech_root;
    needUpdateTechUI = true;
    loadTechConfig();
}

technology::~technology(){

}

void technology::techUIUpdate(){
    tech_tree_view->getRoot()->clearChildren();//清空
    drawTechNode(tech_root,tech_tree_view->getRoot());

}

void technology::addTech(int techId, int parentId, technology::tech_type type, int relatedId, const std::string & relatedName, int iconId, video::ITexture * description){
    if(tech_index.find(techId)!=tech_index.end())//已经存在
        return;
    auto parentIt = tech_index.find(parentId);//定位父节点
    if(parentIt!=tech_index.end()){
        std::shared_ptr<tech_node> t = std::make_shared<tech_node>();
        t->iconId = iconId;
        t->relatedName = relatedName;
        t->relatedId = relatedId;
        t->type = type;
        t->unlocked = false;
        t->techId = techId;
        t->description = description;
        parentIt->second->children.push_back(t);
        tech_index[techId] = t;
        needUpdateTechUI = true;
    }
}

void technology::unlockTech(int techId){
    auto techIt = tech_index.find(techId);//定位父节点
    if(techIt!=tech_index.end()){
        techIt->second->unlocked = true;
        if(techIt->second->type==TECH_BUILD){//解锁建筑
            unlockBuilding(techIt->second->relatedId);
        }
        needUpdateTechUI = true;
    }
}

void technology::loop(){
    package::loop();
    if(needUpdateTechUI){
        techUIUpdate();
        needUpdateTechUI = false;
    }
}

void technology::loadTechConfig(){
    printf("[status]get technology config\n" );
    QFile file("../config/technology.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/technology.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();
    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;
            while (c){
                if(c->type==cJSON_Object){
                    cJSON * node_id = cJSON_GetObjectItem(c,"id");
                    cJSON * node_parent = cJSON_GetObjectItem(c,"parent");
                    cJSON * node_relatedId = cJSON_GetObjectItem(c,"relatedId");
                    cJSON * node_relatedName = cJSON_GetObjectItem(c,"relatedName");
                    cJSON * node_iconId = cJSON_GetObjectItem(c,"iconId");
                    cJSON * node_type = cJSON_GetObjectItem(c,"type");
                    cJSON * node_des = cJSON_GetObjectItem(c,"description");

                    if(node_id && node_parent &&
                       node_id->type==cJSON_Number && node_parent->type==cJSON_Number){

                        int id = node_id->valueint;
                        int parent = node_parent->valueint;

                        tech_type type = TECH_NONE;
                        if(node_type && node_type->type==cJSON_String){
                            if(strcmp(node_type->valuestring,"build")==0){
                                type = TECH_BUILD;
                            }else if(strcmp(node_type->valuestring,"make")==0){
                                type = TECH_MAKE;
                            }
                        }

                        int relatedId = 0;
                        if(node_relatedId && node_relatedId->type==cJSON_Number){
                            relatedId = node_relatedId->valueint;
                        }

                        std::string relatedName;
                        if(node_relatedName && node_relatedName->type==cJSON_String){
                            relatedName = node_relatedName->valuestring;
                        }

                        int icon;
                        if(node_iconId && node_iconId->type==cJSON_Number){
                            icon = node_iconId->valueint;
                        }

                        irr::video::ITexture * des = NULL;
                        if(node_des && node_des->type==cJSON_String){
                            des = driver->getTexture(node_des->valuestring);
                        }

                        addTech(id,parent,type,relatedId,relatedName,icon,des);
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/technology.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}

void technology::selectTech(gui::IGUITreeViewNode * n){
    if(tech_status){
        tech_status->remove();
        tech_status = NULL;
    }
    auto p = (tech_node*)n->getData();
    if(p==NULL)
        return;

    if(p->description){
        tech_status = gui->addImage(
                          driver->getTexture("../../res/tech_status_background.png"),
                          irr::core::position2di(384,2),
                          true,
                          tech_tree_view);
        gui->addImage(p->description,irr::core::position2di(8,8),true,tech_status);

        if(p->unlocked){
            if(p->type==TECH_BUILD){
                tech_button = gui->addButton(irr::core::rect<irr::s32>(8 , 8+300 , 64 , 32+300) , tech_status , -1 , L"use");
            }else if(p->type==TECH_MAKE){
                tech_button = gui->addButton(irr::core::rect<irr::s32>(8 , 8+300 , 64 , 32+300) , tech_status , -1 , L"make");
            }
        }else{
            tech_button = gui->addButton(irr::core::rect<irr::s32>(8 , 8+300 , 64 , 32+300) , tech_status , -1 , L"target");
        }
    }
}

void technology::drawTechNode(std::shared_ptr<technology::tech_node> node,irr::gui::IGUITreeViewNode * uinode){
    if(uinode==NULL)
        return;

    wchar_t buf[64];
    if(node->unlocked){
        switch (node->type) {
            case TECH_NONE:
                swprintf(buf,64,L"");
                break;
            case TECH_BUILD:
                swprintf(buf,64,L"(B)");
                break;
            case TECH_MAKE:
                swprintf(buf,64,L"(M)");
                break;
            default:break;
        }
    }else{
        swprintf(buf,64,L"(?""?""?)");
    }
    auto c = uinode->addChildBack(buf,0,node->iconId,-1,node.get(),0);

    if(node->unlocked){
        for(auto it:node->children){
            drawTechNode(it,c);
        }
    }
}


}
