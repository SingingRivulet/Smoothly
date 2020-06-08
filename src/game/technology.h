#ifndef SMOOTHLY_TECHNOLOGY
#define SMOOTHLY_TECHNOLOGY

#include "package.h"
#include <memory>

namespace smoothly{

class technology:public package{
    public:
        technology();
        ~technology();
        void techUIUpdate();
        irr::gui::IGUITreeView * tech_tree_view;
        irr::gui::IGUIImageList * tech_tree_img;

        bool needUpdateTechUI;

        enum tech_type{//科技类型
            TECH_BUILD,//关系到能使用哪些建筑
            TECH_MAKE,//合成道具
            TECH_NONE//无用但必要的科技树节点
        };

        struct tech_node{
                tech_type type;
                bool unlocked;//是否解锁此科技
                std::vector<std::shared_ptr<tech_node> > children;//子节点
                int techId;//科技的id
                int relatedId;//相关的id
                std::string relatedName;//相关的名称(uuid等)
                int iconId;//图标的id
                irr::video::ITexture * description;
        };
        std::map<int,std::shared_ptr<tech_node> > tech_index;//所有可查看的科技
        int techTarget;
        std::shared_ptr<tech_node> tech_root;//根节点
        void addTech(int techId, int parentId, tech_type type, int relatedId, const std::string & relatedName, int iconId,irr::video::ITexture * description);//添加科技
        void unlockTech(int techId);

        void loop()override;

        void loadTechConfig();

        void selectTech(irr::gui::IGUITreeViewNode *);
        void techActive();

        irr::gui::IGUIImage * tech_status;
        irr::gui::IGUIButton * tech_button;

        void msg_techTarget(bool newTarget, int32_t target)override;
        void msg_unlockTech(bool newtech,int32_t id)override;
        void msg_makeStatus(int32_t id,bool status)override;

        virtual void make(int id)=0;

    private:

        void drawTechNode(std::shared_ptr<tech_node> node, gui::IGUITreeViewNode * uinode);

        irr::gui::IGUIImage * tech_active_status;
        irr::s32 tech_active_time;
        inline void showTechStatus(irr::video::ITexture * tex){
            if(tex==NULL)
                return;
            if(tech_active_status)
                tech_active_status->remove();
            tech_active_time = timer->getTime();
            int cx = width/2;
            int cy = height/2;
            cx -= (tex->getSize().Width)/2;
            cy+=(cy/2);
            tech_active_status = gui->addImage(tex,irr::core::position2di(cx,cy));
        }

        irr::video::ITexture
            * tech_active_success,
            * tech_active_fail,
            * tech_active_unlock,
            * tech_active_target,
            * tech_status_background;
};

}

#endif // TECHNOLOGY
