#include "localLight.h"

namespace smoothly{

localLight::localLight(){
    processId = 0;
}

localLight::lightSource *localLight::addLight(const irr::core::vector3df & position, irr::f32 range, const irr::video::SColorf & color){
    auto p = new lightSource;
    p->position = position;
    p->range = range;
    p->color = color;
    p->node = lightMap.add(dbvt3d::vec3(position.X-range,position.Y-range,position.Z-range),
                           dbvt3d::vec3(position.X+range,position.Y+range,position.Z+range),
                           p);
    p->processed=processId;
    sources.insert(p);
    return p;
}

void localLight::releaseLight(localLight::lightSource * light){
    sources.erase(light);
    light->node->autodrop();
    delete light;
}

void localLight::updateLight(const irr::scene::ICameraSceneNode * camera, std::function<void(localLight::lightSource*)> callback,
                             std::function<void(irr::f32 x1,irr::f32 y1,irr::f32 x2,irr::f32 y2,localLight::lightSource*)> callback2){
    irr::core::vector3df cam = camera->getPosition();
    ++processId;
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
            s->processed = arg->self->processId;
        }
    },&arg);
    for(auto it:sources){
        if(it->processed!=processId){
            it->processed = processId;
            getLightArea(it->position,camera,it->range,[&](irr::f32 x1,irr::f32 y1,irr::f32 x2,irr::f32 y2){
                callback2(x1,y1,x2,y2,it);
            });
        }
    }
}

bool localLight::pos3dToScreen(const irr::core::vector3df & pos3d, const irr::scene::ICameraSceneNode * camera,irr::f32 &x,irr::f32 &y){
    irr::core::matrix4 trans = camera->getProjectionMatrix();
    trans *= camera->getViewMatrix();

    irr::f32 transformedPos[4] = { pos3d.X, pos3d.Y, pos3d.Z, 1.0f };

    trans.multiplyWith1x4Matrix(transformedPos);

    if (transformedPos[3] < 0)
        return false;

    const irr::f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
                                                 irr::core::reciprocal(transformedPos[3]);

    x = transformedPos[0] * zDiv;
    y = transformedPos[1] * zDiv;
    return true;
}

bool localLight::getRadiusInScreen(const irr::scene::ICameraSceneNode * camera, const irr::core::vector3df & ori, irr::f32 r,irr::f32 & x,irr::f32 & y, irr::f32 & scr){
    auto cam = camera->getPosition();
    irr::core::vector3df dir = ori - cam;
    auto c = dir.crossProduct(irr::core::vector3df(0,1,0));//水平方向
    c.normalize();
    c*=r;
    auto u = dir.crossProduct(c);//垂直方向
    u.normalize();
    u*=r;

    auto p = ori+c;
    auto q = ori+u;

    irr::f32 x1,y1,x2,y2,x3,y3;
    if(!pos3dToScreen(ori,camera,x1,y1))
        return false;
    if(!pos3dToScreen(p  ,camera,x2,y2))
        return false;
    if(!pos3dToScreen(q  ,camera,x3,y3))
        return false;
    x=x1;
    y=y1;

    float s1 = fabs(x2-x1);
    float s2 = fabs(y3-y1);

    scr = fmax(s1,s2);
    return true;
}

void localLight::getLightArea(const irr::core::vector3df & pos3d, const irr::scene::ICameraSceneNode * camera,irr::f32 r, std::function<void (irr::f32, irr::f32, irr::f32, irr::f32)> callback){
    irr::f32 x , y , scr , x1 , x2 , y1 , y2;
    if(getRadiusInScreen(camera,pos3d,r,x,y,scr)){
        auto driver = camera->getSceneManager()->getVideoDriver();
        auto d = ((float)driver->getViewPort().getWidth())/((float)driver->getViewPort().getHeight());
        x1 = x-scr*1.2;
        x2 = x+scr*1.2;
        y1 = y-scr*d;
        y2 = y+scr*d;

        if(x1<-1.0)
            x1 = -1.0;
        if(y1<-1.0)
            y1 = -1.0;

        if(x2>1.0)
            x2 = 1.0;
        if(y2>1.0)
            y2 = 1.0;

        if(x1==x2 || y1==y2)//超出范围
            return;

        callback(x1,y1,x2,y2);
    }
}

}
