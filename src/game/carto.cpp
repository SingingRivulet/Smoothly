#include "carto.h"

namespace smoothly{

carto::carto(){
    mapScene = scene->createNewSceneManager();
    mapCamera = mapScene->addCameraSceneNode();
    mapCamera->setUpVector(irr::core::vector3df(0,0,1));
    minimap = driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(128,128),"minimap",irr::video::ECF_A8R8G8B8);
    gui->addImage(minimap,irr::core::vector2di(width-150,height-192),true);
}


void carto::renderMiniMap(){
    auto cmpos = camera->getPosition();
    //设置小地图显示方向
    auto dir    = camera->getTarget()-cmpos;
    dir.Y = 0;
    if(dir.X!=0 || dir.Z!=0){
        dir.normalize();
        mapCamera->setUpVector(dir);
    }
    //设置小地图摄像机
    cmpos.Y=128;
    mapCamera->setPosition(cmpos);
    cmpos.Y=0;
    mapCamera->setTarget(cmpos);

    driver->setRenderTarget(minimap,true,true,irr::video::SColor(0,0,0,0));
    driver->beginScene(true, true, irr::video::SColor(0,0,0,0));
    mapScene->drawAll();
    driver->setRenderTarget(0);
}

}
