#include "server.h"
#include "../utils/cJSON.h"
#include <signal.h>
#include <unistd.h>
#include <atomic>
#include <fontcolor.h>

#include <QFile>
#include <QByteArray>

std::atomic<bool> running;

unsigned short CONF_port = 39065;
int CONF_maxconnect = 256;
int CONF_visualField = 2;

void loadConfig(){
    printf(L_GREEN "[status]" NONE "get server config\n" );
    QFile file("../config/server.json");
    if(!file.open(QFile::ReadOnly)){
        printf(L_RED "[error]" NONE "fail to read ../config/server.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();
    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Object){
            cJSON *c=json->child;
            while (c){
                if(strcmp(c->string,"port")==0){
                    if(c->type==cJSON_Number)
                        CONF_port = c->valueint;
                }else
                if(strcmp(c->string,"maxconnect")==0){
                    if(c->type==cJSON_Number)
                        CONF_maxconnect = c->valueint;
                }else
                if(strcmp(c->string,"visualfield")==0){
                    if(c->type==cJSON_Number)
                        CONF_visualField = c->valueint;
                }
                c=c->next;
            }
        }else{
            printf(L_RED "[error]" NONE "root in ../config/server.json is not Object!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf(L_RED "[error]" NONE "fail to load json\n" );
    }
}
int main(){
    loadConfig();
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
    S.start(CONF_port,CONF_maxconnect,CONF_visualField);
    while(running){
        S.recv();
        RakSleep(30);
    }
    printf(L_GREEN "[status]" NONE "release database\n" );
    S.release();
    return 0;
}
