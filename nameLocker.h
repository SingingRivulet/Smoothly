#ifndef SMOOTHLY_NAMELOCKER
#define SMOOTHLY_NAMELOCKER
#include <mutex>
#include <unordered_set>
namespace smoothly{
    template<typename T,typename S=std::unordered_set<T> >
    class nameLocker{//命名自旋锁
        public:
            void lock(const T & n){
                while(!lock_noblock(n)){}
            }
            bool lock_noblock(const T & n){
                bool res;
                locker.lock();
                res=lst.insert(n).second;
                locker.unlock();
                return res;
            }
            void unlock(const T & n){
                locker.lock();
                lst.erase(n);
                locker.unlock();
            }
        private:
            S lst;
            std::mutex locker;
    };
    typedef nameLocker<std::string> sNameLocker;
}
#endif