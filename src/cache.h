#ifndef SMOOTHLY_CACHE
#define SMOOTHLY_CACHE
#include "utils.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <stdio.h>
#include <sys/time.h>
namespace smoothly{
    class cache{
        public:
            cache(){
                
            }
            ~cache(){
                clear();
            }
            void clear(){
                tablemtx.lock();
                for(auto it:nodes){
                    it.second->drop();
                }
                nodes.clear();
                tablemtx.unlock();
            }
            class node{
                friend class cache;
                protected:
                    long expire;
                    cache * parent;
                public:
                    inline node(){
                        parent=NULL;
                        expire=0;
                        rcount=1;
                    }
                    inline void drop(){
                        if(parent){
                            parent->nodemtx.lock();
                            --rcount;
                            if(rcount<=0){
                                parent->nodemtx.unlock();
                                onFree();
                                delete this;
                                return;
                            }else
                                parent->nodemtx.unlock();
                        }
                    }
                    inline void grab(){
                        if(parent){
                            parent->nodemtx.lock();
                            ++rcount;
                            parent->nodemtx.unlock();
                        }
                    }
                    virtual void onFree()=0;
                private:
                    int rcount;
            };
            friend class node;
        private:
            std::unordered_map<std::string,node*> nodes;
            std::map<long,std::set<std::string> > expires;
            std::mutex expiresLocker;
            inline void addExpire(const std::string & name,long exp){
                expiresLocker.lock();
                expires[exp].insert(name);
                expiresLocker.unlock();
            }
            inline void delExpire(const std::string & name,long exp){
                expiresLocker.lock();
                auto it=expires.find(exp);
                if(it!=expires.end()){
                    it->second.erase(name);
                }
                expiresLocker.unlock();
            }
        public:
            inline static long getTime(){
                struct timeval tv;
                gettimeofday(&tv,NULL);
                return tv.tv_sec;
            }
        protected:
            std::mutex nodemtx;
            std::mutex tablemtx;
        public:
            inline void clearExpire(){
                auto ntm=getTime();
                expiresLocker.lock();
                for(auto it=expires.rbegin();it!=expires.rend();++it){
                    if(it->first  < ntm){
                        for(auto s:it->second){
                            tablemtx.lock();
                            auto itp=nodes.find(s);
                            if(itp!=nodes.end()){
                                auto p=itp->second;
                                nodes.erase(itp);
                                p->drop();
                            }
                            tablemtx.unlock();
                        }
                    }else
                        break;
                }
                expiresLocker.unlock();
            }
            inline node * get(const std::string & name){
                tablemtx.lock();
                auto it=nodes.find(name);
                if(it!=nodes.end()){
                    auto p=it->second;
                    p->grab();
                    tablemtx.unlock();
                    return p;
                }else{
                    tablemtx.unlock();
                    return NULL;
                }
            }
            inline void del(const std::string & name){
                tablemtx.lock();
                auto it=nodes.find(name);
                if(it!=nodes.end()){
                    auto p=it->second;
                    delExpire(it->first , p->expire);
                    nodes.erase(it);
                    p->drop();
                }
                tablemtx.unlock();
            }
            inline void put(const std::string & name,node * n,int life=600){
                clearExpire();
                tablemtx.lock();
                auto it=nodes.find(name);
                if(it!=nodes.end()){
                    if(it->second==n){
                        tablemtx.unlock();
                        return;
                    }else
                        delExpire(name,it->second->expire);
                        it->second->drop();
                }
                n->parent=this;
                n->grab();
                n->expire=getTime()+life;
                addExpire(name,n->expire);
                
                nodes[name]=n;
                
                tablemtx.unlock();
            }
    };
}
#endif
