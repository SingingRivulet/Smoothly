#include "server.h"
#include <signal.h>
#include <unistd.h>
#include <atomic>
#include <fontcolor.h>
std::atomic<bool> running;
int main(){
    signal(SIGINT,[](int ){
        running=false;
    });
    signal(SIGPIPE,[](int ){
        
    });
    signal(SIGHUP,[](int ){
        
    });
    running=true;
    printf(L_GREEN "[status]" NONE "init database\n" );
    smoothly::server::server S;
    printf(L_GREEN"[status]" NONE "open connect\n" );
    S.start(39065,256);
    while(running){
        S.recv();
        RakSleep(30);
    }
    printf(L_GREEN "[status]" NONE "release database\n" );
    S.release();
    return 0;
}
