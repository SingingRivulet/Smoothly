#include "game.h"
#include <atomic>
#include <thread>

std::atomic<bool> running;
smoothly::game game;

std::condition_variable gbcv;
std::mutex gbmtx;

int main(){
    game.gameInit("127.0.0.1",39065,"smoothly.lua");
    running=true;
    game.addEventRecv();
    game.canFly=true;
    game.setCameraPosition(irr::core::vector3df(0,game.getRealHight(0,0)+2,0));
    
    while(running && game.ok()){
        game.loop();
    }
    running=false;
    game.canConnect=false;
    
    game.gameDestroy();
    return 0;
}
