#ifndef SMOOTHLY_WATERWAVE
#define SMOOTHLY_WATERWAVE
#include <irrlicht.h>

namespace smoothly{

class waterwave:irr::video::IShaderConstantSetCallBack{
    public:
        irr::video::ITexture    * waveMap , *waveMap_last;//纹理
        waterwave(irr::video::IVideoDriver * driver);
        void update(const irr::core::vector2df & pos);

        void registerShaderVar(irr::video::IMaterialRendererServices *);

    private:
        irr::video::IVideoDriver * driver;
        irr::core::vector2df    lastPos ,       //上一帧的位置
                                nowPos;         //现在位置
        irr::video::SMaterial material;
        void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;

        void swapTex();
};

}
#endif // SMOOTHLY_WATERWAVE
