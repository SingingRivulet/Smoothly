#include "map.h"
namespace smoothly{
namespace server{
////////////////
void map::updateNode(const std::string & uuid, int x, int y, std::function<void (int,int)> ncallback){
    try{
        ipair & op = cache_nodePosi[uuid];
        if(op.x==x && op.y==y)
            return;
        leveldb::WriteBatch batch;
        batch.Delete(getNodePrefix(op.x , op.y)+uuid);
        batch.Put(   getNodePrefix(x    , y   )+uuid , uuid);

        ipair aMin(op.x-visualField , op.y-visualField),
              aMax(op.x+visualField , op.y+visualField),
              //原矩形的四个顶点
              bMin(x   -visualField , y   -visualField),
              bMax(x   +visualField , y   +visualField);
              //新矩形的四个顶点

        #define pointInA(ix,iy) (ix>=aMin.x && ix<=aMax.x && iy>=aMin.y && iy<=aMax.y)
        #define pointInB(ix,iy) (ix>=bMin.x && ix<=bMax.x && iy>=bMin.y && iy<=bMax.y)

        /*
        for(int i=aMin.x;i<=aMax.x;++i){
            for(int j=aMin.y;j<=aMax.y;++j){
                if(!pointInB(i,j)){

                }
            }
        }
        */
        for(int i=bMin.x;i<=bMax.x;++i){
            for(int j=bMin.y;j<=bMax.y;++j){
                if(!pointInA(i,j)){

                    ncallback(i,j);
                }
            }
        }
        #undef pointInA
        #undef pointInB

        op = ipair(x,y);

        db->Write(leveldb::WriteOptions(), &batch);
    }catch(...){
        logError();
    }
}
void map::addNode(const std::string & uuid,const std::string & owner,int x,int y){

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

    batch.Put(getNodePrefix(x,y)+uuid , uuid);

    db->Write(leveldb::WriteOptions(), &batch);
}
void map::removeNode(const std::string & uuid){
    try{
        auto owner = getNodeOwner(uuid);
        auto op = cache_nodePosi[uuid];

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

        batch.Delete(getNodePrefix(op.x , op.y)+uuid);

        db->Write(leveldb::WriteOptions(), &batch);
        cache_nodePosi.erase(uuid);
    }catch(...){
        logError();
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
void map::getNode(int x,int y,std::function<void (const std::string &)> callback){

    std::string prefix = getNodePrefix(x,y);
    
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions());
    
    for(it->Seek(prefix); it->Valid(); it->Next()) {
        
        auto k = it->key().ToString();
        auto v = it->value().ToString();
        
        if(!isPrefix(prefix,k))
            break;//前缀不匹配，说明搜索完了
        
        callback(v);
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
void map::getUserNodes(const std::string & owner,std::set<ipair> & o,std::function<void (const std::string & uuid,int x,int y)> bcallback){
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
                auto posi = cache_nodePosi[v];
                bcallback(v,posi.x,posi.y);
                int xm= posi.x + visualField;
                int ym= posi.y + visualField;
                for(int i = posi.x - visualField;i<=xm;++i){
                    for(int j = posi.y - visualField;j<=ym;++j){
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

void map::buildVisualFieldArray(int x, int y, std::function<void(int, int)> ncallback){
    int xm= x + visualField;
    int ym= y + visualField;
    for(int i = x - visualField;i<=xm;++i){
        for(int j = y - visualField;j<=ym;++j){
            ncallback(i,j);
        }
    }
}

void map::loop(){
    cache_nodePosi.removeExpire();
}

void map::release()
{
    cache_nodePosi.clear();
}

void map::cache_nodePosi_t::onExpire(const std::string & uuid , ipair & v){
    char buf[64];
    snprintf(buf,sizeof(buf),"%d %d" , v.x , v.y);
    parent->db->Put(leveldb::WriteOptions(), std::string("map_p_")+uuid , std::string(buf));
}

void map::cache_nodePosi_t::onLoad(const std::string & uuid, ipair & v){
    std::string value;
    parent->db->Get(leveldb::ReadOptions(), std::string("map_p_")+uuid , &value);
    if(value.empty())
        v=ipair(0,0);
    else{
        int x,y;
        sscanf(value.c_str(),"%d %d",&x,&y);
        v=ipair(x,y);
    }
}

////////////////
}//////server
}//////smoothly
