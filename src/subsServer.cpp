#include "subsServer.h"
namespace smoothly{
void subsServer::subsInit(const char * path){
    leveldb::Options opt;
    opt.create_if_missing=true;
    leveldb::DB::Open(opt,path,&this->db);
}
void subsServer::subsDestroy(){
    delete this->db;
}
subsServer::subsServer(){
    pthread_rwlock_init(&rwlock, NULL);
}

subsServer::~subsServer(){
    pthread_rwlock_destroy(&rwlock);
}
void subsServer::teleport(
    const std::string & uuid,
    const irr::core::vector3df & posi,
    bool checkOwner,
    const std::string & owner
){
    pthread_rwlock_wrlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        if(!checkOwner || owner==p->userUUID){
            //set memory
            p->position=posi;
            p->checkPosition();
            
            p->lin_vel=btVector3(0,0,0);
            p->ang_vel=btVector3(0,0,0);
            
            p->updateChunkPosition();
            //set database
            p->save(false,true);
            moveUserPosition(uuid,p->userUUID,posi);
            //printf("[subsServer]teleport substance %s to (%f,%f,%f)\n",p->userUUID.c_str(),posi.X,posi.Y,posi.Z);
        }
        p->drop();
    }
    
    pthread_rwlock_unlock(&rwlock);
}
void subsServer::setSubs(
    const std::string & uuid,
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
    const btVector3& lin_vel ,
    const btVector3& ang_vel ,
    int status,
    const std::string & muuid
){//move a lasting substance
    pthread_rwlock_wrlock(&rwlock);
    
    bool resetMan=false;
    
    auto p=seekSubs(uuid);
    if(p){
        if(p->manager=="NULL" || p->manager.empty() || !userOnline(p->manager)){
            p->manager=muuid;
            p->lastChMan=cache::getTime();
            resetMan=true;
        }
        if(resetMan || p->manager==muuid){
            //set memory
            p->position=posi;
            p->checkPosition();
            
            p->rotation=rota;
            p->direction=dire;
            p->lin_vel=lin_vel;
            p->ang_vel=ang_vel;
            p->status=status;
            p->updateChunkPosition();
            //set database
            p->save();
            moveUserPosition(uuid,p->userUUID,posi);
        }else{
            if(p->userUUID==p->manager && userOnline(p->manager)){//owner is online
                
            }else{
                if(p->userUUID==muuid){
                    p->manager=muuid;
                    p->lastChMan=cache::getTime();
                    p->save();
                }else
                if(fabs(p->lastChMan-cache::getTime())>60){
                    if(((int)(posi.X+posi.Y+posi.Z)+cache::getTime())%8 == 1){
                        p->manager=muuid;
                        p->lastChMan=cache::getTime();
                        p->save();
                    }
                }
            }
        }
        p->drop();
    }
    
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::createSubsForUSer(
    long id,
    const irr::core::vector3df & posi,
    const std::string & muuid,
    std::string & subsuuid
){
    pthread_rwlock_wrlock(&rwlock);
    
    bool lasting;
    int hp;
    
    if(getSubsConf(id,lasting,hp)){
        if(lasting){
            auto p=new subs();
            p->id       =id;
            p->parent   =this;
            p->userUUID =muuid;
            p->manager  =muuid;
            p->status   =0;
            
            p->position =posi;
            p->checkPosition();
            
            p->rotation =irr::core::vector3df(0,0,0);
            p->direction =irr::core::vector3df(0,0,1);
            p->lin_vel.setValue(0,0,0);
            p->ang_vel.setValue(0,0,0);
            p->hp       =hp;
            p->updateChunkPosition();
            genuuid:
            p->genUUID();
            if(uuidExist(p->uuid))
                goto genuuid;
        
            subsCache.put(p->uuid,p);
            subsuuid=p->uuid;
            
            boardcastSubsCreate(p->uuid,id,posi,irr::core::vector3df(0,0,0),irr::core::vector3df(0,0,1),btVector3(0,0,0),btVector3(0,0,0),muuid,"");
            
            p->setConfig("");
            setOwner(p->uuid,muuid);
            p->save(true);
            p->saveToDB();
            p->drop();
        }
    }
    
    pthread_rwlock_unlock(&rwlock);
}
void subsServer::createSubs(//添加物体
    long id , 
    const irr::core::vector3df & posi,
    const irr::core::vector3df & rota, 
    const irr::core::vector3df & dire, 
    const btVector3& impulse,
    const btVector3& rel_pos,
    const std::string & muuid,
    const std::string & conf,
    const RakNet::SystemAddress & from
){
    pthread_rwlock_wrlock(&rwlock);
    
    bool lasting;
    int hp;
    std::string c=conf;
    if(c=="[LOADING]")
        c="";
    if(getSubsConf(id,lasting,hp)){
        if(lasting){
            if(userCanPutSubs(muuid)){
                auto p=new subs();
                p->id       =id;
                p->parent   =this;
                p->userUUID =muuid;
                p->manager  =muuid;
                p->status   =0;
                
                p->position =posi;
                p->checkPosition();
                
                p->rotation =rota;
                p->direction=dire;
                
                p->lin_vel.setValue(0,0,0);
                p->ang_vel.setValue(0,0,0);
                p->hp       =hp;
                p->updateChunkPosition();
                genuuid:
                p->genUUID();
                if(uuidExist(p->uuid))
                    goto genuuid;
            
                subsCache.put(p->uuid,p);
                
                boardcastSubsCreate(p->uuid,id,posi,rota,dire,impulse,rel_pos,muuid,c);
                
                p->setConfig(c);
                setOwner(p->uuid,muuid);
                p->save(true);
                p->saveToDB();
                p->drop();
            }else
                sendPutSubsFail(from);
        }else{
            boardcastSubsCreate(id,posi,rota,dire,impulse,rel_pos,muuid,c,from);
        }
    }
    
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::giveUpControl(const std::string & uuid,const std::string & muuid){
    pthread_rwlock_wrlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        if(p->manager==muuid){
            p->manager="NULL";
            p->lastChMan=0;
        }
        p->save();
        p->drop();
    }
    pthread_rwlock_unlock(&rwlock);
}
void subsServer::changeManager(const std::string & uuid,const std::string & muuid){
    pthread_rwlock_wrlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        if(p->manager==p->userUUID){
            
        }else{
            p->manager=muuid;
            p->lastChMan=cache::getTime();
            p->save();
        }
        p->drop();
    }
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::removeSubs(const std::string & uuid){
    pthread_rwlock_wrlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        p->remove();
        p->drop();
    }
    subsCache.del(uuid);
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::getSubsPosition(irr::core::vector3df & posi,const std::string & uuid){
    pthread_rwlock_rdlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        posi=p->position;
        p->drop();
    }
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::attackSubs(const std::string & uuid,int dmg){
    pthread_rwlock_wrlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        p->hp-=dmg;
        if(p->hp <= 0){
            p->remove();
            subsCache.del(uuid);
        }else
            boardcastSubsAttack(uuid,p->position,p->hp,dmg);
        p->drop();
    }
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::sendSubs(const RakNet::SystemAddress & addr,const std::string & uuid){
    pthread_rwlock_rdlock(&rwlock);
    auto p=seekSubs(uuid);
    if(p){
        p->send(addr,true);
        p->drop();
    }
    pthread_rwlock_unlock(&rwlock);
}

void subsServer::sendSubs(const RakNet::SystemAddress & addr,int x,int y){
    pthread_rwlock_rdlock(&rwlock);
    char kbuf[256];
    snprintf(kbuf,256,"subsChunkList_%d_%d_",x,y);
    dblist l;
    l.db=db;
    l.prefix=kbuf;
    l.seekBegin();
    while(1){
        if(!l.key_now.empty()){
            auto p=seekSubs(l.key_now);
            if(p){
                p->send(addr);
                p->drop();
            }
        }
        if(!l.next())
            break;
    }
    pthread_rwlock_unlock(&rwlock);
    sendChunkRun(x,y,addr);
}
subsServer::subs * subsServer::seekSubs(const std::string & uuid){
    auto p=(subs*)subsCache.get(uuid);
    if(p==NULL)
        p=createSubs(uuid);
    return p;
}

subsServer::subs * subsServer::createSubs(const std::string & uuid){
    std::string sbuf;
    char kbuf[256];
    snprintf(kbuf,256,"subsNode %s",uuid.c_str());
    
    if(db->Get(leveldb::ReadOptions(),kbuf,&sbuf).ok() && !sbuf.empty()){
        auto p=new subs();
        p->uuid=uuid;
        p->parent=this;
        p->decode(sbuf.c_str());
        p->updateChunkPosition();
        subsCache.put(uuid,p);
        return p;
    }else
        return NULL;
}

bool subsServer::uuidExist(const std::string & uuid){
    std::string sbuf;
    char kbuf[256];
    snprintf(kbuf,256,"subsNode %s",uuid.c_str());
    
    return (db->Get(leveldb::ReadOptions(),kbuf,&sbuf).ok() && !sbuf.empty());
}

void subsServer::subs::delConfig(){
    char kbuf[256];
    snprintf(kbuf,256,"config %s",uuid.c_str());
    parent->db->Delete(leveldb::WriteOptions(),kbuf);
}
void subsServer::subs::setConfig(const std::string & conf){
    char kbuf[256];
    snprintf(kbuf,256,"config %s",uuid.c_str());
    if(conf=="[LOADING]")
        parent->db->Put(leveldb::WriteOptions(),kbuf,"");
    else
        parent->db->Put(leveldb::WriteOptions(),kbuf,conf);
}
void subsServer::subs::getConfig(std::string & conf){
    char kbuf[256];
    snprintf(kbuf,256,"config %s",uuid.c_str());
    parent->db->Get(leveldb::ReadOptions(),kbuf,&conf);
}

void subsServer::subs::updateChunkPosition(){
    x=(int)(position.X/32);
    y=(int)(position.Z/32);
}
void subsServer::subs::genUUID(){
    getUUID(uuid);
}
void subsServer::subs::encode(char * vbuf,int len){
    snprintf(vbuf,len,
        "%ld %d %d "
        "%f %f %f "
        "%f %f %f "
        "%f %f %f "
        "%f %f %f "
        "%s %s "
        "%f %f %f ",
        id,hp,status,
        position.X,position.Y,position.Z ,
        rotation.X,rotation.Y,rotation.Z ,
        lin_vel.getX(),lin_vel.getY(),lin_vel.getZ() ,
        ang_vel.getX(),ang_vel.getY(),ang_vel.getZ() ,
        userUUID.c_str() , manager.c_str() ,
        direction.X,direction.Y,direction.Z 
    );
}
void subsServer::subs::decode(const char * vbuf){
    float fbuf;
    std::istringstream iss(vbuf);
    iss>>id;
    iss>>hp;
    iss>>status;
    
    iss>>position.X;
    iss>>position.Y;
    iss>>position.Z;
    
    iss>>rotation.X;
    iss>>rotation.Y;
    iss>>rotation.Z;
    
    fbuf=0;iss>>fbuf;lin_vel.setX(fbuf);
    fbuf=0;iss>>fbuf;lin_vel.setY(fbuf);
    fbuf=0;iss>>fbuf;lin_vel.setZ(fbuf);
    
    fbuf=0;iss>>fbuf;ang_vel.setX(fbuf);
    fbuf=0;iss>>fbuf;ang_vel.setY(fbuf);
    fbuf=0;iss>>fbuf;ang_vel.setZ(fbuf);
    
    userUUID.clear();
    iss>>userUUID;
    
    manager.clear();
    iss>>manager;
    
    iss>>direction.X;
    iss>>direction.Y;
    iss>>direction.Z;
    
    checkPosition();
}

void subsServer::subs::save(bool updateChunk,bool tp){
    int tx,ty;
    tx=x;
    ty=y;
    updateChunkPosition();
    if(updateChunk){
        addIntoChunk(x,y);
    }else
    if(tx!=x || ty!=y){
        removeFromChunk(tx,ty);
        addIntoChunk(x,y);
    }
    if(tp){
        parent->boardcastTeleport(uuid,position);
    }else
        send();
}

void subsServer::subs::saveDo(){
    save();
    saveToDB();
}
void subsServer::subs::saveToDB(){
    //printf("[substance]save to db %s\n",uuid.c_str());
    char kbuf[256];
    char vbuf[2048];
    encode(vbuf,sizeof(vbuf));
    snprintf(kbuf,256,"subsNode %s",uuid.c_str());
    parent->db->Put(leveldb::WriteOptions(),kbuf,vbuf);
}
void subsServer::subs::load(){
    std::string sbuf;
    char kbuf[256];
    snprintf(kbuf,256,"subsNode %s",uuid.c_str());
    
    if(parent->db->Get(leveldb::ReadOptions(),kbuf,&sbuf).ok() && !sbuf.empty()){
        decode(sbuf.c_str());
    }
    
}
void subsServer::subs::remove(){
    parent->boardcastSubsRemove(uuid,position);
    removeFromChunk(x,y);
    parent->removeFromOwner(uuid,userUUID);
    parent->delAttaching(uuid);
    char kbuf[256];
    snprintf(kbuf,256,"subsNode %s",uuid.c_str());
    parent->db->Delete(leveldb::WriteOptions(),kbuf);
    delConfig();
}
void subsServer::subs::send(){
    RakNet::SystemAddress ext;
    if(parent->getAddrByUUID(ext,manager))//不给操控者发，防止锁死物体
        parent->boardcastSubsStatus(uuid,id,position,rotation,direction,lin_vel,ang_vel,status,hp,userUUID,ext);
    else
        parent->boardcastSubsStatus(uuid,id,position,rotation,direction,lin_vel,ang_vel,status,hp,userUUID,RakNet::SystemAddress());
}
void subsServer::subs::send(const RakNet::SystemAddress & addr,bool sendconf){
    std::string conf;
    if(sendconf){
        getConfig(conf);
        if(conf=="[LOADING]")
            conf="";
    }else
        conf="[LOADING]";
    parent->sendSubsStatus(uuid,id,position,rotation,direction,lin_vel,ang_vel,status,hp,userUUID,conf,addr);
}
void subsServer::subs::onFree(){
    saveDo();
}
void subsServer::subs::removeFromChunk(int x,int y){
    char kbuf[256];
    snprintf(kbuf,256,"subsChunkList_%d_%d_",x,y);
    dblist l;
    l.db=parent->db;
    l.prefix=kbuf;
    l.del(uuid);
}
void subsServer::subs::addIntoChunk(int x,int y){
    char kbuf[256];
    snprintf(kbuf,256,"subsChunkList_%d_%d_",x,y);
    dblist l;
    l.db=parent->db;
    l.prefix=kbuf;
    l.pushBegin(uuid);
}

}
