#ifndef SMOOTHLY_SERVER_DB
#define SMOOTHLY_SERVER_DB
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <sys/stat.h>
#include "fontcolor.h"

#define logError() logger->error("{}:{} {}",__FILE__,__LINE__,__FUNCTION__)

namespace smoothly{
namespace server{

class datas{
    public:
        inline datas(){
            mkdir("log",0777);
            printf(L_GREEN "[status]" NONE"open log file\n" );
            logger = spdlog::rotating_logger_mt("server", "log/server.log", 1024 * 1024 * 5, 3);
            leveldb::Options options;
            options.create_if_missing = true;
            leveldb::DB::Open(options, "./data", &db);
            logger->info("open database");
        }
        inline ~datas(){
            delete db;
        }
        leveldb::DB * db;
        std::shared_ptr<spdlog::logger> logger;
};

inline bool isPrefix(const std::string & pfx , const std::string & str){
    auto p=pfx.c_str();
    auto s=str.c_str();
    int len = pfx.size();
    for(int i=0;i<len;i++){
        if(p[i]!=s[i])
            return false;
    }
    return true;
}

#define prefixSearchingBegin(prefix) \
    leveldb::Iterator * it = db->NewIterator(leveldb::ReadOptions()); \
    for(it->Seek(prefix); it->Valid(); it->Next()) { \
        auto key = it->key().ToString(); \
        auto val = it->value().ToString(); \
        if(!isPrefix(prefix,key)) \
            break;

#define prefixSearchingEnd() } delete it;

struct vec3{
    float X,Y,Z;
    inline vec3(){
        X=0;Y=0;Z=0;
    }
    inline vec3(float ix,float iy,float iz){
        X=ix;Y=iy;Z=iz;
    }
    inline vec3(const vec3 & i){
        X=i.X;Y=i.Y;Z=i.Z;
    }
    inline const vec3 & operator=(const vec3 & i){
        X=i.X;Y=i.Y;Z=i.Z;
        return *this;
    }
};

class ipair{
    public:
        int32_t x,y;
        inline bool operator==(const ipair & i)const{
            return (x==i.x) && (y==i.y);
        }
        inline bool operator<(const ipair & i)const{
            if(x<i.x)
                return true;
            else
            if(x==i.x){
                if(y<i.y)
                    return true;
            }
            return false;
        }
        inline ipair & operator=(const ipair & i){
            x=i.x;
            y=i.y;
            return *this;
        }
        inline ipair(const ipair & i){
            x=i.x;
            y=i.y;
        }
        inline ipair(const int & ix , const int & iy){
            x=ix;
            y=iy;
        }
        inline ipair(){
            x=0;
            y=0;
        }
};

}//////server
}//////smoothly
#endif
