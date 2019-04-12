#include "user.h"
namespace smoothly{

void users::setOwner(const std::string & subsuuid,const std::string & useruuid){
    uLocker.lock(useruuid);
    //set list
    char kbuf[256];
    snprintf(kbuf,256,"userSubsList_%s_",useruuid.c_str());
    dblist l;
    l.db=db;
    l.prefix=kbuf;
    l.pushBegin(subsuuid);
    //set db
    int n=getUserNumSubs(useruuid);
    setUserNumSubs(useruuid,n+1);
    uLocker.unlock(useruuid);
}
void users::removeFromOwner(const std::string & subsuuid,const std::string & useruuid){
    uLocker.lock(useruuid);
    //set list
    char kbuf[256];
    snprintf(kbuf,256,"userSubsList_%s_",useruuid.c_str());
    dblist l;
    l.db=db;
    l.prefix=kbuf;
    l.del(subsuuid);
    //set db
    int n=getUserNumSubs(useruuid);
    setUserNumSubs(useruuid,n-1);
    uLocker.unlock(useruuid);
}
bool users::userCanPutSubs(const std::string & useruuid){
    uLocker.lock(useruuid);
    bool res=false;
    auto p=seekOnLine(useruuid);
    if(p){
        res=p->subsNum < getUserMaxSubs();
    }
    uLocker.unlock(useruuid);
    return res;
}
void users::moveUserPosition(const std::string & subsuuid,const std::string & uuid,const irr::core::vector3df & posi){
    uLocker.lock(uuid);
    auto p=seekOnLine(uuid);
    if(p){
        if(!p->subsUUID.empty() && p->subsUUID==subsuuid){
            setUserPosition(posi,p->address);
        }
    }
    uLocker.unlock(uuid);
}

bool users::userOnline(const std::string & uuid){
    auto p=seekOnLine(uuid);
    return p!=NULL;
}

bool users::login(const std::string & uuid,const RakNet::SystemAddress & addr,const std::string & pwd){
    uLocker.lock(uuid);
    
    if(!checkPasswd(uuid,pwd))
        return false;
    
    auto p=seekOnLine(uuid);
    if(p){
        delListener(p->address);
    }else{
        p=new user;
    }
    
    p->parent=this;
    p->userUUID=uuid;
    p->address=addr;
    p->update();
    
    std::string suuid=p->subsUUID;
    
    //bind listener
    irr::core::vector3df posi;
    getSubsPosition(posi,suuid);
    setUserPosition(posi,addr);
    
    //insert into list
    pthread_rwlock_wrlock(&rwlock);
    userUUIDs[uuid]=p;
    userAddrs[addr]=uuid;
    pthread_rwlock_unlock(&rwlock);
    uLocker.unlock(uuid);
    
    sendSubs(addr,suuid);
    sendSubsList(addr,uuid);
    sendSetUserSubs(addr,suuid);
    ++onLineNum;
    return true;
}
void users::logout(const std::string & uuid){
    uLocker.lock(uuid);
    auto p=seekOnLine(uuid);
    if(p){
        delListener(p->address);
        pthread_rwlock_wrlock(&rwlock);
        userUUIDs.erase(p->userUUID);
        userAddrs.erase(p->address);
        pthread_rwlock_unlock(&rwlock);
        delete p;
    }
    uLocker.unlock(uuid);
    --onLineNum;
}

void users::createUser(
    std::string & uuid ,
    const std::string & pwd , 
    irr::core::vector3df position ,
    long id
){
    getUUID(uuid);
    
    std::string suuid;
    createSubsForUSer(id,position,uuid,suuid);
    
    char kbuf[256];
    snprintf(kbuf , 256 , "userSubsUUID %s" , uuid.c_str());
    db->Put(leveldb::WriteOptions() , kbuf , suuid);
    
    setPwd(uuid,pwd);
}

void users::setPwd(const std::string & uuid,const std::string & pwd){
    char kbuf[256];
    snprintf(kbuf,256,"userPasswd %s",uuid.c_str());
    db->Put(leveldb::WriteOptions(),kbuf,pwd);
}
bool users::getPwd(const std::string & uuid,std::string & pwd){
    char kbuf[256];
    snprintf(kbuf,256,"userPasswd %s",uuid.c_str());
    return db->Get(leveldb::ReadOptions(),kbuf,&pwd).ok();
}
void users::changePwd(const std::string & uuid ,const std::string & opwd ,const std::string & npwd){
    uLocker.lock(uuid);
    std::string s;
    if(getPwd(uuid,s) && !s.empty()){
        if(s==opwd){
            setPwd(uuid,npwd);
        }
    }
    uLocker.unlock(uuid);
}
void users::getChunkSubs(const std::string & uuid, const RakNet::SystemAddress & address , int x,int y){
    uLocker.lock(uuid);
    sendSubs(address,x,y);
    uLocker.unlock(uuid);
}

bool users::checkPasswd(const std::string & uuid,const std::string & pwd){
    bool rflag=false;
    std::string s;
    if(getPwd(uuid,s) && !s.empty()){
        if(s==pwd){
            rflag=true;
        }
    }
    return rflag;
}
users::user * users::seekOnLine(const std::string & uuid){
    user * p=NULL;
    pthread_rwlock_rdlock(&rwlock);
    auto it=userUUIDs.find(uuid);
    if(it!=userUUIDs.end())
        p=it->second;
    pthread_rwlock_unlock(&rwlock);
    return p;
}
bool users::getUUIDByAddr(std::string & uuid,const RakNet::SystemAddress & address){
    bool rflag=false;
    
    pthread_rwlock_rdlock(&rwlock);
    auto it=userAddrs.find(address);
    if(it!=userAddrs.end()){
        uuid=it->second;
        rflag=true;
    }
    pthread_rwlock_unlock(&rwlock);
    return rflag;
}
int users::getUserNumSubs(const std::string & uuid){
    std::string sbuf;
    char kbuf[256];
    snprintf(kbuf,256,"userSubsNum %s",uuid.c_str());
    
    if(db->Get(leveldb::ReadOptions(),kbuf,&sbuf).ok() && !sbuf.empty()){
        return atoi(sbuf.c_str());
    }else
        return 0;
}
void users::setUserNumSubs(const std::string & uuid,int num){
    std::string sbuf;
    char kbuf[256];
    char nbuf[64];
    snprintf(kbuf,256,"userSubsNum %s",uuid.c_str());
    snprintf(nbuf,64,"%d",num);
    db->Put(leveldb::WriteOptions(),kbuf,nbuf);
}
void users::sendSubsList(const RakNet::SystemAddress & address,const std::string & useruuid){
    uLocker.lock(useruuid);
    //set list
    char kbuf[256];
    snprintf(kbuf,256,"userSubsList_%s_",useruuid.c_str());
    dblist l;
    l.db=db;
    l.prefix=kbuf;
    
    l.seekBegin();
    
    std::list<std::string> uuids;
    while(1){
        if(!l.key_now.empty()){
            //sendUserSubsUUID(l.key_now , address);
            uuids.push_back(l.key_now);
        }
        if(!l.next())
            break;
    }
    sendUserSubsUUIDs(uuids , address);
    
    uLocker.unlock(useruuid);
}
void users::user::update(){
    getSubsUUID();
}
void users::user::getSubsUUID(){
    char kbuf[256];
    snprintf(kbuf,256,"userSubsUUID %s",userUUID.c_str());
    parent->db->Get(leveldb::ReadOptions(),kbuf,&subsUUID);
}
void users::user::save(){
    char kbuf[256];
    snprintf(kbuf,256,"userSubsUUID %s",userUUID.c_str());
    parent->db->Put(leveldb::WriteOptions(),kbuf,subsUUID);
}

}