#include "waterwave.h"

namespace smoothly{

waterwave::waterwave(irr::video::IVideoDriver * d){
    driver = d;
    waveMap = d->addRenderTargetTexture(irr::core::dimension2du(2048,2048) , "waterWave_1",irr::video::ECF_R32F);
    waveMap_last = d->addRenderTargetTexture(irr::core::dimension2du(2048,2048) , "waterWave_2",irr::video::ECF_R32F);
    material.ZBuffer = irr::video::ECFN_DISABLED;
    material.MaterialType = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                "../shader/waterWave.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                "../shader/waterWave.ps.glsl", "main", irr::video::EPST_PS_1_1,
                                this);
}

void waterwave::update(const irr::core::vector2df & pos){
    //配置材质
    lastPos = nowPos;
    nowPos.set(floor(pos.X) , floor(pos.Y));
    swapTex();
    material.setTexture(0,waveMap_last);
    driver->setRenderTarget(waveMap,true,true);
    driver->setMaterial(material);
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1, 1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df(-1, 1,1)),
                           irr::video::SColor(0,0,0,0));
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1,-1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df( 1, 1,1)),
                           irr::video::SColor(0,0,0,0));

    driver->drawPixel(rand()%2048 , rand()%2048 , irr::video::SColor(255,128,128,128));

    driver->setRenderTarget(0);
}

void waterwave::registerShaderVar(irr::video::IMaterialRendererServices * services){
    services->setPixelShaderConstant(services->getPixelShaderConstantID("lastPos"),&lastPos.X, 2);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("nowPos"),&nowPos.X, 2);
}

void waterwave::OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 ){
    services->setPixelShaderConstant(services->getPixelShaderConstantID("lastPos"),&lastPos.X, 2);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("nowPos"),&nowPos.X, 2);
    irr::s32 var0 = 0;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("waveMap"),&var0, 1);
}

void waterwave::swapTex(){
    auto tmp = waveMap_last;
    waveMap_last = waveMap;
    waveMap = tmp;
}

}
