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
    t.generator.seed=123456;
    t.pointNum=128;
    t.altitudeK=80;
    t.hillK=20;
    t.temperatureK=30;
    t.humidityK=30;
    t.altitudeArg=2000;
    t.hillArg=200;
    t.temperatureArg=2000;
    t.humidityArg=2000;
    t.texture=;

    //node->setMaterialFlag(video::EMF_LIGHTING, false);

    // add a first person shooter style user controlled camera
    scenemgr->addCameraSceneNodeFPS();

    // draw everything
    t.visualChunkUpdate(0,0,true);
    while(device->run() && driver){
        driver->beginScene(true, true, video::SColor(255,0,0,255));
        scenemgr->drawAll();
        driver->endScene();
    }

    // delete device
    device->drop();
    return 0;
}

