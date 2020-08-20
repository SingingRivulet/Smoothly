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
    shadowMapLight->setProjectionMatrix(core::matrix4().buildProjectionMatrixOrthoLH(64,64,0.9f,200.f),true);
    shadowMapTexture = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "shadowMap", video::ECF_R32F);//创建渲染目标
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
    services->setVertexShaderConstant("shadowMatrix" , parent->shadowMatrix.pointer() , 16);
    core::matrix4 world = parent->driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant("modelMatrix" , world.pointer() , 16);
}

}
