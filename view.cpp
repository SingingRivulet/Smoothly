#include "view.h"
namespace smoothly{

static void addSkyBox(irr::scene::ISceneManager * scene,irr::video::IVideoDriver * driver){
    scene->addSkyBoxSceneNode(
        driver->getTexture("./res/skybox/top.jpg"),
        driver->getTexture("./res/skybox/bottom.jpg"),
        driver->getTexture("./res/skybox/left.jpg"),
        driver->getTexture("./res/skybox/right.jpg"),
        driver->getTexture("./res/skybox/front.jpg"),
        driver->getTexture("./res/skybox/back.jpg")
    );
}

void view::loadScene(){
    device = irr::createDevice(
        irr::video::EDT_OPENGL,
        irr::core::dimension2d<irr::u32>(1024,768)
    );
    driver = device->getVideoDriver();
    scene = device->getSceneManager();
    timer = device->getTimer();
    device->setWindowCaption(L"Smoothly");
    
    driver->setFog(
        irr::video::SColor(0,255,255,255),
        irr::video::EFT_FOG_LINEAR,
        100,500,
        true,
        false
    );
    
    //m=new mods;
    genTexture();
    visualChunkUpdate(0,0,true);
    //updateBuildingChunks(0,0,3);
    
    scene->setAmbientLight(irr::video::SColor(255,128,128,128));
    
    auto light=scene->addLightSceneNode();
    irr::video::SLight liconf;
    liconf.DiffuseColor=irr::video::SColor(255,192,192,192);
    liconf.SpecularColor=irr::video::SColor(255,230,230,230);
    light->setLightData(liconf);
    light->setPosition(irr::core::vector3df(0,2000,0));
    addSkyBox(scene,driver);
}
void view::destroyScene(){
    device->drop();
    //delete m;
}
void view::loadWorld(){
    printf("dynamicsWorld init\n");
    this->collisionConfiguration = new btDefaultCollisionConfiguration();
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);
    this->overlappingPairCache = new btDbvtBroadphase();
    this->solver = new btSequentialImpulseConstraintSolver;
    this->dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher,
        overlappingPairCache,
        solver,
        collisionConfiguration
    );
    this->dynamicsWorld->setGravity(btVector3(0, -10, 0));
}
void view::destroyWorld(){
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
}
void view::sceneLoop(){
    if(!driver)
        return;
    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    scene->drawAll();
    driver->endScene();
}
void view::worldLoop(){
    if(dynamicsWorld && deltaTime!=0.0f)
        dynamicsWorld->stepSimulation(deltaTime,10);
}
void view::deltaTimeUpdate(){
    if(deltaTimeUpdateFirst){
        deltaTimeUpdateFirst=false;
        lastTime=((float)timer->getTime())/1000.0f;
        deltaTime=0;
        return;
    }
    float tt=((float)timer->getTime())/1000.0f;
    deltaTime=tt-lastTime;
}
view::view(){
    deltaTimeUpdateFirst=true;
}
view::~view(){
    
}

}