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
}

void users::login(const std::string & uuid,const RakNet::SystemAddress & addr,const std::string & pwd){
    uLocker.lock(uuid);
    if(seekOnLine(uuid))
        return;
    if(!checkPasswd(uuid,pwd))
        return;
    
    auto p=new user;
    p->userUUID=uuid;
    p->update();
    
    pthread_rwlock_wrlock(&rwlock);
    userUUIDs[uuid]=p;
    userAddrs[addr]=uuid;
    pthread_rwlock_unlock(&rwlock);
    uLocker.unlock(uuid);
    
    sendSubs(addr,uuid);
}
void users::logout(const std::string & uuid){
    uLocker.lock(uuid);
    auto p=seekOnLine(uuid);
    if(p){
        pthread_rwlock_wrlock(&rwlock);
        userUUIDs.erase(p->userUUID);
        userAddrs.erase(p->address);
        pthread_rwlock_unlock(&rwlock);
        delete p;
    }
    uLocker.unlock(uuid);
}
void users::createUser(const std::string & uuid,const std::string & pwd){
    uLocker.lock(uuid);
    
    uLocker.unlock(uuid);
}
void users::changePwd(const std::string & uuid,const std::string & pwd){
    uLocker.lock(uuid);
    
    uLocker.unlock(uuid);
}
void users::getChunkSubs(const std::string & uuid, const RakNet::SystemAddress address , int x,int y){
    uLocker.lock(uuid);
    
    uLocker.unlock(uuid);
}

bool users::checkPasswd(const std::string & uuid,const std::string & pwd){
    
}
users::user * users::seekOnLine(const std::string & uuid){
    pthread_rwlock_rdlock(&rwlock);
    
    pthread_rwlock_unlock(&rwlock);
}
int users::getUserNumSubs(const std::string & uuid){
    std::string sbuf;
    char kbuf[256];
    snprintf(kbuf,256,"userSubsNum %s",uuid.c_str());
    
    if(db->Get(leveldb::ReadOptions(),kbuf,&sbuf).ok() && !sbuf.empty()){
        return atoi(sbuf.c_str());
    }
}
void users::setUserNumSubs(const std::string & uuid,int num){
    std::string sbuf;
    char kbuf[256];
    char nbuf[64];
    snprintf(kbuf,256,"userSubsNum %s",uuid.c_str());
    snprintf(nbuf,64,"%d",num);
    db->Put(leveldb::WriteOptions(),kbuf,nbuf);
}

}