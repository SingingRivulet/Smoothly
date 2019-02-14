#include "building.h"
using namespace irr;
int main(){
    // start up physical
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    // start up the engine
    IrrlichtDevice * device = createDevice(video::EDT_OPENGL,
        core::dimension2d<u32>(800,600));

    video::IVideoDriver * driver = device->getVideoDriver();
    scene::ISceneManager * scenemgr = device->getSceneManager();

    device->setWindowCaption(L"Smoothly");

    smoothly::buildings t;
    smoothly::mods m;
    t.m=&m;
    t.device=device;
    t.scene=scenemgr;
    t.dynamicsWorld=dynamicsWorld;

    t.generator.seed=1234506;
    t.pointNum=34;
    t.altitudeK=0.08;
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
    camera->setPosition(core::vector3df(0,t.getDefaultCameraHight(0,0)+2,0));

    //auto sph=scenemgr->addSphereSceneNode();
    //sph->setPosition(core::vector3df(0,10,2000));
    //sph->setMaterialFlag(video::EMF_LIGHTING, false );

    auto md=new smoothly::mods::itemBase;
    md->mesh=scenemgr->getGeometryCreator()->createSphereMesh();
    t.m->items[1]=md;
    t.m->mapGenFuncs.push_back(
        [](int x,int y,float temp,float humi,float alti,smoothly::mods::mapGenerator * gen){
            gen->add(
                1,
                irr::core::vector3df(x*32+16.0f,gen->getRealHight(x+16,y+16)+10,y*32+16.0f),
                irr::core::vector3df(0,0,0),
                irr::core::vector3df(1,1,1)
            );
            gen->add(
                1,
                irr::core::vector3df(x*32,gen->getRealHight(x+16,y+16)+10,y*32),
                irr::core::vector3df(0,0,0),
                irr::core::vector3df(1,1,1)
            );
        }
    );

    t.genTexture();

    t.visualChunkUpdate(0,0,true);

    t.remove(smoothly::terrain::mapid(0,0,1,1));

    irr::core::line3d<irr::f32> line;
    irr::core::vector3df outCollisionPoint;
    irr::core::triangle3df outTriangle;
    irr::scene::ISceneNode* outNode;

    while(device->run() && driver){
        driver->beginScene(true, true, video::SColor(255,0,0,0));
        scenemgr->drawAll();
        driver->endScene();
        line.start=camera->getPosition();
        line.end=line.start+(camera->getTarget()-line.start).normalize()*32.0f;
        if(t.selectPointM(line,outCollisionPoint,outTriangle,outNode)){
            //printf("select:(%f,%f,%f)\n",outCollisionPoint.X,outCollisionPoint.Y,outCollisionPoint.Z);
        }
    }
    //t.destroyTexture();
    t.destroy();

    // delete device
    device->drop();
    delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;

    return 0;
}

