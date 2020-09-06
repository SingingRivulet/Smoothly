#ifndef SMOOTHLY_LOCALLIGHT
#define SMOOTHLY_LOCALLIGHT
#include "../utils/dbvt3d.h"
#include <set>

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
                long processed;
        };
        std::set<lightSource*> sources;
        long processId;

        irr::scene::ISceneManager * sceneSpace;
        lightSource * addLight(const irr::core::vector3df & position,irr::f32 range,const irr::video::SColorf & color = irr::video::SColorf(1.0,1.0,1.0,1.0));
        void releaseLight(lightSource * light);

        void updateLight(const irr::scene::ICameraSceneNode * camera, std::function<void(localLight::lightSource*)> callback,
                         std::function<void(irr::f32 x1,irr::f32 y1,irr::f32 x2,irr::f32 y2,localLight::lightSource*)> callback2);

        bool pos3dToScreen(const irr::core::vector3df & pos3d, const irr::scene::ICameraSceneNode * camera,irr::f32 &x,irr::f32 &y);
        bool getRadiusInScreen(const irr::scene::ICameraSceneNode * camera, const irr::core::vector3df & ori, irr::f32 r, irr::f32 & x, irr::f32 & y, irr::f32 & scr);
        void getLightArea(const irr::core::vector3df & pos3d, const irr::scene::ICameraSceneNode * camera, irr::f32 r, std::function<void(irr::f32 x1,irr::f32 y1,irr::f32 x2,irr::f32 y2)> callback);
};

}

#endif // SMOOTHLY_LOCALLIGHT
