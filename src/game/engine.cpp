#include "engine.h"
namespace smoothly{
engine::engine(){
    deltaTimeUpdateFirst    = true;
    running                 = true;
    deltaTime               = 0;
    lastTime                = 0;
    waterLevel              = 32;
    lastFPS                 = 0;
    width                   = 1024;
    height                  = 768;
    //初始化天空计时器
#define initSkyTimer(v)\
    for(int i=0;i<8;i++)\
        v[i]=0;
    initSkyTimer(lastRenderSkyTop);
    initSkyTimer(lastRenderSkyFront);
    initSkyTimer(lastRenderSkyBack);
    initSkyTimer(lastRenderSkyLeft);
    initSkyTimer(lastRenderSkyRight);
    device = irr::createDevice(
                irr::video::EDT_OPENGL,
                irr::core::dimension2d<irr::u32>(width,height)
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
    cloudMaterial.MaterialType = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/cloud.vs.glsl","main", irr::video::EVST_VS_1_1,
                "../shader/cloud.ps.glsl", "main", irr::video::EPST_PS_1_1);

    //创建天空的渲染目标
    cloudTop   = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "cloudTop", video::ECF_A8R8G8B8);
    cloudLeft  = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "cloudLeft", video::ECF_A8R8G8B8);
    cloudRight = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "cloudRight", video::ECF_A8R8G8B8);
    cloudFront = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "cloudFront", video::ECF_A8R8G8B8);
    cloudBack  = driver->addRenderTargetTexture(core::dimension2d<u32>(512, 512), "cloudBack", video::ECF_A8R8G8B8);
    auto sky   = scene->addSkyBoxSceneNode(
            cloudTop,
            NULL,
            cloudLeft,
            cloudRight,
            cloudFront,
            cloudBack
        );
    sky->setMaterialType((irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                             "../shader/sky.vs.glsl","main", irr::video::EVST_VS_1_1,
                             "../shader/sky.ps.glsl", "main", irr::video::EPST_PS_1_1));
    device->getCursorControl()->setVisible(false);

    scene->setAmbientLight(irr::video::SColor(255,80,80,80));
    auto light = scene->addLightSceneNode();
    light->setPosition(irr::core::vector3df(0,500,0));
/*
    driver->setFog(video::SColor(255, 255, 255, 255 ),    //雾颜色（忽略Alpha位）
                   video::EFT_FOG_LINEAR,                 //雾的类型
                   128.0f,                                //雾起始距离
                   1000.0f,                               //雾结束距离
                   0.1f,                                  //雾的浓度
                   true,                                  //true代表像素（片断）雾，false代表顶点雾
                   true);
*/
    water = new RealisticWaterSceneNode(scene, 2048, 2048, "../../");
    water->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true );
    scene->getRootSceneNode()->addChild(water);
    auto g = scene->getGeometryCreator();
    auto m = g->createPlaneMesh(irr::core::dimension2df(2048,2048));
    auto dw = driver->getTexture("../../res/deepwater.png");
    auto f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-1024,1024),irr::core::vector3df(-90,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    f->setMaterialTexture(0, dw);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(1024,-1024,0),irr::core::vector3df(-90,90,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    f->setMaterialTexture(0, dw);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(-1024,-1024,0),irr::core::vector3df(-90,-90,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    f->setMaterialTexture(0, dw);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,0,0),irr::core::vector3df(-180,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    irr::video::ITexture* pTexture = driver->getTexture("../../res/waterTop.png");
    f->setMaterialTexture(0, pTexture);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    m->drop();
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
    auto cm  = camera->getPosition();
    water->setPosition(irr::core::vector3df(cm.X,waterLevel,cm.Z));
    water->setRotation(irr::core::vector3df(0,camera->getRotation().Y,0));

    renderSky();

    driver->setRenderTarget(0);
    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    scene->drawAll();
    gui->drawAll();

    if(cm.Y<waterLevel){//水下效果
        driver->draw2DRectangle(irr::video::SColor(128,128,128,255),irr::core::rect<irr::s32>(0,0,width,height));
    }

    driver->endScene();
    int fps = driver->getFPS();
    if (lastFPS != fps){
        irr::core::stringw str = L"Smoothly [";
        str += driver->getName();
        str += "] FPS:";
        str += fps;
        device->setWindowCaption(str.c_str());
        lastFPS = fps;
    }
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
    deltaTime = tt-lastTime;
    lastTime  = tt;
}

void engine::renderSky(){
    int tm = time(0);
#define processFace(tmr,tex,col) \
    if(abs(tm-tmr[0])>2){ \
        driver->setRenderTarget(tex,false,false); \
        driver->setMaterial(cloudMaterial); \
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,-1,1),\
                                                      irr::core::vector3df(-1,0,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[0] = tm;\
        return;\
    }\
    if(abs(tm-tmr[1])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,1,1),\
                                                      irr::core::vector3df(0,1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[1] = tm;\
        return;\
    }\
    if(abs(tm-tmr[2])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,1,1),\
                                                      irr::core::vector3df(1,0,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[2] = tm;\
        return;\
    }\
    if(abs(tm-tmr[3])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,-1,1),\
                                                      irr::core::vector3df(0,-1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[3] = tm;\
        return;\
    }\
    if(abs(tm-tmr[4])>2){ \
        driver->setRenderTarget(tex,false,false); \
        driver->setMaterial(cloudMaterial); \
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(-1,0,1),\
                                                      irr::core::vector3df(-1,1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[4] = tm;\
        return;\
    }\
    if(abs(tm-tmr[5])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0,1,1),\
                                                      irr::core::vector3df(1,1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[5] = tm;\
        return;\
    }\
    if(abs(tm-tmr[6])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(1,0,1),\
                                                      irr::core::vector3df(1,-1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[6] = tm;\
        return;\
    }\
    if(abs(tm-tmr[7])>2){\
        driver->setRenderTarget(tex,false,false);\
        driver->setMaterial(cloudMaterial);\
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(0,-1,1),\
                                                      irr::core::vector3df(-1,-1,1),\
                                                      irr::core::vector3df(0,0,1)),\
                               col);\
        tmr[7] = tm;\
        return;\
    }
///////////////////////////////////////////////
    processFace(lastRenderSkyTop,cloudTop,irr::video::SColor(0,0,255,0));
    processFace(lastRenderSkyFront,cloudFront,irr::video::SColor(0,255,0,0));
    processFace(lastRenderSkyBack,cloudBack,irr::video::SColor(0,255,0,255));
    processFace(lastRenderSkyLeft,cloudLeft,irr::video::SColor(0,0,0,0));
    processFace(lastRenderSkyRight,cloudRight,irr::video::SColor(0,0,0,255));
}

}
