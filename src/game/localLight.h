#ifndef SMOOTHLY_LOCALLIGHT
#define SMOOTHLY_LOCALLIGHT
#include "../utils/dbvt3d.h"

namespace smoothly{

class localLight{
    public:
        localLight();
        dbvt3d lightMap;
        struct lightSource{
                dbvt3d::AABB * node;
                irr::core::vector3df position;
                irr::f32 range;
                irr::video::SColorf color;
        };
        irr::video::ITexture * gpuBVH;
        lightSource * addLight(const irr::core::vector3df & position,irr::f32 range,const irr::video::SColorf & color = irr::video::SColorf(1.0,1.0,1.0,1.0));
        void releaseLight(lightSource * light);

        void updateLight(const irr::core::vector3df & cam,irr::video::IVideoDriver * driver);
        irr::s32 nearLightNum;
};

}

#endif // SMOOTHLY_LOCALLIGHT
