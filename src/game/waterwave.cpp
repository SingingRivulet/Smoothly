#include "waterwave.h"

namespace smoothly{

waterwave::waterwave(irr::video::IVideoDriver * d){
    driver = d;
    waveMap = d->addRenderTargetTexture(irr::core::dimension2du(2048,2048) , "waterWave_1",irr::video::ECF_R32F);
    waveMap_last = d->addRenderTargetTexture(irr::core::dimension2du(2048,2048) , "waterWave_2",irr::video::ECF_R32F);
    waveNormal = d->addRenderTargetTexture(irr::core::dimension2du(2048,2048) , "waterNormal",irr::video::ECF_A16B16G16R16F);

    waveMaterial.ZBuffer = irr::video::ECFN_DISABLED;
    waveMaterial.MaterialType = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                "../shader/waterWave.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                "../shader/waterWave.ps.glsl", "main", irr::video::EPST_PS_1_1,
                                this);
    normalMaterial.ZBuffer = irr::video::ECFN_DISABLED;
    normalMaterial.MaterialType = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                "../shader/waterWave.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                "../shader/waterWave_normal.ps.glsl", "main", irr::video::EPST_PS_1_1,
                                this);
}

void waterwave::update(const irr::core::vector2df & pos){
    //配置材质
    lastPos = nowPos;
    nowPos.set(floor(pos.X) , floor(pos.Y));
    swapTex();

    //计算波（GPU端）
    waveMaterial.setTexture(0,waveMap_last);
    driver->setRenderTarget(waveMap,true,true);
    driver->setMaterial(waveMaterial);
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1, 1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df(-1, 1,1)),
                           irr::video::SColor(0,0,0,0));
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1,-1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df( 1, 1,1)),
                           irr::video::SColor(0,0,0,0));

    auto ptr = (irr::f32*)waveMap->lock();
    for(int i=0;i<10;++i)
        ptr[rand()%2048 + (rand()%2048)*2048] = 10;
    waveMap->unlock();
    //driver->drawPixel(rand()%256 , rand()%256 , irr::video::SColorf(10,0,0));

    //计算法线（GPU端）
    normalMaterial.setTexture(0,waveMap);
    driver->setRenderTarget(waveNormal,true,true);
    driver->setMaterial(normalMaterial);
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1, 1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df(-1, 1,1)),
                           irr::video::SColor(0,0,0,0));
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1,-1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df( 1, 1,1)),
                           irr::video::SColor(0,0,0,0));

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
