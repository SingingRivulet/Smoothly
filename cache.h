#ifndef SMOOTHLY_CACHE
#define SMOOTHLY_CACHE
#include "utils.h"
#include <mutex>
#include <atomic>
#include <string>
#include <unordered_map>
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
                public:
                    long expire;
                    cache * parent;
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
                tablemtx.lock();
                auto ntm=getTime();
                for(auto it=nodes.begin();it!=nodes.end();){
                    auto tmp=it;
                    it++;
                    if(tmp->second->expire < ntm){
                        auto p=tmp->second;
                        nodes.erase(tmp);
                        p->drop();
                    }
                }
                tablemtx.unlock();
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
                    nodes.erase(it);
                    p->drop();
                }
                tablemtx.unlock();
            }
            inline void put(const std::string & name,node * n,int life=600){
                tablemtx.lock();
                auto it=nodes.find(name);
                if(it!=nodes.end()){
                    if(it->second==n){
                        tablemtx.unlock();
                        return;
                    }else
                        it->second->drop();
                }
                n->parent=this;
                n->grab();
                n->expire=getTime()+life;
                
                nodes[name]=n;
                
                tablemtx.unlock();
            }
    };
}
#endif
