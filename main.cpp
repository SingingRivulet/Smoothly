#include "terrain.h"
using namespace irr;
int main(){
    // start up the engine
    IrrlichtDevice * device = createDevice(video::EDT_OPENGL,
        core::dimension2d<u32>(640,480));

    video::IVideoDriver * driver = device->getVideoDriver();
    scene::ISceneManager * scenemgr = device->getSceneManager();

    device->setWindowCaption(L"Hello World!");

    smoothly::terrain t;
    smoothly::mods m;
    t.m=&m;
    t.device=device;
    t.scene=scenemgr;
    t.generator.seed=1234506;
    t.pointNum=129;
    t.altitudeK=0.8;
    t.hillK=0.2;
    t.temperatureK=0.3;
    t.humidityK=0.3;
    t.altitudeArg=20000;
    t.hillArg=200;
    t.temperatureArg=2000;
    t.humidityArg=2000;

    //node->setMaterialFlag(video::EMF_LIGHTING, false);

    // add a first person shooter style user controlled camera
    auto camera=scenemgr->addCameraSceneNodeFPS();
    camera->setPosition(core::vector3df(0,1190,0));
    
    //auto sph=scenemgr->addSphereSceneNode();
    //sph->setPosition(core::vector3df(0,10,2000));
    //sph->setMaterialFlag(video::EMF_LIGHTING, false );

    // draw everything
    t.genTexture();
    
    t.visualChunkUpdate(0,0,true);
    while(device->run() && driver){
        driver->beginScene(true, true, video::SColor(255,0,0,0));
        scenemgr->drawAll();
        driver->endScene();
    }
    //t.destroyTexture();
    t.destroy();

    // delete device
    device->drop();
    return 0;
}

