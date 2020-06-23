#ifndef SMOOTHLY_CARTO
#define SMOOTHLY_CARTO
#include "engine.h"

namespace smoothly{

class carto:public engine{
    public:
        carto();
        irr::video::ITexture * minimap;//小地图
        irr::video::ITexture * fullmap;//大地图
        irr::gui::IGUIImage * fullmap_gui;
        irr::gui::IGUIImage * minimap_gui;
        void updateMiniMap();
        inline void setFullMapMode(bool m){
            fullmap_gui->setVisible(m);
            minimap_gui->setVisible(!m);
        }
        inline void setFullMapMode(){
            auto m = minimap_gui->isVisible();
            setFullMapMode(m);
        }
        irr::scene::ISceneManager * mapScene;//用于地图的抽象场景
        irr::scene::ICameraSceneNode * mapCamera;
        void renderMiniMap()override;
        float mapCamera_height;
};

}

#endif // SMOOTHLY_CARTO
