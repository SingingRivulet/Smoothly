#include "localLight.h"

namespace smoothly{

localLight::localLight(){
}

localLight::lightSource *localLight::addLight(const irr::core::vector3df & position, irr::f32 range, const irr::video::SColorf & color){
    auto p = new lightSource;
    p->position = position;
    p->range = range;
    p->color = color;
    p->node = lightMap.add(dbvt3d::vec3(position.X-range,position.Y-range,position.Z-range),
                           dbvt3d::vec3(position.X+range,position.Y+range,position.Z+range),
                           p);
    return p;
}

void localLight::releaseLight(localLight::lightSource * light){
    light->node->autodrop();
    delete light;
}

void localLight::updateLight(const irr::core::vector3df & cam, std::function<void(localLight::lightSource*)> callback){
    struct arg_t{
            localLight * self;
            std::function<void(localLight::lightSource*)> callback;
    }arg;
    arg.self = this;
    arg.callback = callback;
    lightMap.fetchByPoint(cam,[](dbvt3d::AABB * box,void * a){
        auto arg = (arg_t*)a;
        if(box->data){
            auto s = (localLight::lightSource*)box->data;
            arg->callback(s);
        }
    },&arg);
}

}
