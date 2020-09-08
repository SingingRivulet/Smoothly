#include "shadow.h"

namespace smoothly{

shadow::shadow(){
    shadowCallback.parent = this;
    shadowMapShader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(//创建着色器
                          "../shader/shadow.vs.glsl", "main", irr::video::EVST_VS_1_1,
                          "../shader/shadow.ps.glsl", "main", irr::video::EPST_PS_1_1,
                          &shadowCallback);
    shadowSpace = scene->createNewSceneManager(false);//创建光影空间
    shadowMapLight = shadowSpace->addCameraSceneNode();//创建光源
    shadowMapLight->setProjectionMatrix(core::matrix4().buildProjectionMatrixOrthoLH(256,256,0.9f,400.f),true);
    shadowMapTexture = driver->addRenderTargetTexture(core::dimension2d<u32>(shadowMapSize, shadowMapSize), "shadowMap", video::ECF_R32F);//创建渲染目标
    shadowFactor = 0.3;
    defaultCallback.parent = this;
}

void shadow::renderShadow(){
    driver->setRenderTarget(shadowMapTexture,true,true,irr::video::SColor(255,255,255,255));

    auto cam = camera->getPosition();

    shadowMapLight->setPosition(lightDir*100+cam);
    shadowMapLight->setTarget(cam);
    shadowMapLight->updateAbsolutePosition();

    //position = projection * view * pixel
    //透视矩阵×视图矩阵×像素点坐标
    shadowMatrix = shadowMapLight->getProjectionMatrix() * shadowMapLight->getViewMatrix();

    driver->beginScene(true, true, irr::video::SColor(255,255,0,0));
    shadowSpace->drawAll();
}

scene::IMeshSceneNode *shadow::createShadowNode(
        irr::scene::IMesh * mesh,
        irr::scene::ISceneNode *  	parent,
        s32  	id,
        const irr::core::vector3df &  	position,
        const irr::core::vector3df &  	rotation,
        const irr::core::vector3df &  	scale){
    auto res = shadowSpace->addMeshSceneNode(mesh,parent,id,position,rotation,scale);
    res->setMaterialType((irr::video::E_MATERIAL_TYPE)shadowMapShader);
    return res;
}

void shadow::ShadowCallback::OnSetConstants(video::IMaterialRendererServices * services, s32){
    services->setVertexShaderConstant(services->getVertexShaderConstantID("shadowMatrix") , parent->shadowMatrix.pointer() , 16);
    core::matrix4 world = parent->driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("modelMatrix") , world.pointer() , 16);
}

void shadow::DefaultCallback::OnSetConstants(video::IMaterialRendererServices * services, s32 userData){

    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipY"),&parent->clipY, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipYUp"),&parent->clipYUp, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("enableClipY"),&parent->enableClipY, 1);

    s32 var0 = 0;
    s32 var1 = 1;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMap"),&var1, 1);

    irr::f32 sas = parent->scan_animation_showing;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_showing"),&sas, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_size"),&parent->scan_animation_size, 1);

    auto campos = parent->camera->getPosition();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("campos"),&campos.X, 3);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowFactor"),&parent->shadowFactor, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("tex"),&var0, 1);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("shadowMatrix") , parent->shadowMatrix.pointer() , 16);
    core::matrix4 world = parent->driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("modelMatrix") , world.pointer() , 16);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("transformMatrix") , (parent->camera->getProjectionMatrix()*parent->camera->getViewMatrix()).pointer() , 16);
}

}
