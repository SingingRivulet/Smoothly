#ifndef SMOOTHLY_DBLIST
#define SMOOTHLY_DBLIST
#include "utils.h"
#include <string>
#include <stdio.h>
namespace smoothly{
    class dblistBase{//在leveldb上建一个链表
        public:
            void seek(const std::string & key);
            bool seekBegin();
            bool last();
            bool next();
            void del(const std::string & key);
            void pushBegin(const std::string & key);
            void clear();
            
            std::string prefix;
            std::string key_now;
            std::string key_last;
            std::string key_next;
            
            virtual void onDel  (const std::string & key);
            virtual void readDb (const std::string & key,std::string & value)=0;
            virtual void delDb  (const std::string & key)=0;
            virtual void writeDb(const std::string & key,const std::string & value)=0;
            
        private:
            void setNext(const std::string & key,const std::string & nkey);
            void setLast(const std::string & key,const std::string & nkey);
            void setBegin(const std::string & key);
            void delNode_unsafe(const std::string & key);
            
        private:
            void getBeginKey(std::string & key);
            void getLastKey (std::string & key,const std::string & name);
            void getNextKey (std::string & key,const std::string & name);
    };
}
#endif
