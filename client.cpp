#include "game.h"
#include <atomic>
#include <thread>

std::atomic<bool> running;
smoothly::game game;

std::condition_variable gbcv;
std::mutex gbmtx;

int terrainUpdater(){
    while(running){
        game.updateTerrainThread();
    }
    std::unique_lock <std::mutex> lck(gbmtx);
    gbcv.notify_all();
}
int main(){
    game.gameInit("127.0.0.1",39065,"smoothly.lua");
    running=true;
    game.addEventRecv();
    game.canFly=true;
    game.setCameraPosition(irr::core::vector3df(0,game.getRealHight(0,0)+2,0));
    std::thread t(terrainUpdater);
    t.detach();
    while(running && game.ok()){
        game.loop();
    }
    running=false;
    game.canConnect=false;
    game.terrainWake();
    std::unique_lock <std::mutex> lck(gbmtx);
    gbcv.wait(lck);
    
    game.gameDestroy();
    return 0;
}
