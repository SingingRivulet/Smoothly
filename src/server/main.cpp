#include "server.h"
#include <signal.h>
#include <unistd.h>
#include <atomic>
std::atomic<bool> running;
int main(){
    signal(SIGINT,[](int sign){
        running=false;
    });
    signal(SIGPIPE,[](int sign){
        
    });
    signal(SIGHUP,[](int sign){
        
    });
    running=true;
    smoothly::server::server S;
    S.start(39065,256);
    while(running){
        S.recv();
        RakSleep(30);
    }
    S.release();
    return 0;
}