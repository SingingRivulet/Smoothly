#include "game.h"
#include <atomic>
std::atomic<bool> running;
int main(){
    smoothly::game game;
    game.gameInit("127.0.0.1",39065,"smoothly.lua");
    running=true;
    game.addEventRecv();
    game.canFly=true;
    game.setCameraPosition(irr::core::vector3df(0,game.getRealHight(0,0)+2,0));
    while(running && game.ok()){
        game.loop();
    }
    game.gameDestroy();
    return 0;
}
