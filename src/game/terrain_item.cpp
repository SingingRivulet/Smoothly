#include "terrain_item.h"
#include <stdio.h>
namespace smoothly{

#define findChunk(x,y) \
    auto it = chunks.find(ipair(x,y)); \
    if(it!=chunks.end())

void terrain_item::setRemoveTable(int x,int y,const std::set<mapItem> & rmt){
    chunk * ptr;
    if(!chunkLoaded(x,y))//chunk未被创建，将不再创建items
        return;
    findChunk(x,y){
        ptr = it->second;
        releaseChildren(ptr);
    }else{
        ptr = new chunk;
        ptr->x = x;
        ptr->y = y;
        chunks[ipair(x,y)]=ptr;
    }
    std::list<genProb> pl;//物体->概率
    world::terrain::predictableRand randg;
    
    int tem = getTemperatureF(x,y);
    int hu  = getHumidityF(x,y);
    
    getGenList(x,y,tem,hu,getRealHight(x*32,y*32),pl);
    
    randg.setSeed((x+10)*(y+20)*(hu+30)*(tem+40));
    
    for(auto it:pl){
        int delta=it.prob*1000;
        for(int i=0;i<it.num;i++){
            int pr=(randg.rand())%1000;
            if(pr>delta){
                float mx=(randg.frand()+x)*32;
                float my=(randg.frand()+y)*32;
                float mr=randg.frand()*360;
                if(rmt.find(mapItem(it.id , i))!=rmt.end())
                    continue;
                
                auto im=makeTerrainItem(it.id , i , mx , my , mr);
                if(im==NULL)
                    continue;
                
                ptr->children[mapItem(it.id , i)]=im;
                im->parent = ptr;
            }
        }
    }
}

void terrain_item::releaseChunk(int x,int y){
    terrain::releaseChunk(x,y);
    auto it = chunks.find(ipair(x,y));
    if(it!=chunks.end()){
        releaseChildren(it->second);
        chunks.erase(it);
        delete it->second;
    }
}
void terrain_item::releaseChunk(chunk * ch){
    releaseChildren(ch);
    chunks.erase(ipair(ch->x,ch->y));
    delete ch;
}
void terrain_item::releaseAllChunk(){
    for(auto it:chunks){
        releaseChildren(it.second);
        delete it.second;
    }
    chunks.clear();
}
void terrain_item::releaseChildren(chunk * ch){
    for(auto it:ch->children){
        releaseTerrainItem(it.second);
    }
    ch->children.clear();
}
void terrain_item::removeTerrainItem(chunk * ch,int index,int id){
    auto it = ch->children.find(mapItem(index,id));
    if(it!=ch->children.end()){
        releaseTerrainItem(it->second);
        ch->children.erase(it);
    }
}
void terrain_item::removeTerrainItem(int x , int y ,int index,int id){
    findChunk(x,y){
        removeTerrainItem(it->second , index , id);
    }
}
void terrain_item::releaseTerrainItems(int x , int y){
    findChunk(x,y){
        releaseChildren(it->second);
        delete it->second;
        chunks.erase(it);
    }
}
terrain_item::item * terrain_item::makeTerrainItem(int id,int index,float x,float y,float r){
    auto it = config.find(id);
    if(it==config.end())
        return NULL;
    
    auto res = new item;
    res->id.x=x;
    res->id.y=y;
    res->id.id.id=id;
    res->id.id.index=index;

    int ml=3;
    for(int i = 0;i<4;++i){
        if(it->second->mesh[i]==NULL){
            ml=i-1;
            if(ml<0)
                ml=0;
            break;
        }
        res->node[i]=scene->addMeshSceneNode(
                    it->second->mesh[i],NULL,
                    -1,
                    vec3(x,getRealHight(x,y)+it->second->deltaHeight,y),
                    vec3(0,r,0),
                    it->second->scale
                    );
        res->node[i]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
        res->node[i]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
        res->node[i]->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        res->node[i]->addShadowVolumeSceneNode();

        res->node[i]->updateAbsolutePosition();//更新矩阵

        if(it->second->useShader){
            res->node[i]->setMaterialType((irr::video::E_MATERIAL_TYPE)it->second->shader);
        }
    }
    res->node[ml]->setVisible(true);//显示最低lod级别

    if(it->second->haveBody){
        res->bodyState=setMotionState(res->node[0]->getAbsoluteTransformation().pointer());//创建状态
        res->rigidBody=createBody(it->second->shape.compound,res->bodyState);//创建物体
        res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);//设置碰撞模式
        
        res->info.type=BODY_TERRAIN_ITEM;//设置用户数据，指向mapId
        res->info.ptr=&res->id;
        res->rigidBody->setUserPointer(&(res->info));
        
        dynamicsWorld->addRigidBody(res->rigidBody);//添加物体
    }else{
        res->bodyState=NULL;
        res->rigidBody=NULL;
    }
    
    return res;
}

void terrain_item::updateLOD(int x, int y, int lv){
    terrain::updateLOD(x,y,lv);
    findChunk(x,y){
        if(lv==0){
            for(auto c:it->second->children){
                item * im = c.second;
                for(int i=0;i<4;++i){
                    if(im->node[i])
                        im->node[i]->setVisible(false);
                }
            }
        }else{
            for(auto c:it->second->children){
                item * im = c.second;
                for(int i=0;i<4;++i){
                    if(im->node[i])
                        im->node[i]->setVisible(false);
                }
                for(int i=lv-1;i>=0;--i){
                    //反向遍历lod列表
                    if(im->node[i]){
                        //找到可用的最大lod
                        im->node[i]->setVisible(true);
                        break;
                    }
                }
            }
        }
    }
}
void terrain_item::releaseTerrainItem(item * p){
    if(p->rigidBody){
        dynamicsWorld->removeRigidBody(p->rigidBody);
        delete p->rigidBody;
    }
    if(p->bodyState)
        delete p->bodyState;
    for(int i = 0;i<4;++i){
        if(p->node[i]){
            p->node[i]->remove();
        }
    }
    delete p;
}
terrain_item::terrain_item(){
    loadConfig();
}
terrain_item::~terrain_item(){
    releaseAllChunk();
    releaseConfig();
}
void terrain_item::loadConfig(){
    config.clear();
    //读取文件
    char* text;
    FILE *pf = fopen("../config/terrainItem.json","r");
    if(pf==NULL){
        printf("[error]terrain_item:load config fail\n");
        return;
    }
    fseek(pf,0,SEEK_END);
    long lSize = ftell(pf);
    if(lSize<=0){
        fclose(pf);
        printf("[error]terrain_item:load config fail\n");
        return;
    }
    text=(char*)malloc(lSize+1);// 用完后需要将内存free掉
    rewind(pf); 
    fread(text,sizeof(char),lSize,pf);
    text[lSize] = '\0';
    fclose(pf);
    //解析json
    cJSON * json=cJSON_Parse(text);
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;//遍历
            while (c){
                if(c->type==cJSON_Object){
                    loadJSON(c);
                }
                c=c->next;
            }
        }
        cJSON_Delete(json);
    }
    free(text);
}
void terrain_item::loadJSON(cJSON * json){
    auto id = cJSON_GetObjectItem(json,"id");
    if(id && id->type==cJSON_Number){
        auto it = config.find(id->valueint);
        if(it!=config.end())//已经存在
            return;
    }else
        return;
    
    auto path = cJSON_GetObjectItem(json,"mesh");
    
    irr::scene::IMesh * mesh;
    if(path && path->type==cJSON_String){
        mesh = scene->getMesh(path->valuestring);
        if(mesh==NULL)
            return;
    }else
        return;
    
    auto c          = new conf;
    c->haveBody     = false;
    c->deltaHeight  = 0;
    c->scale.set(1,1,1);
    c->useShader    = false;
    for(int i = 0;i<4;++i)
        c->mesh[i] = NULL;

    c->mesh[0] = mesh;
    
    auto body = cJSON_GetObjectItem(json,"body");
    if(body && body->type==cJSON_String){
        c->haveBody = true;
        c->shape.init(body->valuestring);
    }
    
    auto deltaHeight = cJSON_GetObjectItem(json,"deltaHeight");
    if(deltaHeight && deltaHeight->type==cJSON_Number){
        c->deltaHeight = deltaHeight->valuedouble;
    }
    
    auto scale = cJSON_GetObjectItem(json,"scale");
    if(scale && scale->type==cJSON_Object){
        
        auto sx = cJSON_GetObjectItem(scale , "x");
        if(sx && sx->type==cJSON_Number)c->scale.X = sx->valuedouble;
        
        auto sy = cJSON_GetObjectItem(scale , "y");
        if(sy && sy->type==cJSON_Number)c->scale.Y = sy->valuedouble;
        
        auto sz = cJSON_GetObjectItem(scale , "z");
        if(sz && sz->type==cJSON_Number)c->scale.Z = sz->valuedouble;
        
    }

    auto shader = cJSON_GetObjectItem(json,"shader");
    if(shader && shader->type==cJSON_Object){
        auto vs = cJSON_GetObjectItem(shader,"vs");
        auto ps = cJSON_GetObjectItem(shader,"ps");
        if(vs && ps && vs->type==cJSON_String && ps->type==cJSON_String){
            c->useShader = true;
            c->shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                        vs->valuestring, "main", irr::video::EVST_VS_1_1,
                        ps->valuestring, "main", irr::video::EPST_PS_1_1);
        }
    }
    
    config[id->valueint] = c;
}
void terrain_item::releaseConfig(){
    for(auto it:config){
        //it.second->mesh->drop();
        delete it.second;
    }
    config.clear();
}
void terrain_item::msg_addRemovedItem(int x,int y,int id,int index){
    removeTerrainItem(x,y,id,index);
}
void terrain_item::msg_setRemovedItem(int x,int y,const std::set<mapItem> & rmt){
    setRemoveTable(x,y,rmt);
}

}
