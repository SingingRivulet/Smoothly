#include "body.h"

#include <QFile>
#include <QByteArray>

namespace smoothly{


void body::loadBodyConfig(){
    printf("[status]get body config\n" );
    QFile file("../config/body.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/body.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();
    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;
            while (c){
                if(c->type==cJSON_Object){
                    auto idnode = cJSON_GetObjectItem(c,"id");
                    if(idnode && idnode->type==cJSON_Number){
                        int id = idnode->valueint;
                        if(bodyConfig.find(id)!=bodyConfig.end()){
                            printf("[error]can't redefine body %d\n" , id);
                        }else{
                            auto mnode = cJSON_GetObjectItem(c,"mesh");
                            if(mnode && mnode->type==cJSON_String){
                                auto mesh = scene->getMesh(mnode->valuestring);
                                if(mesh){
                                    auto ptr  = new bodyConf;
                                    bodyConfig[id]= ptr;
                                    ptr->mesh = mesh;
                                    auto item = c->child;
                                    while(item){
                                        if(item->type==cJSON_Number){
                                            if(strcmp(item->string,"width")==0){
                                                ptr->width = item->valueint;
                                            }else
                                            if(strcmp(item->string,"height")==0){
                                                ptr->height = item->valueint;
                                            }else
                                            if(strcmp(item->string,"walkInSky")==0){
                                                ptr->walkInSky = (item->valueint!=0);
                                            }else
                                            if(strcmp(item->string,"jumpInSky")==0){
                                                ptr->jumpInSky = (item->valueint!=0);
                                            }
                                        }else
                                        if(item->type==cJSON_String){
                                            if(strcmp(item->string,"aniCallback")==0){
                                                ptr->aniCallback = item->valuestring;
                                            }
                                        }
                                        item = item->next;
                                    }
                                }else{
                                    printf("[error]fail to load mesh\n");
                                }
                            }else{
                                printf("[error]mesh have not been defined\n");
                            }
                        }
                    }else{
                        printf("[error]can't get id\n");
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/body.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}
void body::loadWearingConfig(){
    printf("[status]get wearing config\n" );
    QFile file("../config/wearing.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/wearing.json\n" );
        return;
    }
    QByteArray allData = file.readAll();
    file.close();
    auto str = allData.toStdString();
    cJSON * json=cJSON_Parse(str.c_str());
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;
            while (c){
                if(c->type==cJSON_Object){
                    auto idnode = cJSON_GetObjectItem(c,"id");
                    if(idnode && idnode->type==cJSON_Number){
                        int id = idnode->valueint;
                        if(wearingConfig.find(id)!=wearingConfig.end()){
                            printf("[error]can't redefine wearing %d\n" , id);
                        }else{
                            auto mnode = cJSON_GetObjectItem(c,"mesh");
                            if(mnode && mnode->type==cJSON_String){
                                auto mesh = scene->getMesh(mnode->valuestring);
                                if(mesh){
                                    auto ptr  = new wearingConf;
                                    wearingConfig[id]= ptr;
                                    ptr->mesh = mesh;
                                    auto item = c->child;
                                    while(item){
                                        if(item->type==cJSON_String){
                                            if(strcmp(item->string,"attach")==0){
                                                ptr->attach = item->valuestring;
                                            }
                                        }
                                        item = item->next;
                                    }
                                }else{
                                    printf("[error]fail to load mesh\n");
                                }
                            }else{
                                printf("[error]mesh have not been defined\n");
                            }
                        }
                    }else{
                        printf("[error]can't get id\n");
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/wearing.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}

void body::releaseBodyConfig(){
    for(auto it:bodyConfig){
        it.second->mesh->drop();
        delete it.second;
    }
    bodyConfig.clear();
}

void body::releaseWearingConfig(){
    for(auto it:wearingConfig){
        it.second->mesh->drop();
        delete it.second;
    }
    wearingConfig.clear();
}

}
