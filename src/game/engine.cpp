#include "engine.h"
#include <sstream>
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
    shadowMapSize           = 2048;
    haveSSAO                = true;
    haveSSRTGI              = true;
    SSRTStep                = 16;
    waterMapSize            = 512;
    mblurStep               = 4;
    haveShadowBlur          = false;
    haveMblur               = true;
    haveShadow              = true;
    haveLSGI                = true;

    loadConfig();
    ambientColor.set(1.0,0.2,0.2,0.2);

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
    hizbuf_size = width*height;

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
    water = new RealisticWaterSceneNode(scene, 2048, 2048, "../../" , core::dimension2du(waterMapSize,waterMapSize));
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
    post_ssao = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "ssao", video::ECF_R32F);
    post_ssrt = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "ssrt", video::ECF_A8R8G8B8);
    post_ssrtConf = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "ssrtConf", video::ECF_A16B16G16R16F);
    post_sky = driver->addRenderTargetTexture(core::dimension2d<u32>(1024, 1024), "post_sky", video::ECF_A8R8G8B8);
    post_posi = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "posi", video::ECF_A32B32G32R32F);
    post_final = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "final", video::ECF_A8R8G8B8);
    post_shadow = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "shadow", video::ECF_A8R8G8B8);
    post_lsgi = driver->addRenderTargetTexture(core::dimension2d<u32>(width, height), "lsgi", video::ECF_A8R8G8B8);

    post = driver->addRenderTarget();
    core::array<video::ITexture*> textureArray(3);
    textureArray.push_back(post_tex);
    textureArray.push_back(post_mat);
    textureArray.push_back(post_normal);
    textureArray.push_back(post_posi);
    textureArray.push_back(post_ssrtConf);
    post->setTexture(textureArray, post_depth);

    water->renderTarget = post;

    postShaderCallback.parent = this;
    postShaderCallback.preViewMatrix = camera->getViewMatrix();

#define initPostMat(p) \
    p.setTexture(0,post_tex);\
    p.setTexture(1,post_depth);\
    p.setTexture(2,post_mat);\
    p.setTexture(3,post_normal);\
    p.setTexture(4,post_posi);\
    p.ZBuffer = video::ECFN_DISABLED;

    printf("post shader:final\n");
    initPostMat(postMaterial);
    postMaterial.setTexture(5,post_ssao);
    postMaterial.setTexture(6,post_ssrt);
    postMaterial.setTexture(7,post_ssrtConf);
    postMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/post.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/post.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:light\n");
    initPostMat(lightMaterial);
    lightMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/light.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/light.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:ssao\n");
    initPostMat(ssaoMaterial);
    ssaoMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/ssao.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/ssao.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:mblur\n");
    initPostMat(mblurMaterial);
    mblurMaterial.setTexture(5,post_final);
    mblurMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/mblur.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/mblur.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:ssrt\n");
    initPostMat(ssrtMaterial);
    ssrtMaterial.setTexture(5,post_sky);
    ssrtMaterial.setTexture(6,post_ssrtConf);
    ssrtMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/ssrt.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/ssrt.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:shadow blend\n");
    initPostMat(shadowBlendMaterial);
    shadowBlendMaterial.setTexture(5,post_shadow);
    shadowBlendMaterial.setTexture(6,post_lsgi);
    shadowBlendMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/shadowBlend.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/shadowBlend.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:shadow soft\n");
    initPostMat(shadowSoftMaterial);
    shadowSoftMaterial.setTexture(5,post_shadow);
    shadowSoftMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/shadowSoft.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/shadowSoft.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:shadow map\n");
    initPostMat(shadowMapMaterial);
    shadowMapMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/shadowMap.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/shadowMap.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);

    printf("post shader:lsgi\n");
    initPostMat(lsgiMaterial);
    lsgiMaterial.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                    "../shader/lsgi.vs.glsl", "main", video::EVST_VS_1_1,
                                    "../shader/lsgi.ps.glsl", "main", video::EPST_PS_1_1,
                                    &postShaderCallback);
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
    postShaderCallback.lightMode = false;
    postShaderCallback.finalPass  = false;
    postShaderCallback.ssrtMode = false;
    postShaderCallback.mblurMode = false;
    postShaderCallback.shadowMapMode = false;
    postShaderCallback.shadowBlendMode = false;
    postShaderCallback.lsgiMode = false;
    auto cm  = camera->getPosition();

    if(cm.Y<waterLevel){
        water->setVisible(false);
    }else{
        water->setVisible(true);
    }

    auto coll = scene->getSceneCollisionManager();
    screenCenter = coll->getScreenCoordinatesFrom3DPosition(camera->getTarget(),camera);
    {
        auto lastPos = water->getPosition();
        auto newPos  = irr::core::vector3df(cm.X,waterLevel,cm.Z);
        auto deltaPos_o = newPos - lastPos;
        deltaPos_o.X /= 2048;
        deltaPos_o.Z /= 2048;
        water->setWaveDeltaPos(core::vector2df(deltaPos_o.X , deltaPos_o.Z));
        water->setPosition(newPos);
        water->getMaterial(0).BlendOperation=irr::video::EBO_ADD;
        water->updateWave();
    }

    renderSky();
    if(haveShadow){
        renderShadow();
    }
    renderMiniMap();
    updateListener();

    //前向渲染
    driver->beginScene(true, true, irr::video::SColor(255,0,0,0));
    driver->setRenderTargetEx(post,video::ECBF_COLOR | video::ECBF_DEPTH);
    if(cm.Y>waterLevel){
        enableClipY = 1;
        clipYUp = -1;
        clipY = waterLevel;
    }
    scene->drawAll();
    enableClipY = 0;

#define drawScreen \
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1, 1,1),\
                                                  irr::core::vector3df(-1,-1,1),\
                                                  irr::core::vector3df(-1, 1,1)),\
                           irr::video::SColor(0,0,0,0));\
    driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df( 1,-1,1),\
                                                  irr::core::vector3df(-1,-1,1),\
                                                  irr::core::vector3df( 1, 1,1)),\
                           irr::video::SColor(0,0,0,0));

    //后期：阴影
    if(haveShadow){

        //lsgi
        if(haveLSGI){
            driver->setRenderTarget(post_lsgi,true,true);
            driver->setMaterial(lsgiMaterial);
            postShaderCallback.lsgiMode = true;
            drawScreen;
            postShaderCallback.lsgiMode = false;
        }

        //pass 1
        driver->setRenderTarget(post_shadow,false,false);
        driver->setMaterial(shadowMapMaterial);
        postShaderCallback.shadowMapMode = true;
        drawScreen;
        postShaderCallback.shadowMapMode = false;

        //pass 2
        postShaderCallback.shadowBlendMode = true;
        if(haveShadowBlur){
            driver->setMaterial(shadowSoftMaterial);
            drawScreen;
        }

        //pass 3
        driver->setRenderTarget(post_tex,false,false);
        driver->setMaterial(shadowBlendMaterial);
        drawScreen;
        postShaderCallback.shadowBlendMode = false;
    }

    //后期：光照
    driver->setRenderTarget(post_tex,false,false);
    driver->setMaterial(lightMaterial);
    postShaderCallback.lightMode = true;
    lightManager.updateLight(camera,[&](localLight::lightSource * sour){
        postShaderCallback.lightColor.X = sour->color.r;
        postShaderCallback.lightColor.Y = sour->color.g;
        postShaderCallback.lightColor.Z = sour->color.b;
        postShaderCallback.lightPos = sour->position;
        postShaderCallback.lightRange = sour->range;
        drawScreen;
    },[&](irr::f32 x1,irr::f32 y1,irr::f32 x2,irr::f32 y2,localLight::lightSource * sour){
        postShaderCallback.lightColor.X = sour->color.r;
        postShaderCallback.lightColor.Y = sour->color.g;
        postShaderCallback.lightColor.Z = sour->color.b;
        postShaderCallback.lightPos = sour->position;
        postShaderCallback.lightRange = sour->range;
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(x2,y2,1),
                                                      irr::core::vector3df(x1,y1,1),
                                                      irr::core::vector3df(x1,y2,1)),
                               irr::video::SColor(0,0,0,0));
        driver->draw3DTriangle(irr::core::triangle3df(irr::core::vector3df(x2,y1,1),
                                                      irr::core::vector3df(x1,y1,1),
                                                      irr::core::vector3df(x2,y2,1)),
                               irr::video::SColor(0,0,0,0));
    });
    postShaderCallback.lightMode = false;

    if(haveSSAO){
        //ssao
        driver->setRenderTarget(post_ssao);
        driver->setMaterial(ssaoMaterial);
        drawScreen;
    }

    if(haveSSRTGI){
        //ssrt
        postShaderCallback.ssrtMode = true;
        driver->setRenderTarget(post_ssrt);
        driver->setMaterial(ssrtMaterial);
        drawScreen;
        postShaderCallback.ssrtMode = false;
    }

    //最终后期处理
    postShaderCallback.finalPass = true;
    if(haveMblur){
        driver->setRenderTarget(post_final);
    }else{
        driver->setRenderTarget(0);
    }
    driver->setMaterial(postMaterial);
    drawScreen;
    postShaderCallback.finalPass = false;

    if(haveMblur){
        //动态模糊
        postShaderCallback.mblurMode = true;
        driver->setRenderTarget(0);
        driver->setMaterial(mblurMaterial);
        drawScreen;
        postShaderCallback.mblurMode = false;
    }

    //渲染gui
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

    //为下一帧做准备
    postShaderCallback.preViewMatrix = camera->getViewMatrix();
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

void engine::hizBegin(){
    hizbuf = (u32*)post_depth->lock(irr::video::ETLM_READ_ONLY);
}

bool engine::pointVisible(const core::vector3df & pos3d){
    irr::core::matrix4 trans = camera->getProjectionMatrix();
    trans *= postShaderCallback.preViewMatrix;

    irr::f32 transformedPos[4] = { pos3d.X, pos3d.Y, pos3d.Z, 1.0f };

    trans.multiplyWith1x4Matrix(transformedPos);

    if (transformedPos[3] < 0)
        return false;

    const irr::f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
                                                 irr::core::reciprocal(transformedPos[3]);

    float x = transformedPos[0] * zDiv;
    float y = transformedPos[1] * zDiv;
    float z = transformedPos[2] * zDiv;

    if(x<=-1 || x>=1 || y<=-1 || y>=1 || z<=0 || z>=1)//在屏幕外
        return false;

    u32 sx = round(width*((x+1.0)/2.0));
    u32 sy = round(height*((y+1.0)/2.0));
    u32 index = sy*width+sx;
    if(index>=hizbuf_size)
        return false;

    u32 dep = hizbuf[index];
    u32 ndep = z*0xffffffff;

    return (ndep<dep);
}

void engine::hizEnd(){
    post_depth->unlock();
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

void engine::loadConfig(){
    auto fp = fopen("../config/game.conf","r");
    if(fp){
        char buf[1024];
        while (!feof(fp)) {
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(buf[0]!='#'){
                int val = 0;
                std::istringstream iss(buf);
                std::string key;
                iss>>key;
                if(key=="shadowMapSize"){
                    iss>>shadowMapSize;
                }else if(key=="shadowArea"){
                    iss>>shadowArea;
                }else if(key=="SSAO"){
                    iss>>val;
                    haveSSAO = (val==1);
                }else if(key=="SSRTGI"){
                    iss>>val;
                    haveSSRTGI = (val==1);
                }else if(key=="SSRTStep"){
                    iss>>SSRTStep;
                }else if(key=="waterMapSize"){
                    iss>>waterMapSize;
                }else if(key=="mblurStep"){
                    iss>>mblurStep;
                    if(mblurStep<=1){
                        haveMblur = false;
                    }
                }else if(key=="haveShadowBlur"){
                    iss>>val;
                    haveShadowBlur = (val==1);
                }else if(key=="haveShadow"){
                    iss>>val;
                    haveShadow = (val==1);
                }else if(key=="haveRefraction"){
                    iss>>val;
                    haveRefraction = (val==1);
                }else if(key=="haveReflection"){
                    iss>>val;
                    haveReflection = (val==1);
                }else if(key=="halfFrameWater"){
                    iss>>val;
                    halfFrameWater = (val==1);
                }else if(key=="haveLSGI"){
                    iss>>val;
                    haveLSGI = (val==1);
                }
            }
        }
        fclose(fp);
    }
}

void engine::PostShaderCallback::OnSetConstants(video::IMaterialRendererServices * services, s32 userData){
    s32 var0 = 0;
    s32 var1 = 1;
    s32 var2 = 2;
    s32 var3 = 3;
    s32 var4 = 4;
    auto cam = parent->camera->getPosition();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("tex"),&var0, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("depth"),&var1, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("materialMap"),&var2, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("normalMap"),&var3, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("posMap"),&var4, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("waterLevel"),&parent->waterLevel, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("camera"),&cam.X, 3);
    if(lightMode){
        services->setPixelShaderConstant(services->getPixelShaderConstantID("lightColor"),&lightColor.X, 3);
        services->setPixelShaderConstant(services->getPixelShaderConstantID("lightPos"),&lightPos.X, 3);
        services->setPixelShaderConstant(services->getPixelShaderConstantID("lightRange"),&lightRange, 1);
    }
    if(finalPass){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("ssaoMap"),&var5, 1);
        s32 var6 = 6;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("ssrtMap"),&var6, 1);
        s32 var7 = 7;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("ssrtConfMap"),&var7, 1);
        s32 ssao = parent->haveSSAO?1:0;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("haveSSAO"),&ssao, 1);
        s32 ssrtgi = parent->haveSSRTGI?1:0;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("haveSSRTGI"),&ssrtgi, 1);
    }
    if(ssrtMode){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("skyMap"),&var5, 1);
        s32 var6 = 6;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("ssrtConfMap"),&var6, 1);
        services->setPixelShaderConstant(services->getPixelShaderConstantID("skyMatrix") , parent->skyMatrix.pointer(), 16);
    }
    if(mblurMode){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("resultMap"),&var5, 1);
        services->setPixelShaderConstant(services->getPixelShaderConstantID("mblurStep"),&parent->mblurStep, 1);
    }
    if(shadowBlendMode){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowViewMap"),&var5, 1);
        s32 var6 = 6;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("lsgiMap"),&var6, 1);
    }
    if(lsgiMode){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("lsgiMap"),&var5, 1);
        s32 var6 = 6;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowPosMap"),&var6, 1);
    }
    if(shadowMapMode){
        s32 var5 = 5;
        services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMap"),&var5, 1);
        services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMapSize"),&parent->shadowMapSize, 1);
    }
    services->setPixelShaderConstant(services->getPixelShaderConstantID("SSRTStep"),&parent->SSRTStep, 1);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("windowWidth"),&parent->width, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("windowHeight"),&parent->height, 1);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("lightDir"),&parent->lightDir.X, 3);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowFactor"),&parent->shadowFactor, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMatrix"),parent->shadowMatrix.pointer(), 16);

    auto pmat = parent->camera->getProjectionMatrix();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("ProjMatrix") , pmat.pointer(), 16);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("preViewMatrix") , preViewMatrix.pointer(), 16);

    auto vmat = parent->camera->getViewMatrix();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("ViewMatrix") , vmat.pointer(), 16);
    irr::core::matrix4 vmati;
    vmat.getInverse(vmati);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("ViewMatrixInv") , vmati.pointer(), 16);

}

}
