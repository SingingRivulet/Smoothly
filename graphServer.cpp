#include "graphServer.h"
namespace smoothly{

void graphServer::remove(const std::string & uuid){
    removeAllLink(uuid);
    remove(uuid,maxdeep);
}
void graphServer::remove(const std::string & uuid,int deep){
    if(deep<=0)
        return;
    std::string buf1;
    std::string knode=std::string("node_")+uuid;
    int x,y;
    if(db->Get(leveldb::ReadOptions(),knode,&buf1).ok() && !buf1.empty()){
        //removeAllLink(uuid);
        db->Delete(leveldb::WriteOptions(),std::string("link_")+uuid);
        db->Delete(leveldb::WriteOptions(),std::string("hp_")+uuid);
        db->Delete(leveldb::WriteOptions(),std::string("position_")+uuid);
        db->Delete(leveldb::WriteOptions(),std::string("rotation_")+uuid);
        getChunk(uuid,x,y);
        //removeFromChunk(uuid,x,y);
        removeList[ipair(x,y)].insert(uuid);
        db->Delete(leveldb::WriteOptions(),std::string("chunk_")+uuid);
        removeAllLinkTo(uuid,deep-1);
        db->Delete(leveldb::WriteOptions(),knode);
    }
}
void graphServer::removeAllLinkTo(const std::string & uuid,int deep){
    if(deep<=0)
        return;
    std::string key=std::string("linkTo_")+uuid;
    std::string buf1,buf2;
    if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
        std::istringstream iss(buf1);
        while(!iss.eof()){
            buf2.clear();
            iss>>buf2;
            if(buf2.empty())
                break;
            if(removeLink(buf2,uuid)==0)
                remove(buf2,deep);
        }
        db->Delete(leveldb::WriteOptions(),key);
    }
}
void graphServer::removeAllLink(const std::string & uuid){
    std::string key=std::string("link_")+uuid;
    std::string buf1,buf2;
    if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
        std::istringstream iss(buf1);
        while(!iss.eof()){
            buf2.clear();
            iss>>buf2;
            if(buf2.empty())
                break;
            if(buf2==uuid)
                continue;
            removeLinkTo(buf2,uuid);
        }
    }
    db->Delete(leveldb::WriteOptions(),key);
}
int graphServer::removeLinkTo(const std::string & uuid,const std::string & rm){
    return removeSet("linkTo_",uuid,rm);
}
int graphServer::removeLink(const std::string & uuid,const std::string & rm){
    return removeSet("link_",uuid,rm);
}
int graphServer::removeSet(const std::string & pre,const std::string & uuid,const std::string & rm){
    std::string key=pre+uuid;
    std::string buf1,buf2,buf3;
    if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
        std::istringstream iss(buf1);
        buf3.clear();
        int count=0;
        
        while(!iss.eof()){
            buf2.clear();
            iss>>buf2;
            if(buf2.empty())
                break;
            if(buf2==rm || buf2==uuid)
                continue;
            buf3+=buf2+" ";
            ++count;
        }
        
        buf1.clear();
        if(buf3.empty()){
            db->Delete(leveldb::WriteOptions(),key);
            return 0;
        }
        
        db->Put(leveldb::WriteOptions(),key,buf3);
        return count;
    }
    return 0;
}

void graphServer::addSet(const std::string & pre,const std::string & uuid,const std::string & a){
    std::string key=pre+uuid;
    std::string buf1,buf2;
    buf1.clear();
    buf2.clear();
    if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
        buf2=buf1+a+" ";
    }else
        buf2=a+" ";
    db->Put(leveldb::WriteOptions(),key,buf2);
}

void graphServer::addLink(const std::string & uuid,const std::string & a){
    addSet("link_",uuid,a);
}
void graphServer::addLinkTo(const std::string & uuid,const std::string & a){
    addSet("linkTo_",uuid,a);
}
void graphServer::attack(const std::string & uuid,int h){
    if(hurt(uuid,h)<=0)
        remove(uuid);
}
int graphServer::hurt(const std::string & uuid,int hurt){
    std::string key=std::string("hp_")+uuid;
    std::string buf1;
    if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
        int hp=atoi(buf1.c_str());
        char buf2[32];
        hp-=hurt;
        snprintf(buf2,32,"%d",hp);
        db->Put(leveldb::WriteOptions(),key,buf2);
        return hp;
    }
    return 0;
}

void graphServer::createNode(
    long type,
    const irr::core::vector3df & position,
    const irr::core::vector3df & rotation,
    const std::set<std::string> & link
){
    std::string uuid;
    int hp=getHPByType(type);
    getUUID(uuid);
    char buf[512];
    
    int x,y;
    getChunkPosition(position.X,position.Z,x,y);
    
    snprintf(buf,512,"%d",hp);
    db->Put(leveldb::WriteOptions(),std::string("hp_")+uuid , buf);
    
    snprintf(buf,512,"%ld",type);
    db->Put(leveldb::WriteOptions(),std::string("node_")+uuid , buf);
    
    snprintf(buf,512,"%f %f %f",position.X,position.Y,position.Z);
    db->Put(leveldb::WriteOptions(),std::string("position_")+uuid , buf);
    
    snprintf(buf,512,"%f %f %f",rotation.X,rotation.Y,rotation.Z);
    db->Put(leveldb::WriteOptions(),std::string("rotation_")+uuid , buf);
    
    snprintf(buf,512,"%d %d",x,y);
    db->Put(leveldb::WriteOptions(),std::string("chunk_")+uuid , buf);
    
    addIntoChunk(uuid,x,y);
    
    std::string bufs="";
    for(auto it:link){
        addLinkTo(it,uuid);
        bufs+=it+" ";
    }
    db->Put(leveldb::WriteOptions(),std::string("link_")+uuid,bufs);
    
    onCreateNode(uuid,position,rotation,link,x,y);
}

void graphServer::getChunk(const std::string & uuid,int & x,int & y){
    auto key=std::string("chunk_")+uuid;
    std::string buf;
    if(db->Get(leveldb::ReadOptions(),key,&buf).ok() && !buf.empty()){
        std::istringstream iss(buf);
        iss>>x;
        iss>>y;
    }
}
void graphServer::removeFromChunk(const std::string & uuid,int x,int y){
    char key[64];
    snprintf(key,64,"chunkdata_%d_%d_",x,y);
    removeSet("map_",key,uuid);
}
void graphServer::addIntoChunk(const std::string & uuid,int x,int y){
    char key[64];
    snprintf(key,64,"chunkdata_%d_%d_",x,y);
    addSet("map_",key,uuid);
}
void graphServer::removeApplay(){
    for(auto it:removeList){
        char key[256];
        snprintf(key,256,"map_chunkdata_%d_%d_",it.first.x,it.first.y);
        std::string buf1,buf2,buf3;
        if(db->Get(leveldb::ReadOptions(),key,&buf1).ok() && !buf1.empty()){
            buf3.clear();
            std::istringstream iss(buf1);
            while(!iss.eof()){
                buf2.clear();
                iss>>buf2;
                if(buf2.empty())
                    break;
                if(it.second.find(buf2)!=it.second.end())
                    continue;
                buf3+=buf2+" ";
            }
            if(buf3.empty())
                db->Delete(leveldb::WriteOptions(),key);
            else
                db->Put(leveldb::WriteOptions(),key,buf3);
        }
    }
    removeList.clear();
}

}