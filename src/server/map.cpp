#include "map.h"
namespace smoothly{
namespace server{
////////////////
void map::updateNode(const std::string & uuid,int x,int y,std::set<ipair> & nwTable){
    try{
        auto op = getNodePosi(uuid);
        if(op.x==x && op.y==y)
            return;
        std::set<ipair> rmTable,tmpTable;
        buildVisualFieldArray(op.x , op.y , rmTable);
        buildVisualFieldArray(   x ,    y , nwTable);
        tmpTable=nwTable;
        for(auto it:tmpTable){
            if(rmTable.find(it)!=rmTable.end()){
                rmTable.erase(it);
                nwTable.erase(it);
            }
        }
        leveldb::WriteBatch batch;
        for(auto it:rmTable){
            batch.Delete(getNodePrefix(it.x,it.y)+uuid);
        }
        for(auto it:nwTable){
            batch.Put(getNodePrefix(it.x,it.y)+uuid , uuid);
        }
        char buf[128],buf2[128];
        snprintf(buf ,sizeof(buf) ,"map_p_%s",uuid.c_str());
        snprintf(buf2,sizeof(buf2),"%d %d",x,y);
        batch.Put(buf,buf2);
    
        db->Write(leveldb::WriteOptions(), &batch);
    }catch(...){
        logError();
    }
}
void map::addNode(const std::string & uuid,const std::string & owner,int x,int y){
    std::set<ipair> nwTable;
    buildVisualFieldArray(x , y , nwTable);
    leveldb::WriteBatch batch;
    
    char buf[128],buf2[128];
    //写入拥有者
    snprintf(buf,sizeof(buf),"map_o_%s",uuid.c_str());
    batch.Put(buf,owner);
    
    snprintf(buf,sizeof(buf),"map_u_%s_%s",owner.c_str(),uuid.c_str());
    batch.Put(buf,uuid);
    
    snprintf(buf ,sizeof(buf) ,"map_p_%s",uuid.c_str());
    snprintf(buf2,sizeof(buf2),"%d %d",x,y);
    batch.Put(buf,buf2);
    
    for(auto it:nwTable){
        batch.Put(getNodePrefix(it.x,it.y)+uuid , uuid);
    }
    db->Write(leveldb::WriteOptions(), &batch);
}
void map::removeNode(const std::string & uuid){
    try{
        auto op = getNodePosi(uuid);
        auto owner = getNodeOwner(uuid);
        
        std::set<ipair> nwTable;
        buildVisualFieldArray(op.x , op.y , nwTable);
        leveldb::WriteBatch batch;
    
        char buf[128];//拥有者
        snprintf(buf,sizeof(buf),"map_o_%s",uuid.c_str());
        batch.Delete(buf);
        
        snprintf(buf ,sizeof(buf) ,"map_p_%s",uuid.c_str());
        batch.Delete(buf);
        
        if(!owner.empty()){
            snprintf(buf,sizeof(buf),"map_u_%s_%s",owner.c_str(),uuid.c_str());
            batch.Delete(buf);
        }
    
        for(auto it:nwTable){
            batch.Delete(getNodePrefix(it.x,it.y)+uuid);
        }
        db->Write(leveldb::WriteOptions(), &batch);
    }catch(...){
        logError();
    }
}
ipair map::getNodePosi(const std::string & uuid){
    std::string value;
    db->Get(leveldb::ReadOptions(), std::string("map_p_")+uuid , &value);
    if(value.empty())
        throw std::out_of_range("getNodePosi");
    else{
        int x,y;
        sscanf(value.c_str(),"%d %d",&x,&y);
        return ipair(x,y);
    }
}
void map::getUsers(int x,int y,std::set<std::string> & o){
    
    o.clear();
    
    std::string prefix = getNodePrefix(x,y);
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        {
            char buf[128];
            snprintf(buf,sizeof(buf),"map_o_%s",v.c_str());
            std::string value;
            db->Get(leveldb::ReadOptions(), buf , &value);
            if(!value.empty())
                o.insert(value);
        }
    }
    delete it;
    
}
void map::getNode(int x,int y,std::set<std::string> & o){
    
    o.clear();
    
    std::string prefix = getNodePrefix(x,y);
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        
        o.insert(v);
    }
    delete it;
    
}
std::string map::getNodePrefix(int x,int y){
    char buf[128];
    snprintf(buf,sizeof(buf),"map_#%d,%d:",x,y);
    return std::string(buf);
}
std::string map::getNodeOwner(const std::string & uuid){
    char buf[128];//拥有者
    snprintf(buf,sizeof(buf),"map_o_%s",uuid.c_str());
    std::string value;
    db->Get(leveldb::ReadOptions(), buf , &value);
    return value;
}
void map::getUserNodes(const std::string & owner,std::set<ipair> & o){
    o.clear();
    char buf[128];
    snprintf(buf,sizeof(buf),"map_u_%s",owner.c_str());
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(buf); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(buf,k))
            break;//前缀不匹配，说明搜索完了
        
        {
            try{
                auto posi = getNodePosi(v);
                int i = posi.x - visualField;
                int j = posi.y - visualField;
                int xm= posi.x + visualField;
                int ym= posi.y + visualField;
                for(;i<=xm;++i){
                    for(;j<=ym;++j){
                        o.insert(ipair(i,j));
                    }
                }
            }catch(...){
                logError();
            }
        }
        
    }
    delete it;
}
void map::buildVisualFieldArray(int x,int y,std::set<ipair> & o){
    o.clear();
    int i = x - visualField;
    int j = y - visualField;
    int xm= x + visualField;
    int ym= y + visualField;
    for(;i<=xm;++i){
        for(;j<=ym;++j){
            o.insert(ipair(i,j));
        }
    }
}
////////////////
}//////server
}//////smoothly
