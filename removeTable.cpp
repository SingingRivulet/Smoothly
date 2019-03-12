#include "removeTable.h"
namespace smoothly{
void removeTable::init(const char * path){
    leveldb::Options opt;
    opt.create_if_missing=true;
    leveldb::DB::Open(opt,path,&this->db);
}
void removeTable::destroy(){
    delete this->db;
}
bool removeTable::mapidExist(int x,int y,long itemid,int mapid){
    std::string buf;
    char key[128];
    snprintf(key,sizeof(key),"mapid_%d_%d_%ld_%d",x,y,itemid,mapid);
    return (db->Get(leveldb::ReadOptions(),key,&buf).ok() && !buf.empty());
}
void removeTable::setMapidElement(int x,int y,long itemid,int mapid){
    char key[128];
    snprintf(key,sizeof(key),"mapid_%d_%d_%ld_%d",x,y,itemid,mapid);
    db->Put(leveldb::WriteOptions(),key,"1");
}
void removeTable::removeMapidElement(int x,int y,long itemid,int mapid){
    char key[128];
    snprintf(key,sizeof(key),"mapid_%d_%d_%ld_%d",x,y,itemid,mapid);
    db->Delete(leveldb::WriteOptions(),key);
}
void removeTable::removeTerrain(int x,int y,long itemid,int mapid){
    if(itemidNum(mapid)<mapid)
        return;
    if(mapidExist(x,y,itemid,mapid))
        return;
    setMapidElement(x,y,itemid,mapid);
    
    char key[128];
    snprintf(key,128,"table_%d_%d",x,y);
    char tmp[128];
    mapid2str(itemid,mapid,tmp,128);
    
    std::string buf;
    if(db->Get(leveldb::ReadOptions(),key,&buf).ok() && !buf.empty())
        buf+=std::string(" ")+tmp;
    else
        buf=tmp;
    db->Put(leveldb::WriteOptions(),key,buf);
    onRemoveTerrain(x,y,itemid,mapid);
}
void removeTable::getRemoveTable(const RakNet::SystemAddress & to,int x,int y){
    char key[128];
    snprintf(key,128,"table_%d_%d",x,y);
    std::string buf,buf2;
    std::list<std::pair<long,int> > l;
    if(db->Get(leveldb::ReadOptions(),key,&buf).ok() && !buf.empty()){
        std::istringstream iss(buf);
        int num=0;
        while(!iss.eof()){
            buf2.clear();
            iss>>buf2;
            if(buf2.empty())
                break;
            long iid;
            int mid;
            str2mapid(buf2.c_str(),&iid,&mid);
            l.push_back(std::pair<long,int>(iid,mid));
            ++num;
            if(num>=7){
                sendRemoveTable(to,l,x,y);
                l.clear();
                num=0;
            }
        }
        if(!l.empty()){
            sendRemoveTable(to,l,x,y);
        }
    }
    removeTableApply(to,x,y);
}
void removeTable::resetTerrain(int x,int y){
    char key[128];
    snprintf(key,128,"table_%d_%d",x,y);
    std::string buf,buf2;
    if(db->Get(leveldb::ReadOptions(),key,&buf).ok() && !buf.empty()){
        std::istringstream iss(buf);
        while(!iss.eof()){
            buf2.clear();
            iss>>buf2;
            if(buf2.empty())
                break;
            long iid;
            int mid;
            str2mapid(buf2.c_str(),&iid,&mid);
            removeMapidElement(x,y,iid,mid);
        }
    }
}
}