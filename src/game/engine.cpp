#include "engine.h"
namespace smoothly{
engine::engine(){
    deltaTimeUpdateFirst = true;
    deltaTime=0;
    lastTime=0;
    device = irr::createDevice(
        irr::video::EDT_OPENGL,
        irr::core::dimension2d<irr::u32>(1024,768)
    );
    driver = device->getVideoDriver();
    scene  = device->getSceneManager();
    gui    = device->getGUIEnvironment();
    timer  = device->getTimer();
    device->setWindowCaption(L"Smoothly");
    scene->setAmbientLight(irr::video::SColor(255,128,128,128));
    scene->setShadowColor(irr::video::SColor(150, 0, 0, 0));
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
    overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    this->dynamicsWorld->setGravity(btVector3(0, -10, 0));
    camera=scene->addCameraSceneNodeFPS();
    scene->addSkyBoxSceneNode(
            driver->getTexture("../../res/skybox/top.jpg"),
            driver->getTexture("../../res/skybox/bottom.jpg"),
            driver->getTexture("../../res/skybox/left.jpg"),
            driver->getTexture("../../res/skybox/right.jpg"),
            driver->getTexture("../../res/skybox/front.jpg"),
            driver->getTexture("../../res/skybox/back.jpg")
        );
    device->getCursorControl()->setVisible(false);

    auto light = scene->addLightSceneNode();
    light->setPosition(irr::core::vector3df(0,500,0));
}
engine::~engine(){
    device->drop();
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
}
void engine::sceneLoop(){
    if(!driver)
        return;
    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    scene->drawAll();
    gui->drawAll();
    driver->endScene();
}
void engine::worldLoop(){
    if(dynamicsWorld && deltaTime!=0.0f){
        //printf("[world]stepSimulation\n");
        dynamicsWorld->stepSimulation(deltaTime,10);
        //dynamicsWorld->stepSimulation(1.f / 60.f,10);
        //collision
        auto disp=dynamicsWorld->getDispatcher();
        int numManifolds = disp->getNumManifolds();
        for(int i=0;i<numManifolds;i++){
            btPersistentManifold * contactManifold = disp->getManifoldByIndexInternal(i);
            int numContacts= contactManifold->getNumContacts();
            if(numContacts>0){
                //const btCollisionObject * obA = contactManifold->getBody0();
                //const btCollisionObject * obB = contactManifold->getBody1();
                
                onCollision(contactManifold);
                
                //int numContacts=contactManifold->getNumContacts();
                //for(int j=0;j<numContacts;j++){
                    
                //    auto point=contactManifold->getContactPoint(j);
                    
                //    //float impulse=point.getAppliedImpulse();
                    
                //    onCollision(obA,obB,point);
                    
                //}
            }
        }
    }
}
void engine::deltaTimeUpdate(){
    if(deltaTimeUpdateFirst){
        deltaTimeUpdateFirst=false;
        lastTime=((float)timer->getTime())/1000.0f;
        deltaTime=0;
        return;
    }
    float tt=((float)timer->getTime())/1000.0f;
    deltaTime=tt-lastTime;
}

}
