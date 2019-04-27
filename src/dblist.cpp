#include "dblist.h"
namespace smoothly{

void dblistBase::seek(const std::string & key){
    std::string lk,nk;
    this->key_now=key;
    getLastKey(lk , key);
    getNextKey(nk , key);
    readDb(lk,key_last);
    readDb(nk,key_next);
}

bool dblistBase::seekBegin(){
    std::string k,bk;
    getBeginKey(k);
    readDb(k,bk);
    if(bk.empty()){
        key_last.clear();
        key_next.clear();
        key_now.clear();
        return false;
    }else{
        this->seek(bk);
        return true;
    }
}

bool dblistBase::last(){
    if(key_last.empty())
        return false;
    else{
        this->seek(key_last);
        return true;
    }
}

bool dblistBase::next(){
    if(key_next.empty())
        return false;
    else{
        this->seek(key_next);
        return true;
    }
}

void dblistBase::del(const std::string & key){
    this->seek(key);
    if(!key_last.empty())
        setNext(key_last,key_next);
    else{
        setBegin(key_next);
    }
    if(!key_next.empty())
        setLast(key_next,key_last);
    delNode_unsafe(key);
    this->key_last.clear();
    this->key_next.clear();
    this->key_now.clear();
}

void dblistBase::pushBegin(const std::string & key){
    std::string k,bk;
    getBeginKey(k);
    readDb(k,bk);
    if(bk.empty()){
        setBegin(key);
    }else{
        setNext(key,bk);
        setLast(bk,key);
        setBegin(key);
    }
}

void dblistBase::clear(){
    seekBegin();
    std::string nk;
    getBeginKey(nk);
    delDb(nk);
    while(1){
        if(!key_now.empty()){
            delNode_unsafe(key_now);
            if(!next())
                break;
        }else
            break;
    }
}
void dblistBase::delNode_unsafe(const std::string & key){
    std::string lk,nk;
    getLastKey(lk , key);
    getNextKey(nk , key);
    delDb(lk);
    delDb(nk);
}
void dblistBase::onDel  (const std::string & key){
    
}

void dblistBase::getBeginKey(std::string & key){
    char buf[256];
    snprintf(buf,256,"%s_begin",prefix.c_str());
    key=buf;
}

void dblistBase::getLastKey (std::string & key,const std::string & name){
    char buf[256];
    snprintf(buf,256,"%s_last_%s",prefix.c_str(),name.c_str());
    key=buf;
}

void dblistBase::getNextKey (std::string & key,const std::string & name){
    char buf[256];
    snprintf(buf,256,"%s_next_%s",prefix.c_str(),name.c_str());
    key=buf;
}

void dblistBase::setNext(const std::string & key,const std::string & nkey){
    std::string nk;
    getNextKey(nk , key);
    writeDb(nk,nkey);
}
void dblistBase::setLast(const std::string & key,const std::string & nkey){
    std::string nk;
    getLastKey(nk , key);
    writeDb(nk,nkey);
}
void dblistBase::setBegin(const std::string & nkey){
    std::string nk;
    getBeginKey(nk);
    writeDb(nk,nkey);
}
void dblist::readDb (const std::string & key,std::string & value){
    db->Get(leveldb::ReadOptions(),key,&value);
}
void dblist::delDb  (const std::string & key){
    db->Delete(leveldb::WriteOptions(),key);
}
void dblist::writeDb(const std::string & key,const std::string & value){
    db->Put(leveldb::WriteOptions(),key,value);
}

void dblist::clear(){
    leveldb::WriteBatch batch;
    seekBegin();
    std::string lk,nk;
    getBeginKey(nk);
    batch.Delete(nk);
    
    while(1){
        if(!key_now.empty()){
            
            getLastKey(lk , key_now);
            getNextKey(nk , key_now);
            batch.Delete(lk);
            batch.Delete(nk);
            
            if(!next())
                break;
        }else
            break;
    }
    db->Write(leveldb::WriteOptions(), &batch);
}

}