#include "serverNetwork.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <atomic>
std::atomic<bool> running;
int main(int argc,char * argv[]){
    
    signal(SIGINT,[](int sign){
        running=false;
    });
    signal(SIGPIPE,[](int sign){
        
    });
    signal(SIGHUP,[](int sign){
        
    });
    
    char pre[256];
    short port=39065;
    int maxconn=64;
    
    if(argc>=2)
        snprintf(pre,sizeof(pre),"%s",argv[1]);
    else
        snprintf(pre,sizeof(pre),"data");
    
    if(argc>=3)
        port=atoi(argv[2]);
    
    if(argc>=4)
        maxconn=atoi(argv[3]);
    
    smoothly::serverNetwork server;
    
    server.start(pre,port,maxconn);
    running=true;
    while(running){
        server.recv();
        RakSleep(30);
    }
    server.release();
    
    return 0;
}