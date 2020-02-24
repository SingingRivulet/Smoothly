#ifndef SMOOTHLY_SERVER_CACHE
#define SMOOTHLY_SERVER_CACHE

#include <time.h>
#include <map>
#include <functional>
#include <unordered_map>

namespace smoothly{
namespace server{

template<typename V,typename T=std::string>
class cache{
    public:
        cache(){
            begin = NULL;
            end   = NULL;
            timeout = 60;
        }
        ~cache(){
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
            V value;
        };
        node * begin , * end;
        std::unordered_map<T,node*> nodes;

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
        V & operator[](const T & i){
            auto p = markNode(i);
            return p->value;
        }
        void erase(const T & o){
            node * n;
            auto it = nodes.find(o);
            if(it!=nodes.end()){
                n = it->second;
                removeNode(n);
                delete n;
                nodes.erase(it);
            }
        }
        void clear(){
            auto n = begin;
            while(n){
                auto tmp = n->next;
                onExpire(tmp->name,tmp->value);
                delete n;
                n = tmp;
            }
        }
        int timeout;
        inline void removeExpire(){
            if(timeout<=0)
                return;
            #define rmNode \
                onExpire(n->name,n->value); \
                nodes.erase(n->name); \
                delete n;

            auto n = end;
            time_t t = time(0);
            while(n){
                if(t - n->ltime > timeout){
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
        virtual void onExpire(const T &,V &)=0;
        virtual void onLoad(const T &,V &)=0;
    private:
        inline node * markNode(T o){
            node * n;
            auto it = nodes.find(o);
            if(it==nodes.end()){
                V tmp;
                onLoad(o,tmp);//可通过throw退出
                n = new node;
                n->name = o;
                addNode(n);
                n->value=tmp;
                nodes[o] = n;
            }else{
                n = it->second;
                if(n->last!=NULL){//不是第一个
                    removeNode(n);
                    addNode(n);
                }
            }
            n->ltime = time(0);
            return n;
        }
};

}
}
#endif // SMOOTHLY_SERVER_CACHE
