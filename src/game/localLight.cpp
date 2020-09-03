#include "localLight.h"

namespace smoothly{

localLight::localLight(){
    needUpdateLight = true;
}

void localLight::setTexture(irr::video::ITexture * tex){
    if(tex->getColorFormat()==irr::video::ECF_A32B32G32R32F){
        //+0红 +1绿 +2蓝 +3alpha
        auto data = (irr::f32*)tex->lock();
        lightMap.onMakeID = [&](dbvt3d::AABB * box){
            if(box->id<65535 && box->data){//512*512的纹理最多容纳65535个节点
                auto ptr = (lightSource*)box->data;
                int i = box->id * 16;//一个节点有4个vec4向量

                //传box
                auto & from = box->from;
                auto & to   = box->to;
                data[i+0+0] = from.X;
                data[i+0+1] = from.Y;
                data[i+0+2] = from.Z;
                data[i+4+0] = to.X;
                data[i+4+1] = to.Y;
                data[i+4+2] = to.Z;

                //传左节点
                if(box->left)
                    data[i+3] = box->left->id;
                else
                    data[i+3] = 0.f;
                //传右节点
                if(box->right)
                    data[i+7] = box->right->id;
                else
                    data[i+7] = 0.f;

                //传物体
                data[i+8+0] = ptr->position.X;
                data[i+8+1] = ptr->position.Y;
                data[i+8+2] = ptr->position.Z;
                data[i+8+3] = ptr->range;

                //传颜色
                data[i+12+0] = ptr->color.r;
                data[i+12+1] = ptr->color.g;
                data[i+12+2] = ptr->color.b;
                data[i+12+3] = ptr->color.a;

            }
        };
        lightMap.makeID();
        tex->unlock();
    }
}

localLight::lightSource *localLight::addLight(const irr::core::vector3df & position, irr::f32 range, const irr::video::SColorf & color){
    auto p = new lightSource;
    p->position = position;
    p->range = range;
    p->color = color;
    p->node = lightMap.add(dbvt3d::vec3(position.X-range,position.Y-range,position.Z-range),
                           dbvt3d::vec3(position.X+range,position.Y+range,position.Z+range),
                           p);
    //setTexture();//更新gpu端
    needUpdateLight = true;
    return p;
}

void localLight::releaseLight(localLight::lightSource * light){
    light->node->autodrop();
    delete light;
    //setTexture();
    needUpdateLight = true;
}

}
