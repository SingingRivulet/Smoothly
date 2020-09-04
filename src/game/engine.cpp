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
    param.Stencilbuffer = true;
    param.WindowSize.set(width,height);
    param.Bits = 16;

    device = irr::createDeviceEx(param);

    driver = device->getVideoDriver();
    scene  = device->getSceneManager();
    gui    = device->getGUIEnvironment();
    timer  = device->getTimer();
    collisionManager = scene->getSceneCollisionManager();
    device->setWindowCaption(L"Smoothly");
    driver->setAllowZWriteOnTransparent(true);
    vendor = driver->getVendorInfo();
    scene->setAmbientLight(irr::video::SColor(255,128,128,128));
    scene->setShadowColor(irr::video::SColor(250, 0, 0, 0));

    //gui
    gui::IGUISkin* newskin = gui->createSkin(gui::EGST_WINDOWS_CLASSIC);
    font = gui->getFont("../../res/font/fonthaettenschweiler.bmp");
    newskin->setFont(font);
    newskin->setColor(irr::gui::EGDC_BUTTON_TEXT,irr::video::SColor(255, 128, 128, 128));
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
    water->setWindForce(5);
    water->graph = this;
    scene->getRootSceneNode()->addChild(water);
    auto g = scene->getGeometryCreator();
    auto m = g->createPlaneMesh(irr::core::dimension2df(2048,2048));
    /*
    auto dw = driver->getTexture("../../res/deepwater.png");
    auto f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-1024,1024),irr::core::vector3df(-90,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    f->setMaterialTexture(0, dw);
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    //f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-1024,-1024),irr::core::vector3df(-90,180,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    f->setMaterialTexture(0, dw);
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    //f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(1024,-1024,0),irr::core::vector3df(-90,90,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    f->setMaterialTexture(0, dw);
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    //f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(-1024,-1024,0),irr::core::vector3df(-90,-90,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    f->setMaterialTexture(0, dw);
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    //f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    //水底
    auto f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,-2048,0),irr::core::vector3df(0,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    f->setMaterialTexture(0, driver->getTexture("../../res/waterBottom.png"));
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    //f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    */

    auto f = scene->addMeshSceneNode(m,water,0,irr::core::vector3df(0,0,0),irr::core::vector3df(-180,0,0));
    f->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    irr::video::ITexture* pTexture = driver->getTexture("../../res/waterTop.png");
    f->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    f->setMaterialTexture(0, pTexture);
    f->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    m->drop();

    ttf = new irr::gui::CGUITTFont(driver);
    auto face = new irr::gui::CGUITTFace;
    face->load("../../res/font.ttf");
    ttf->attach(face,16);
    face->drop();

    //初始化音频
    audioDevice = alcOpenDevice(NULL);
    if(audioDevice  == NULL)
        exit(1);
    audioContext=alcCreateContext(audioDevice, NULL);
    alcMakeContextCurrent(audioContext);
    alGetError();

    post_tex = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "tex", video::ECF_A8R8G8B8);
    post_mat = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "mat", video::ECF_A8R8G8B8);
    post_normal = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "normal", video::ECF_A8R8G8B8);
    post_depth = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "DepthStencil", video::ECF_D32);
    post = driver->addRenderTarget();
    core::array<video::ITexture*> textureArray(3);
    textureArray.push_back(post_tex);
    textureArray.push_back(post_mat);
    textureArray.push_back(post_normal);
    post->setTexture(textureArray, post_depth);

    water->renderTarget = post;
    postMaterial.setTexture(0,post_tex);
    postMaterial.setTexture(1,post_depth);
    postMaterial.setTexture(2,post_mat);
    postMaterial.setTexture(3,post_normal);
    postShaderCallback.parent = this;
    postMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/post.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/post.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);;
}
engine::~engine(){
    ttf->drop();
    device->drop();
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;

    //释放音频buffer
    for(auto it:audioBuffers){
        it.second->drop();
    }
    audioBuffers.clear();
    //释放音频设备
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice( context );
    alcMakeContextCurrent( NULL );
    alcDestroyContext( context );
    alcCloseDevice( device );
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
    renderShadow();
    renderMiniMap();
    updateListener();

    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    driver->setRenderTargetEx(post,video::ECBF_COLOR | video::ECBF_DEPTH);
    scene->drawAll();
    driver->setRenderTarget(0);
    driver->setMaterial(postMaterial);
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1, 1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df(-1, 1,1)),
                           irr::video::SColor(0,0,0,0));
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1,-1,1),
                                                  irr::core::vector3df(-1,-1,1),
                                                  irr::core::vector3df( 1, 1,1)),
                           irr::video::SColor(0,0,0,0));
    //driver->draw2DImage(post_tex,core::vector2di(0,0));
    //scene->drawAll();
    //if(cm.Y<waterLevel){//水下效果
    //    driver->draw2DRectangle(irr::video::SColor(128,128,128,255),irr::core::rect<irr::s32>(0,0,width,height));
    //}

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

void engine::drawArcProgressBar(core::position2d<s32> center,
    f32 radius, video::SColor color, s32 count,f32 progress){
    if (count < 2)
        return;

    f32 rpro = progress;
    if(rpro>1.f)
        rpro = 1.f;
    else if(rpro<0.f)
        rpro = 0.f;

    s32 drawCount = count*rpro;

    core::position2d<s32> first;
    core::position2d<s32> a,b;

    for (s32 j=0; j<drawCount+1; ++j){
        b = a;

        f32 p = j / (f32)count * (core::PI*2);
        a = center + core::position2d<s32>((s32)(sin(p)*radius), (s32)(cos(p)*radius));

        if (j==0)
            first = a;
        else
            driver->draw2DLine(a, b, color);
    }
}

engine::audioBuffer *engine::getAudioBuffer(const char * path){
    auto it = audioBuffers.find(path);
    if(it==audioBuffers.end()){
        auto p = new audioBuffer(path);
        audioBuffers[path] = p;
        return p;
    }else{
        return it->second;
    }
}

void engine::playAudioPosition(const core::vector3df & posi, engine::audioBuffer * buf){
    auto au = new audioSource;
    if(au){
        au->setPosition(posi);
        au->play(buf,false);
        playingSources.push_back(au);
    }
}

void engine::updateListener(){
    auto posi = camera->getPosition();
    auto targ = camera->getTarget();
    auto dir  = targ-posi;
    dir.normalize();//朝向
    auto up   = camera->getUpVector();

    alListener3f(AL_POSITION, posi.X, posi.Y, posi.Z);

    float vec[6];
    vec[0] = dir.X;
    vec[1] = dir.Y;
    vec[2] = dir.Z;
    vec[3] = up.X;
    vec[4] = up.Y;
    vec[5] = up.Z;
    alListenerfv(AL_ORIENTATION, vec);

    playingSources.remove_if([](audioSource * s){
        if(!s->isPlaying()){
            s->drop();
            return true;
        }
        return false;
    });
}

void engine::PostShaderCallback::OnSetConstants(video::IMaterialRendererServices * services, s32 userData){
    s32 var0 = 0;
    s32 var1 = 1;
    s32 var2 = 2;
    s32 var3 = 3;
    auto cam = parent->camera->getPosition();
    services->setPixelShaderConstant("tex",&var0, 1);
    services->setPixelShaderConstant("depth",&var1, 1);
    services->setPixelShaderConstant("materialMap",&var2, 1);
    services->setPixelShaderConstant("normalMap",&var3, 1);
    services->setPixelShaderConstant("waterLevel",&parent->waterLevel, 1);
    services->setPixelShaderConstant("camera",&cam.X, 3);

    auto vmat = parent->camera->getViewMatrix();
    services->setPixelShaderConstant("ViewMatrix" , vmat.pointer(), 16);
    irr::core::matrix4 vmati;
    vmat.getInverse(vmati);
    services->setPixelShaderConstant("ViewMatrixInv" , vmati.pointer(), 16);

}

}
