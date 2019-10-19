#include "removeTable.h"
namespace smoothly{
namespace server{
////////////////
void removeTable::getRemovedItem(int x,int y,std::list<std::pair<int,int> > & o){
    o.clear();
    
    char prefix[128];
    snprintf(prefix,sizeof(prefix),"mRMT%d,%d:",x,y);
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        
        o.push_back(getIdFromStr(v.c_str()));
    }
    delete it;
}
void removeTable::addRemovedItem(int x,int y,int id,int index){
    char key[256];
    char val[256];
    snprintf(key ,sizeof(key) ,"mRMT%d,%d:%d,%d",x,y,id,index);
    snprintf(val ,sizeof(val) ,"%d %d",id,index);
    db->Put(leveldb::WriteOptions(), key, val);
}
void removeTable::clearRemovedChunk(int x,int y){
    char prefix[256];
    snprintf(prefix,sizeof(prefix),"mRMT%d,%d:",x,y);
    
    leveldb::WriteBatch batch;
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        
        batch.Delete(k);
    }
    delete it;
    db->Write(leveldb::WriteOptions(), &batch);
}
std::pair<int,int> removeTable::getIdFromStr(const char * s){
    int id,index;
    sscanf(s,"%d %d",&id,&index);
    return std::pair<int,int>(id,index);
}
////////////////
}//////server
}//////smoothly