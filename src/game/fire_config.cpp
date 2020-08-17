#include "fire.h"

#include <QFile>
#include <QByteArray>

namespace smoothly{

vec3 json2vec(cJSON * json){
    vec3 res(0,0,0);

    cJSON * x = cJSON_GetObjectItem(json,"x");
    if(x->type==cJSON_Number)
        res.X = x->valuedouble;

    cJSON * y = cJSON_GetObjectItem(json,"y");
    if(y->type==cJSON_Number)
        res.Y = y->valuedouble;

    cJSON * z = cJSON_GetObjectItem(json,"z");
    if(z->type==cJSON_Number)
        res.Z = z->valuedouble;

    return res;
}

irr::video::SColor json2color(cJSON * json){
    int ra=255,rr=255,rg=255,rb=255;

    cJSON * r = cJSON_GetObjectItem(json,"r");
    if(r->type==cJSON_Number)
        rr = r->valueint;

    cJSON * g = cJSON_GetObjectItem(json,"g");
    if(g->type==cJSON_Number)
        rg = g->valueint;

    cJSON * b = cJSON_GetObjectItem(json,"b");
    if(b->type==cJSON_Number)
        rb = b->valueint;

    cJSON * a = cJSON_GetObjectItem(json,"a");
    if(a->type==cJSON_Number)
        ra = a->valueint;

    return irr::video::SColor(ra,rr,rg,rb);
}

irr::core::dimension2df json2dimension(cJSON * json){
    irr::core::dimension2df res(0,0);

    cJSON * w = cJSON_GetObjectItem(json,"w");
    if(w->type==cJSON_Number)
        res.Width = w->valuedouble;

    cJSON * h = cJSON_GetObjectItem(json,"h");
    if(h->type==cJSON_Number)
        res.Height = h->valuedouble;

    return res;
}

fire::fireConfig::fireConfig(){
    this->breakWhenHit  = true;

    this->bulletConf.billboardMode  = true;
    this->bulletConf.light          = true;
    this->bulletConf.have           = false;
    this->bulletConf.mesh           = NULL;
    this->bulletConf.texture        = NULL;

    this->startAudio                = NULL;
    this->flyAudio                  = NULL;

    this->castShape                 = NULL;
    this->id                        = 0;
    this->lifeTime                  = 4000;

    this->particleConfig.gravity.gravity    = vec3(0,-0.7,0);
    this->particleConfig.gravity.have       = false;
    this->particleConfig.gravity.timeForceLost = 1000;
    this->particleConfig.have       = false;
    this->particleConfig.lifeTimeMax= 2000;
    this->particleConfig.lifeTimeMin= 1000;
    this->particleConfig.light      = false;

    this->particleConfig.maxAngleDegrees = 90;

    this->particleConfig.minParticlesPerSecond = 5;
    this->particleConfig.maxParticlesPerSecond = 10;

    this->particleConfig.minStartColor.set(255,0,0,0);
    this->particleConfig.maxStartColor.set(255,255,255,255);

    this->particleConfig.minStartSize.set(5.0f , 5.0f);
    this->particleConfig.maxStartSize.set(5.0f , 5.0f);

    this->particleConfig.texture    = NULL;

    this->radius                    = 1;
    this->range                     = 256;
    this->streamParticleNum         = 256;
    this->streamParticleVelocity    = 4;
    this->type                      = FIRE_SHOOT;
    this->mass                      = 1;
    this->inertia.setValue(1,1,1);

    this->impulse                   = 100;
    this->billboardSize             = 5;

    this->timePerFrame              = 30;
    //this->shape
}
void fire::releaseConfig(fireConfig * c){
    if(c->castShape)
        delete c->castShape;
    //if(c->particleConfig.texture)
    //    c->particleConfig.texture->drop();
    //if(c->bulletConf.mesh)
    //    c->bulletConf.mesh->drop();
    //if(c->bulletConf.texture)
    //    c->bulletConf.texture->drop();
    delete c;
}

void fire::openConfig(){
    printf("[status]get fire config\n" );
    QFile file("../config/fire.json");
    if(!file.open(QFile::ReadOnly)){
        printf("[error]fail to read ../config/fire.json\n" );
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
                        if(config.find(id)!=config.end()){
                            printf("[error]can't redefine fire %d\n" , id);
                        }else{
                            ////////////////////////////////////////////////////////
                            auto p = new fireConfig;
                            p->id  = id;
                            //遍历json
                            cJSON * line = c->child;
                            while(line){
                                if(line->type==cJSON_Object){
                                    if(strcmp(line->string,"particleGravity")==0){
                                        p->particleConfig.gravity.gravity=json2vec(line);
                                    }else
                                    if(strcmp(line->string,"particleMinStartSize")==0){
                                        p->particleConfig.minStartSize=json2dimension(line);
                                    }else
                                    if(strcmp(line->string,"particleMaxStartSize")==0){
                                        p->particleConfig.maxStartSize=json2dimension(line);
                                    }else
                                    if(strcmp(line->string,"particleMinStartColor")==0){
                                        p->particleConfig.minStartColor=json2color(line);
                                    }else
                                    if(strcmp(line->string,"particleMaxStartColor")==0){
                                        p->particleConfig.maxStartColor=json2color(line);
                                    }else
                                    if(strcmp(line->string,"inertia")==0){
                                        auto v = json2vec(json);
                                        p->inertia.setValue(v.X , v.Y , v.Z);
                                    }
                                }else
                                if(line->type==cJSON_Array){
                                    if(strcmp(line->string,"textures")==0){
                                        auto i = line->child;
                                        while(i){
                                            if(i->type==cJSON_String){
                                                auto t = driver->getTexture(i->valuestring);
                                                if(t){
                                                    p->textures.push_back(t);
                                                }
                                            }
                                            i=i->next;
                                        }
                                    }
                                }else
                                if(line->type==cJSON_String){
                                    if(strcmp(line->string,"bulletMesh")==0){
                                        p->bulletConf.mesh=scene->getMesh(line->valuestring);
                                    }else
                                    if(strcmp(line->string,"bulletTexture")==0){
                                        p->bulletConf.texture=driver->getTexture(line->valuestring);
                                    }else
                                    if(strcmp(line->string,"shape")==0){
                                        p->shape.init(line->valuestring);
                                    }else
                                    if(strcmp(line->string,"castShape")==0){
                                        if(strcmp(line->valuestring,"capsule")==0){
                                            auto rad    = cJSON_GetObjectItem(c , "castShapeRad");
                                            auto height = cJSON_GetObjectItem(c , "castShapeHeight");
                                            if(rad!=NULL && height!=NULL && rad->type==cJSON_Number && height->type==cJSON_Number){
                                                p->castShape = new btCapsuleShape(rad->valuedouble , height->valuedouble);
                                            }
                                        }else
                                        if(strcmp(line->valuestring,"cylinder")==0){
                                            auto cbx     = cJSON_GetObjectItem(c , "castShapeX");
                                            auto cby     = cJSON_GetObjectItem(c , "castShapeY");
                                            auto cbz     = cJSON_GetObjectItem(c , "castShapeZ");
                                            if(
                                                    cbx!=NULL && cby!=NULL && cbz!=NULL &&
                                                    cbx->type==cJSON_Number && cby->type==cJSON_Number && cbz->type==cJSON_Number){
                                                p->castShape = new btCylinderShape(btVector3(cbx->valuedouble ,cby->valuedouble ,cbz->valuedouble));
                                            }
                                        }else
                                        if(strcmp(line->valuestring,"cone")==0){
                                            auto rad    = cJSON_GetObjectItem(c , "castShapeRad");
                                            auto height = cJSON_GetObjectItem(c , "castShapeHeight");
                                            if(rad!=NULL && height!=NULL && rad->type==cJSON_Number && height->type==cJSON_Number){
                                                p->castShape = new btConeShape(rad->valuedouble , height->valuedouble);
                                            }
                                        }else
                                        if(strcmp(line->valuestring,"box")==0){
                                            auto cbx     = cJSON_GetObjectItem(c , "castShapeX");
                                            auto cby     = cJSON_GetObjectItem(c , "castShapeY");
                                            auto cbz     = cJSON_GetObjectItem(c , "castShapeZ");
                                            if(
                                                    cbx!=NULL && cby!=NULL && cbz!=NULL &&
                                                    cbx->type==cJSON_Number && cby->type==cJSON_Number && cbz->type==cJSON_Number){
                                                p->castShape = new btBoxShape(btVector3(cbx->valuedouble ,cby->valuedouble ,cbz->valuedouble));
                                            }
                                        }else{
                                            auto rad    = cJSON_GetObjectItem(c , "castShapeRad");
                                            if(rad!=NULL && rad->type==cJSON_Number){
                                                p->castShape= new btSphereShape(rad->valuedouble);
                                            }
                                        }
                                    }else
                                    if(strcmp(line->string,"particleTexture")==0){
                                        p->particleConfig.texture=driver->getTexture(line->valuestring);
                                    }else
                                    if(strcmp(line->string,"type")==0){
                                        if(strcmp(line->valuestring,"stream")==0){
                                            p->type = FIRE_STREAM;
                                        }else
                                        if(strcmp(line->valuestring,"shoot")==0){
                                            p->type = FIRE_SHOOT;
                                        }else
                                        if(strcmp(line->valuestring,"chop")==0){
                                            p->type = FIRE_CHOP;
                                        }else
                                        if(strcmp(line->valuestring,"radio")==0){
                                            p->type = FIRE_RADIO;
                                        }else
                                        if(strcmp(line->valuestring,"laser")==0){
                                            p->type = FIRE_LASER;
                                        }
                                    }else
                                    if(strcmp(line->string,"startAudio")==0){
                                        p->startAudio = getAudioBuffer(line->valuestring);
                                    }else
                                    if(strcmp(line->string,"flyAudio")==0){
                                        p->flyAudio = getAudioBuffer(line->valuestring);
                                    }
                                }else
                                if(line->type==cJSON_Number){
                                    if(strcmp(line->string,"lifetime")==0){
                                        p->lifeTime=line->valueint;
                                    }else
                                    if(strcmp(line->string,"particleGravityTimeForceLost")==0){
                                        p->particleConfig.gravity.timeForceLost=line->valueint;
                                    }else
                                    if(strcmp(line->string,"particleLifeTimeMax")==0){
                                        p->particleConfig.lifeTimeMax=line->valueint;
                                    }else
                                    if(strcmp(line->string,"particleLifeTimeMin")==0){
                                        p->particleConfig.lifeTimeMin=line->valueint;
                                    }else
                                    if(strcmp(line->string,"particleMaxAngleDegrees")==0){
                                        p->particleConfig.maxAngleDegrees=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"particleMinParticlesPerSecond")==0){
                                        p->particleConfig.minParticlesPerSecond=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"particleMaxParticlesPerSecond")==0){
                                        p->particleConfig.maxParticlesPerSecond=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"radius")==0){
                                        p->radius=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"range")==0){
                                        p->range=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"streamParticleNum")==0){
                                        p->streamParticleNum=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"streamParticleVelocity")==0){
                                        p->streamParticleVelocity=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"breakWhenHit")==0){
                                        p->breakWhenHit=line->valueint;
                                    }else
                                    if(strcmp(line->string,"bulletLight")==0){
                                        p->bulletConf.light=line->valueint;
                                    }else
                                    if(strcmp(line->string,"haveBullet")==0){
                                        p->bulletConf.have=line->valueint;
                                    }else
                                    if(strcmp(line->string,"haveParticle")==0){
                                        p->particleConfig.have=line->valueint;
                                    }else
                                    if(strcmp(line->string,"haveParticleGravity")==0){
                                        p->particleConfig.gravity.have=line->valueint;
                                    }else
                                    if(strcmp(line->string,"haveParticleLight")==0){
                                        p->particleConfig.light=line->valueint;
                                    }else
                                    if(strcmp(line->string,"mass")==0){
                                        p->mass=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"impulse")==0){
                                        p->impulse=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"billboardSize")==0){
                                        p->billboardSize=line->valuedouble;
                                    }else
                                    if(strcmp(line->string,"timePerFrame")==0){
                                        p->timePerFrame=line->valueint;
                                    }
                                }
                                line=line->next;
                            }
                            ////////////////////////////////////////////////////////
                            config[id] = p;
                        }
                    }else{
                        printf("[error]can't get id\n");
                    }
                }
                c=c->next;
            }
        }else{
            printf("[error]root in ../config/fire.json is not Array!\n" );
        }
        cJSON_Delete(json);
    }else{
        printf("[error]fail to load json\n" );
    }
}
void fire::closeConfig(){
    for(auto it:config){
        releaseConfig(it.second);
    }
    config.clear();
}

}
