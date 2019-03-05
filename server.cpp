#include "serverNetwork.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <atomic>
#include <sys/types.h>
#include <sys/stat.h>
std::atomic<bool> running;
int main(int argc,char * argv[]){
    
    signal(SIGINT,[](int sign){
        running=false;
    });
    signal(SIGPIPE,[](int sign){
        
    });
    signal(SIGHUP,[](int sign){
        
    });
    
    short port=39065;
    int maxconn=64;
    char pre[128];
    char path1[PATH_MAX];
    char path2[PATH_MAX];
    char path3[PATH_MAX];
    
    if(argc>=2)
        snprintf(pre,sizeof(pre),"%s",argv[1]);
    else
        snprintf(pre,sizeof(pre),"data");
    
    mkdir(pre,0777);
    
    if(argc>=3)
        port=atoi(argv[2]);
    
    if(argc>=4)
        maxconn=atoi(argv[3]);
    
    snprintf(path1,sizeof(path1),"%s/gra",pre);
    snprintf(path2,sizeof(path1),"%s/rmt",pre);
    snprintf(path3,sizeof(path1),"%s/mod.lua",pre);
    
    smoothly::serverNetwork server(path1,path2,path3,port,maxconn);
    running=true;
    while(running){
        server.recv();
        RakSleep(30);
    }
}