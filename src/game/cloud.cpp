#include "cloud.h"

namespace smoothly{

cloud::cloud(){
    cloudThre               = 0.5;
    cloudy                  = 0.5;
    lightness               = 1.2;
    astrAtomScat            = 0.5;
    cloudy_text   = NULL;
    daytime_text  = NULL;
    gametime_text = NULL;
    astronomical.set(1,1,1);
    astrLight.set(0.5,0.5,0.5);
    astrColor.set(3.0,2.8,2.6);
    astrTheta               = 0.025;
    cloudShaderCallback.parent = this;
    cloudTime = time(0);
    auto cloudShader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                    "../shader/cloud.vs.glsl","main", irr::video::EVST_VS_1_1,
                    "../shader/cloud.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &cloudShaderCallback);
    sky_1.driver = driver;
    sky_1.scene  = scene;
    sky_1.init("sky1",cloudShader);
    sky_2.driver = driver;
    sky_2.scene  = scene;
    sky_2.init("sky2",cloudShader);
    sky_2.box->setVisible(false);
    sky_p  = &sky_1;
    sky_pb = &sky_2;

    snow = scene->addParticleSystemSceneNode(false);
    snow->setMaterialTexture(0,driver->getTexture("../../res/snow.png"));
    snow->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    snow->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    snow->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;

    rain = scene->addParticleSystemSceneNode(false);
    rain->setMaterialTexture(0,driver->getTexture("../../res/rain.png"));
    rain->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    rain->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    rain->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;

    scene->setAmbientLight(irr::video::SColor(255,80,80,80));
    light = scene->addLightSceneNode();
    light->setPosition(irr::core::vector3df(0,500,0));

    //setSnow(1);
    //setRain(1);
}

void cloud::setSnow(float k){
    if(k<=0)
        snow->setEmitter(0);
    else{
        auto emt = snow->createBoxEmitter(
                    core::aabbox3df(-1000, 28,-1000, 1000, 30, 1000),
                    core::vector3df(0,-0.5,0),
                    8000*k,8500*k,
                    video::SColor(128,64,64,255),
                    video::SColor(128,64,64,255),
                    2000,2000,45,
                    core::dimension2df(1,1),
                    core::dimension2df(5,5));
        snow->setEmitter(emt);
        emt->drop();
    }
}

void cloud::setRain(float k){
    if(k<=0)
        rain->setEmitter(0);
    else{
        auto emt = rain->createBoxEmitter(
                    core::aabbox3df(-1000, 28,-1000, 1000, 30, 1000),
                    core::vector3df(0,-1,0),
                    8000*k,8500*k,
                    video::SColor(128,64,64,255),
                    video::SColor(128,64,64,255),
                    2000,2000,0,
                    core::dimension2df(2,2),
                    core::dimension2df(2,2));
        rain->setEmitter(emt);
        emt->drop();
    }
}

void cloud::renderSky(){
    rain->setPosition(camera->getPosition()+core::vector3df(0,1000,0));
    snow->setPosition(camera->getPosition()+core::vector3df(0,1000,0));
    clock_t starts,ends;
    starts=clock();
    begin:
    if(sky_pb->process()){
        sky_pb->box->setVisible(true);
        sky_p->box->setVisible(false);
        //交换双缓冲
        auto tmp = sky_p;
        sky_p = sky_pb;
        sky_pb = tmp;
        cloudTime = time(0);//更新时间
        updateWeather(cloudTime);

        return;
    }
    ends=clock();
    if((ends-starts)/(CLOCKS_PER_SEC/1000.0)<0.005)
        goto begin;

    if(gametime_text)
        gametime_text->remove();
    if(daytime_text)
        daytime_text->remove();
    if(cloudy_text)
        cloudy_text->remove();
    wchar_t buf[256];

    swprintf(buf,256,L"Game time:%d",cloudTime);
    gametime_text=gui->addStaticText(buf,irr::core::rect<irr::s32>(0,height-70,200,height),false,false);
    gametime_text->setOverrideColor(irr::video::SColor(255,255,255,255));
    gametime_text->setOverrideFont(font);

    swprintf(buf,256,L"clock:%d",cloudTime%1200);
    daytime_text=gui->addStaticText(buf,irr::core::rect<irr::s32>(200,height-70,300,height),false,false);
    daytime_text->setOverrideColor(irr::video::SColor(255,255,255,255));
    daytime_text->setOverrideFont(font);

    swprintf(buf,256,L"weather:%f",cloudy);
    cloudy_text=gui->addStaticText(buf,irr::core::rect<irr::s32>(300,height-70,400,height),false,false);
    cloudy_text->setOverrideColor(irr::video::SColor(255,255,255,255));
    cloudy_text->setOverrideFont(font);
}

void cloud::skyBox::init(const std::string & name, irr::s32 cloud){
    cloudMaterial.MaterialType = (irr::video::E_MATERIAL_TYPE)cloud;

    //创建天空的渲染目标
    cloudTop   = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), (name+"cloudTop").c_str(), video::ECF_A8R8G8B8);
    cloudLeft  = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), (name+"cloudLeft").c_str(), video::ECF_A8R8G8B8);
    cloudRight = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), (name+"cloudRight").c_str(), video::ECF_A8R8G8B8);
    cloudFront = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), (name+"cloudFront").c_str(), video::ECF_A8R8G8B8);
    cloudBack  = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), (name+"cloudBack").c_str(), video::ECF_A8R8G8B8);
    box        = scene->addSkyBoxSceneNode(
            cloudTop,
            NULL,
            cloudLeft,
            cloudRight,
            cloudFront,
            cloudBack
        );
    box->setMaterialFlag(irr::video::EMF_ANTI_ALIASING,true);
    for(int i=0;i<6;++i){
        auto & m = box->getMaterial(i);
        m.ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
        m.BlendOperation = irr::video::EBO_ADD;
    }

#define processFace(id,tex,col) \
    callback[id*16]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true); \
        self->driver->setMaterial(self->cloudMaterial); \
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,-1  ,1),\
                                                            irr::core::vector3df(-1,-0.5,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+1]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,-0.5,1),\
                                                            irr::core::vector3df(-1,0   ,1),\
                                                            irr::core::vector3df(0 ,0,1)),\
                               col);\
    };\
    callback[id*16+2]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,0  ,1),\
                                                            irr::core::vector3df(-1,0.5,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+3]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,0.5,1),\
                                                            irr::core::vector3df(-1,1  ,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+4]=[](skyBox * self){ \
        self->driver->setRenderTarget(tex,false,true); \
        self->driver->setMaterial(self->cloudMaterial); \
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1  ,1  ,1),\
                                                            irr::core::vector3df(-0.5,1  ,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+5]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-0.5,1,1),\
                                                            irr::core::vector3df(0   ,1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+6]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0  ,1,1),\
                                                            irr::core::vector3df(0.5,1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+7]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0.5,1,1),\
                                                            irr::core::vector3df(1  ,1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+8]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true); \
        self->driver->setMaterial(self->cloudMaterial); \
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,1  ,1),\
                                                            irr::core::vector3df(1,0.5,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+9]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,0.5,1),\
                                                            irr::core::vector3df(1,0  ,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+10]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,0   ,1),\
                                                            irr::core::vector3df(1,-0.5,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+11]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,-0.5,1),\
                                                            irr::core::vector3df(1,-1  ,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+12]=[](skyBox * self){ \
        self->driver->setRenderTarget(tex,false,true); \
        self->driver->setMaterial(self->cloudMaterial); \
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1  ,-1,1),\
                                                            irr::core::vector3df(0.5,-1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+13]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0.5,-1,1),\
                                                            irr::core::vector3df(0,  -1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+14]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0   ,-1,1),\
                                                            irr::core::vector3df(-0.5,-1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };\
    callback[id*16+15]=[](skyBox * self){\
        self->driver->setRenderTarget(tex,false,true);\
        self->driver->setMaterial(self->cloudMaterial);\
        self->driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-0.5,-1,1),\
                                                            irr::core::vector3df(-1  ,-1,1),\
                                                            irr::core::vector3df(0,0,1)),\
                               col);\
    };
    ///////////////////////////////////////////////
    processFace(0,self->cloudTop,irr::video::SColor(0,0,255,0));
    processFace(1,self->cloudFront,irr::video::SColor(0,255,0,0));
    processFace(2,self->cloudBack,irr::video::SColor(0,255,0,255));
    processFace(3,self->cloudLeft,irr::video::SColor(0,0,0,0));
    processFace(4,self->cloudRight,irr::video::SColor(0,0,0,255));
    count = -1;
}

bool cloud::skyBox::process(){
    if(count>=80){
        count = 0;
        return true;
    }else{
        if(count<0){
            //避开第一帧，防止渲染不全
            ++count;
            return false;
        }
        callback[count](this);
        ++count;
        return false;
    }
}

void cloud::CloudShaderCallback::OnSetConstants(irr::video::IMaterialRendererServices *services, irr::s32){
    services->setVertexShaderConstant("time",&parent->cloudTime,1);
    services->setVertexShaderConstant("cloudThre",&parent->cloudThre,1);
    services->setVertexShaderConstant("cloudy",&parent->cloudy,1);
    services->setVertexShaderConstant("lightness",&parent->lightness,1);
    services->setVertexShaderConstant("astrAtomScat",&parent->astrAtomScat,1);
    services->setVertexShaderConstant("astrViewTheta",&parent->astrTheta,1);
    services->setVertexShaderConstant("astronomical",&parent->astronomical.X,3);
    services->setVertexShaderConstant("astrLight",&parent->astrLight.X,3);
    services->setVertexShaderConstant("astrColor",&parent->astrColor.X,3);
}

}
