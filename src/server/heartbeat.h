#ifndef SMOOTHLY_SERVER_HEARTBEAT
#define SMOOTHLY_SERVER_HEARTBEAT

#include <time.h>
#include <map>
#include <functional>

namespace smoothly{
namespace server{

//心跳包处理，自动踢出超时的客户端
template<typename T>
class heartbeat{
    public:
        heartbeat(){
            begin = NULL;
            end   = NULL;
        }
        ~heartbeat(){
            auto n = begin;
            while(n){
                auto tmp = n->next;
                delete n;
                n = tmp;
            }
        }
    private:
        struct node{
            node(){
                last = NULL;
                next = NULL;
            }
            int ltime;
            T name;
            node * last;
            node * next;
        };
        node * begin , * end;
        std::map<T,node*> nodes;

        inline void addNode(node * n){
            if(begin){
                auto tmp = begin;
                begin = n;
                tmp->last = n;
                n->next = tmp;
                n->last = NULL;
            }else{
                begin   = n;
                end     = n;
                n->last = NULL;
                n->next = NULL;
            }
        }
        inline void removeNode(node * n){
            if(n->next){
                if(n->last){
                    n->next->last = n->last;
                    n->last->next = n->next;
                }else{
                    begin         = n->next;
                    n->next->last = NULL;
                }
            }else{
                if(n->last){
                    end           = n->last;
                    n->last->next = NULL;
                }else{
                    begin = NULL;
                    end   = NULL;
                }
            }
        }
    public:
        inline void removeExpire(std::function<void(const T&)> callback){

            #define rmNode \
                callback(n->name); \
                nodes.erase(n->name); \
                delete n;

            auto n = end;
            time_t t = time(0);
            while(n){
                if(t - n->ltime > 10){
                    auto tmp = n->last;
                    if(n->next){
                        if(n->last){
                            n->next->last = n->last;
                            n->last->next = n->next;
                        }else{
                            begin         = n->next;
                            n->next->last = NULL;
                            rmNode;
                            return;
                        }
                    }else{
                        if(n->last){
                            end           = n->last;
                            n->last->next = NULL;
                        }else{
                            begin = NULL;
                            end   = NULL;
                            rmNode;
                            return;
                        }
                    }
                    rmNode;
                    n = tmp;
                }else{
                    return;
                }
            }
            #undef rmNode
        }
        inline void markNode(T o){
            node * n;
            auto it = nodes.find(o);
            if(it==nodes.end()){
                n = new node;
                n->name = o;
                addNode(n);
                nodes[o] = n;
            }else{
                n = it->second;
                if(n->last!=NULL){//不是第一个
                    removeNode(n);
                    addNode(n);
                }
            }
            n->ltime = time(0);
        }
        inline void eraseNode(T o){
            node * n;
            auto it = nodes.find(o);
            if(it!=nodes.end()){
                n = it->second;
                removeNode(n);
                delete n;
                nodes.erase(it);
            }
        }
};

}//server
}//smoothly

#endif // SMOOTHLY_SERVER_HEARTBEAT
