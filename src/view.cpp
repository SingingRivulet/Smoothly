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
    setSeed(123456);
    device = irr::createDevice(
        irr::video::EDT_OPENGL,
        irr::core::dimension2d<irr::u32>(m->windowWidth,m->windowHeight)
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
    scene->setShadowColor(irr::video::SColor(150, 0, 0, 0));
    
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
    overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
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
    if(dynamicsWorld && deltaTime!=0.0f){
        //printf("[world]stepSimulation\n");
        dynamicsWorld->stepSimulation(deltaTime*m->booster,10);
        //dynamicsWorld->stepSimulation(1.f / 60.f,10);
        //collision
        auto disp=dynamicsWorld->getDispatcher();
        int numManifolds = disp->getNumManifolds();
        for(int i=0;i<numManifolds;i++){
            btPersistentManifold * contactManifold = disp->getManifoldByIndexInternal(i);
            int numContacts= contactManifold->getNumContacts();
            if(numContacts>0){
                const btCollisionObject * obA = contactManifold->getBody0();
                const btCollisionObject * obB = contactManifold->getBody1();
                
                auto pA=(bodyInfo*)obA->getUserPointer();
                auto pB=(bodyInfo*)obB->getUserPointer();
                
                if(pA==NULL || pB==NULL)
                    continue;
                
                int numContacts=contactManifold->getNumContacts();
                for(int j=0;j<numContacts;j++){
                    
                    auto point=contactManifold->getContactPoint(j);
                    
                    //float impulse=point.getAppliedImpulse();
                    
                    onCollision(pA,pB,point);
                    
                }
            }
        }
    }
    subsUpdate();
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