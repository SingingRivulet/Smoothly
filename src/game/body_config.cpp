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
                                                ptr->width = item->valuedouble;
                                            }else
                                            if(strcmp(item->string,"height")==0){
                                                ptr->height = item->valuedouble;
                                            }else
                                            if(strcmp(item->string,"walkInSky")==0){
                                                ptr->walkInSky = (item->valueint!=0);
                                            }else
                                            if(strcmp(item->string,"jumpInSky")==0){
                                                ptr->jumpInSky = (item->valueint!=0);
                                            }else
                                            if(strcmp(item->string,"teleport")==0){
                                                ptr->teleport = (item->valueint!=0);
                                            }else
                                            if(strcmp(item->string,"walkVelocity")==0){
                                                ptr->walkVelocity = item->valuedouble;
                                            }else
                                            if(strcmp(item->string,"deltaY")==0){
                                                ptr->deltaY = item->valuedouble;
                                            }else
                                            if(strcmp(item->string,"jump")==0){
                                                ptr->jump = item->valuedouble;
                                            }
                                        }else if(item->type==cJSON_String){
                                            if(strcmp(item->string,"aniCallback")==0){

                                                lua_settop(L,0);
                                                lua_getglobal(L,item->valuestring);//获取函数
                                                if(lua_isfunction(L,-1)){//检查
                                                    ptr->aniCallback = luaL_ref(L,LUA_REGISTRYINDEX);
                                                    ptr->haveAniCallback = true;
                                                }
                                                lua_settop(L,0);

                                            }else
                                            if(strcmp(item->string,"script")==0){
                                                luaL_dofile(L, item->valuestring);
                                            }else
                                            if(strcmp(item->string,"texture")==0){
                                                ptr->texture = driver->getTexture(item->string);
                                            }
                                        }else if(item->type==cJSON_Array){
                                            if(strcmp(item->string,"animation")==0){
                                                auto line = item->child;
                                                while(line){
                                                    if(line->type==cJSON_String){
                                                        auto path = line->valuestring;
                                                        if(strlen(path)>0){
                                                            auto animesh = scene->getMesh(path);
                                                            if(animesh){
                                                                //处理mesh
                                                                if(animesh->getMeshType()==irr::scene::EAMT_SKINNED){
                                                                    //是骨骼动画
                                                                    auto bmesh = (irr::scene::ISkinnedMesh*)animesh;
                                                                    ptr->animation.push_back(bmesh);
                                                                }else{
                                                                    animesh->drop();
                                                                    printf("[error]%s is not skinned mesh\n",path);
                                                                    break;
                                                                }
                                                            }else{
                                                                printf("[error]fail to load %s\n",path);
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    line = line->next;
                                                }
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
    FILE * fp = fopen("../config/wearing_bullet.txt" , "r");
    if(fp){
        char buf[128];
        while(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(strlen(buf)>0){
                int firingWearingId;
                int fireId;
                int fireDelta;
                if(sscanf(buf,"%d=%d,%d" , &firingWearingId , &fireId , &fireDelta)>=3){
                    wearingToBullet[firingWearingId] = wearingBullet(fireId , fireDelta);
                }
            }
        }
        fclose(fp);
    }else{
        printf("[error]fail to load 'wearing_bullet'\n" );
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
                                            if(strcmp(item->string,"texture")==0){
                                                ptr->texture= driver->getTexture(item->valuestring);
                                            }
                                        }else
                                        if(item->type==cJSON_Array){
                                            if(strcmp(item->string,"attach")==0){
                                                cJSON * line = item->child;
                                                while(line){
                                                    if(line->type==cJSON_Array){
                                                        auto k = cJSON_GetArrayItem(line,0);
                                                        auto v = cJSON_GetArrayItem(line,1);
                                                        if(k && v && k->type==cJSON_Number && v->type==cJSON_String){
                                                            ptr->attach[k->valueint] = v->valuestring;
                                                        }
                                                    }
                                                    line = line->next;
                                                }
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
void body::loadBagIcons(){
    printf("[status]get bag_icon config\n" );
    QFile file("../config/bag_icon.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/bag_icon.json\n" );
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
                    auto texnode = cJSON_GetObjectItem(c,"icon");
                    if(idnode && texnode && idnode->type==cJSON_Number && texnode->type==cJSON_String){
                        auto tex = driver->getTexture(texnode->valuestring);
                        if(tex){
                            auto istool = cJSON_GetObjectItem(c,"istool");
                            if(istool && istool->type==cJSON_Number && istool->valueint!=0){
                                bag_tool_icons_mapping[idnode->valueint] = bag_icons->addTextureAsSprite(tex);
                            }else{
                                bag_res_icons_mapping[idnode->valueint] = bag_icons->addTextureAsSprite(tex);
                            }
                        }
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/bag_icon.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}

void body::loadFireCost(){
    printf("[status]get fire_cost config\n" );
    QFile file("../config/fire_cost.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/fire_cost.json\n" );
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
                if(c->type == cJSON_Object){
                    auto id = cJSON_GetObjectItem(c,"id");
                    auto cost_num = cJSON_GetObjectItem(c,"cost_num");
                    auto cost_id  = cJSON_GetObjectItem(c,"cost_id");
                    auto get_num = cJSON_GetObjectItem(c,"get_num");
                    auto get_id  = cJSON_GetObjectItem(c,"get_id");
                    auto dur_cost = cJSON_GetObjectItem(c,"dur_cost");
                    if(id && cost_num && cost_id && id->type==cJSON_Number && cost_num->type==cJSON_Number && cost_id->type==cJSON_Number){
                        fire_cost f;
                        f.id        = id->valueint;
                        f.cost_id   = cost_id->valueint;
                        f.cost_num  = cost_num->valueint;
                        if(get_num && get_id && get_num->type==cJSON_Number && get_id->type==cJSON_Number){
                            f.get_id  = get_id->valueint;
                            f.get_num = get_num->valueint;
                        }
                        if(dur_cost && dur_cost->type == cJSON_Number){
                            f.dur_cost = dur_cost->valueint;
                        }
                        fire_costs[f.id] = f;
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/fire_cost.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}

void body::releaseBodyConfig(){
    for(auto it:bodyConfig){
        //it.second->mesh->drop();
        delete it.second;
    }
    bodyConfig.clear();
}

void body::releaseWearingConfig(){
    for(auto it:wearingConfig){
        //it.second->mesh->drop();
        delete it.second;
    }
    wearingConfig.clear();
}

}
