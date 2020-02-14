#include "game.h"

namespace smoothly {

game::game(const char *addr, unsigned short port){
    this->connect(addr , port);
}

game::~game(){
    this->shutdown();
}

bool game::gameLoop(){
    if(!ok())
        return false;
    recv();
    process();
    processControl();
    deltaTimeUpdate();
    worldLoop();
    sceneLoop();
    return true;
}

}
