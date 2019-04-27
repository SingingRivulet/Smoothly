#include "game.h"
#include <stdlib.h>
#include <atomic>
#include <thread>

std::atomic<bool> running;
smoothly::game game;

std::condition_variable gbcv;
std::mutex gbmtx;

int main(int argc , char * argv[]){
    if(argc<5){
        printf(
            "Smoothly Client\n"
            "A Open Source First Person Sandbox Strategy game\n\n"
            "Format: %s [address] [port] [user name] [password]\n"
            "Such as %s 127.0.0.1 39065 d8eb0a45-8fb5-45a8-9e65-475458337b17 123456\n\n"
            "Singing Rivulet Project(https://github.com/SingingRivulet)\n"
            "By cgoxopx<cgoxopx@qq.com>\n",
            argv[0],argv[0]);
        return 1;
    }
    
    std::string iaddr   =argv[1];
    int port            =atoi(argv[2]);
    std::string iname   =argv[3];
    std::string ipasswd =argv[4];
    
    game.gameInit(iaddr.c_str(),port,iname,ipasswd);
    running=true;
    game.addEventRecv();
    
    while(running && game.ok()){
        game.loop();
    }
    running=false;
    game.canConnect=false;
    
    game.gameDestroy();
    return 0;
}
