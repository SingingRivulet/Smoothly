#ifndef SMOOTHLY_CARTO
#define SMOOTHLY_CARTO
#include "engine.h"

namespace smoothly{

class carto:public engine{
    public:
        carto();
        irr::video::ITexture * minimap;//小地图
        void updateMiniMap();
        irr::scene::ISceneManager * mapScene;//用于地图的抽象场景
        irr::scene::ICameraSceneNode * mapCamera;
        void renderMiniMap()override;
};

}

#endif // SMOOTHLY_CARTO
