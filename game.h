#ifndef SMOOTHLY_CLIENT_FINAL
#define SMOOTHLY_CLIENT_FINAL
#include "control.h"
namespace smoothly{
    class game:public control{
        public:
            void gameInit(const char * addr,short port,const char * modpath){
                loadWorld();
                clientNetwork::init(addr,port);
                m=new mods;
                m->init(modpath);
                m->scriptInit("./script/smoothly.lua");
                
                loadScene();
                
                m->scene=scene;
                m->loadMesh();
                
                buildingSelectionInit();
                
                addCamera();
                //addEventRecv();
            }
            void gameDestroy(){
                m->scriptDestroy();
                m->destroy();
                delete m;
                
                clientNetwork::shutdown();
                destroyScene();
                destroyWorld();
            }
        
    };
};
#endif