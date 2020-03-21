#include "cloud.h"

namespace smoothly{

cloud::cloud(){
    cloudThre               = 0.5;
    cloudy                  = 0.5;
    lightness               = 1.2;
    astrAtomScat            = 0.5;
    astronomical.set(1,1,1);
    astrLight.set(0.5,0.5,0.5);
    astrColor.set(3.0,1.8,1.6);
    astrTheta               = 0.05;
    cloudShaderCallback.parent = this;
    cloudTime = time(0);
    auto cloudShader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                    "../shader/cloud.vs.glsl","main", irr::video::EVST_VS_1_1,
                    "../shader/cloud.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &cloudShaderCallback);
    auto skyShader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/sky.vs.glsl","main", irr::video::EVST_VS_1_1,
                "../shader/sky.ps.glsl", "main", irr::video::EPST_PS_1_1);

    sky_1.device = device;
    sky_1.driver = driver;
    sky_1.scene  = scene;
    sky_1.timer  = timer;
    sky_1.init("sky1",cloudShader,skyShader);
    sky_2.device = device;
    sky_2.driver = driver;
    sky_2.scene  = scene;
    sky_2.timer  = timer;
    sky_2.init("sky2",cloudShader,skyShader);
    sky_2.box->setVisible(false);
    sky_p  = &sky_1;
    sky_pb = &sky_2;
}

void cloud::renderSky(){
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
        return;
    }
    ends=clock();
    if(ends-starts<30)
        goto begin;
}

void cloud::skyBox::init(const std::string & name,irr::s32 cloud,irr::s32 /*sky*/){
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

    //box->setMaterialType((irr::video::E_MATERIAL_TYPE)sky);
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
