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
        std::shared_ptr<tech_node> tech_root;//根节点
        void addTech(int techId, int parentId, tech_type type, int relatedId, const std::string & relatedName, int iconId,irr::video::ITexture * description);//添加科技
        void unlockTech(int techId);

        void loop()override;

        void loadTechConfig();

        void selectTech(irr::gui::IGUITreeViewNode *);

        irr::gui::IGUIImage * tech_status;
        irr::gui::IGUIButton * tech_button;

    private:

        void drawTechNode(std::shared_ptr<tech_node> node, gui::IGUITreeViewNode * uinode);
};

}

#endif // TECHNOLOGY
