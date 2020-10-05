#ifndef SMOOTHLY_CLOUD
#define SMOOTHLY_CLOUD
#include "carto.h"
namespace smoothly{

class cloud:public carto{
    public:
        cloud();
        irr::f32 cloudThre, // 云量，越大云越少
                 cloudy,    // 阴天
                 lightness; // 亮度
        irr::core::vector3df astronomical,//发光天体位置
                             astrLight,   //亮度
                             astrColor,   //天体颜色
                             glow;
        float                astrAtomScat,//大气层散射
                             astrTheta;   //天体在视野中夹角
        void setSnow(float k);
        void setRain(float k);
        void renderSky()override;
        virtual void updateWeather(int)=0;
    private:
        struct skyBox{
            irr::video::IVideoDriver  * driver;
            irr::scene::ISceneManager * scene;
            irr::video::SMaterial cloudMaterial;
            irr::video::ITexture * cloudTop;
            irr::video::ITexture * cloudFront;
            irr::video::ITexture * cloudBack;
            irr::video::ITexture * cloudLeft;
            irr::video::ITexture * cloudRight;
            irr::video::ITexture * cloudBottom;
            irr::scene::ISceneNode * box;
            void(*callback[81])(skyBox *);
            int count;
            bool first;
            void init(const std::string &name, irr::s32 cloud, video::ITexture * sky_cubemap);
            bool process();
        };
        skyBox * sky_p , * sky_pb;
        irr::scene::ISceneNode * skySpace_p , * skySpace_pb;
        skyBox sky_1,sky_2;
        class CloudShaderCallback:public irr::video::IShaderConstantSetCallBack{//shader回调
            public:
                cloud * parent;
                void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
        }cloudShaderCallback;
        irr::s32 cloudTime;
        irr::scene::IParticleSystemSceneNode *rain,*snow;

        irr::gui::IGUIStaticText * cloudy_text;
        irr::gui::IGUIStaticText * daytime_text;
        irr::gui::IGUIStaticText * gametime_text;

    public:
        irr::scene::ILightSceneNode * light;
        irr::core::vector3df lightTarget;
        irr::scene::ISceneManager * skySpace;
        irr::scene::ICameraSceneNode * skySpace_camera;
};

}
#endif // SMOOTHLY_ENGINE
