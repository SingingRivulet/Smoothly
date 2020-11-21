#ifndef SMOOTHLY_SHADOW
#define SMOOTHLY_SHADOW

#include "cloud.h"
#include "../utils/mempool.h"

namespace smoothly{

class shadow:public cloud{
    public:

        irr::scene::ISceneManager    * shadowSpace;             //光影使用单独的场景
        irr::scene::ICameraSceneNode * shadowMapLight;          //光源
        irr::video::ITexture         * shadowMapTexture;        //渲染目标
        irr::s32                       shadowMapShader;         //光影着色器
        shadow();

        void renderShadow()override;

        class ShadowCallback:public irr::video::IShaderConstantSetCallBack{//shader回调
            public:
                shadow * parent;
                void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
        }shadowCallback;

        irr::scene::IMeshSceneNode * createShadowNode(
                irr::scene::IMesh * mesh,
                irr::scene::ISceneNode *  	parent = 0,
                s32  	id = -1,
                const irr::core::vector3df &  	position = irr::core::vector3df(0, 0, 0),
                const irr::core::vector3df &  	rotation = irr::core::vector3df(0, 0, 0),
                const irr::core::vector3df &  	scale    = irr::core::vector3df(1.0f, 1.0f, 1.0f));//创建一个光影节点

        class DefaultCallback:public irr::video::IShaderConstantSetCallBack{
            public:
                shadow * parent;
                void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
        }defaultCallback;

};

}

#endif // SMOOTHLY_SHADOW
