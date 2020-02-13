#ifndef SMOOTHLY_MEMPOOL
#define SMOOTHLY_MEMPOOL
#include <mutex>
#include <atomic>
namespace smoothly{
    template<typename T>
    class mempool_block{
            T * freed;
            std::mutex locker;
            std::atomic<int>    rnum,//引用计数器
                                malloced,used;
        public:
            inline void pickup(){
                rnum++;
            }
            inline void giveup(){
                rnum--;
                if(rnum==0){
                    delete this;
                    return;
                }
            }
            mempool_block(){
                malloced=0;
                used=0;
                freed=NULL;
                rnum=1;
            }
            ~mempool_block(){
                T * it1;
                T * it=freed;
                while(it){
                    it1=it;
                    it=it->next;
                    delete it1;
                }
                #ifdef DEBUG
                int mn=malloced;
                int un=used;
                printf("malloc:%d\tused:%d\n",mn,un);
                #endif
            }
            T * get(){
                used++;
                locker.lock();
                if(freed){
                    T * r=freed;
                    freed=freed->next;
                    locker.unlock();
                    return r;
                }else{
                    locker.unlock();
                    malloced++;
                    return new T;
                }
            }
            void del(T * f){
                locker.lock();
                f->next=freed;
                freed=f;
                locker.unlock();
                used--;
            }
        };
        template<typename T>
        class mempool{
            protected:
                mempool_block<T> * parpool;
                std::atomic<int> malloced,used;
            public:
                mempool(){
                    malloced=0;
                    used=0;
                    static mempool_block<T> gbpool;
                    parpool=&gbpool;
                }
                ~mempool(){
                    #ifdef DEBUG
                    int mn=malloced;
                    int un=used;
                    printf("malloc:%d\tused:%d\n",mn,un);
                    #endif
                }
                T * get(){
                    malloced++;
                    used++;
                    return parpool->get();
                }
                void del(T * f){
                    used--;
                    parpool->del(f);
                }
        };
}
#endif
