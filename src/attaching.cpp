#include "attaching.h"
namespace smoothly{

void attaching::setAttaching(
    const std::string & user , 
    const std::string & subs , 
    bodyAttaching & att//不声明const是因为可以被回调函数修改
){
    auto p=seekSubs(subs);
    if(p==NULL)
        return;
    
    if(!setAttachingCallback(user,p,att)){
        p->drop();
        return;
    }
    
    setAttachingToDB(subs,att);
    
    boardcastAttaching(subs,att,p->position);
    
    p->drop();
}

void attaching::getAttaching(const std::string & subs,const RakNet::SystemAddress & to){
    bodyAttaching att;
    
    getAttachingFromDB(subs,att);
    
    sendAttaching(subs,att,to);
}

/*
此函数因存在大量业务逻辑，被单独定义为so文件
bool attaching::setAttachingCallback(
    const std::string & user , 
    subs * p, 
    bodyAttaching & att//不声明const是因为可以被回调函数修改
){
    
}
*/

void attaching::getAttachingFromDB(const std::string & subs,bodyAttaching & att){
    std::string str;
    //read
    db->Get(leveldb::ReadOptions(),subs,&str);
    //parse
    att.clear();
    att.loadString(str);
}

void attaching::delAttaching(const std::string & subs){
    db->Delete(leveldb::WriteOptions(),subs);
}

void attaching::setAttachingToDB(const std::string & subs,const bodyAttaching & att){
    std::string str;
    att.toString(str);
    //write
    db->Put(leveldb::WriteOptions(),subs,str);
}

void attaching::attachingInit(const char * path){
    leveldb::Options opt;
    opt.create_if_missing=true;
    leveldb::DB::Open(opt,path,&this->db);
}

void attaching::attachingDestroy(){
    delete this->db;
}

}