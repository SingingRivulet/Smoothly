#include "engine.h"
namespace smoothly{
engine::engine(){
    deltaTimeUpdateFirst    = true;
    running                 = true;
    deltaTime               = 0;
    lastTime                = 0;
    waterLevel              = 0;
    lastFPS                 = 0;
    width                   = 1024;
    height                  = 768;

    //device = irr::createDevice(
    //            irr::video::EDT_OPENGL,
    //            irr::core::dimension2d<irr::u32>(width,height),
    //            16,
    //            false,
    //            true
    //);
    irr::SIrrlichtCreationParameters param;
    param.DriverType = irr::video::EDT_OPENGL;
    param.DeviceType = irr::EIDT_BEST;
    param.WindowSize.set(width,height);
    param.Bits = 16;

    device = irr::createDeviceEx(param);

    driver = device->getVideoDriver();
    scene  = device->getSceneManager();
    gui    = device->getGUIEnvironment();
    timer  = device->getTimer();
    device->setWindowCaption(L"Smoothly");
    driver->setAllowZWriteOnTransparent(true);
    vendor = driver->getVendorInfo();
    scene->setAmbientLight(irr::video::SColor(255,128,128,128));
    scene->setShadowColor(irr::video::SColor(150, 0, 0, 0));

    //gui
    gui::IGUISkin* newskin = gui->createSkin(gui::EGST_WINDOWS_CLASSIC);
    font = gui->getFont("../../res/font/fonthaettenschweiler.bmp");
    newskin->setFont(font);
    newskin->setColor(irr::gui::EGDC_3D_DARK_SHADOW,irr::video::SColor(0, 0, 0, 0));
    newskin->setColor(irr::gui::EGDC_3D_SHADOW,irr::video::SColor(0, 0, 0, 0));
    newskin->setColor(irr::gui::EGDC_3D_HIGH_LIGHT,irr::video::SColor(0, 0, 0, 0));
    newskin->setColor(irr::gui::EGDC_3D_LIGHT,irr::video::SColor(0, 0, 0, 0));
    gui->setSkin(newskin);
    newskin->drop();

    //初始化物理世界
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
    overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());//ghost查询的回调函数

    struct collisionFilterCallback : public btOverlapFilterCallback{//碰撞过滤器
            virtual bool  needBroadphaseCollision(btBroadphaseProxy * proxy0,btBroadphaseProxy* proxy1) const{
                btCollisionObject* b0 = (btCollisionObject*)proxy0->m_clientObject;
                btCollisionObject* b1 = (btCollisionObject*)proxy1->m_clientObject;
                auto bf0 = (bodyInfo*)b0->getUserPointer();
                auto bf1 = (bodyInfo*)b1->getUserPointer();
                if(bf0 && bf1 && bf0->type==BODY_BULLET && bf1->type==BODY_BULLET){
                    //两个都是子弹
                    return false;
                }
                bool collides = (proxy0->m_collisionFilterGroup &proxy1->m_collisionFilterMask) != 0;
                collides = collides && (proxy1->m_collisionFilterGroup &proxy0->m_collisionFilterMask);
                return collides;
            }
    };
    dynamicsWorld->getPairCache()->setOverlapFilterCallback(new collisionFilterCallback());//加入碰撞过滤器

    this->dynamicsWorld->setGravity(btVector3(0, -10, 0));


    camera=scene->addCameraSceneNodeFPS();

    //隐藏鼠标
    device->getCursorControl()->setVisible(false);

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
    water->setWindForce(10);
    scene->getRootSceneNode()->addChild(water);
    auto g = scene->getGeometryCreator();
    auto m = g->createPlaneMesh(irr::core::dimension2df(2048,2048));
    auto dw = driver->getTexture("../../res/deepwater.png");
    auto f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-1024,1024),irr::core::vector3df(-90,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    f->setMaterialTexture(0, dw);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-1024,-1024),irr::core::vector3df(-90,180,0));
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

    //水底
    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-2048,0),irr::core::vector3df(0,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, false );
    f->setMaterialTexture(0, driver->getTexture("../../res/waterBottom.png"));
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

    auto coll = scene->getSceneCollisionManager();
    screenCenter = coll->getScreenCoordinatesFrom3DPosition(camera->getTarget(),camera);
    water->setPosition(irr::core::vector3df(cm.X,waterLevel,cm.Z));
    water->getMaterial(0).BlendOperation=irr::video::EBO_ADD;

    renderSky();

    driver->setRenderTarget(0);
    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    scene->drawAll();
    if(cm.Y<waterLevel){//水下效果
        driver->draw2DRectangle(irr::video::SColor(128,128,128,255),irr::core::rect<irr::s32>(0,0,width,height));
    }
    onDraw();
    gui->drawAll();

    driver->endScene();
    int fps = driver->getFPS();
    if (lastFPS != fps){
        irr::core::stringw str = L"Smoothly [";
        str += vendor+":"+driver->getName();
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

void engine::onDraw(){}
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

}
