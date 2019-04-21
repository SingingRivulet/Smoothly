#ifndef SMOOTHLY_CLIENT_FINAL
#define SMOOTHLY_CLIENT_FINAL
#include "control.h"
namespace smoothly{
    class game:public control{
        public:
            void gameInit(const char * addr,short port,const std::string & name,const std::string & passwd){
                loadWorld();
                clientNetwork::init(addr,port);
                clientNetwork::login(name,passwd);
                m=new mods;
                m->init();
                m->scriptInit("./script/smoothly.lua");
                
                loadScene();
                
                m->scene=scene;
                m->loadMesh();
                m->loadConfig();
                buildingSelectionInit();
                
                addCamera();
                //addEventRecv();
            }
            void gameDestroy(){
                m->scriptDestroy();
                m->destroy();
                delete m;
                
                clientNetwork::shutdown();
                destroyWorld();
                destroyScene();
            }
        
    };
};
#endif