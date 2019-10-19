#ifndef SMOOTHLY_SERVER_MAP
#define SMOOTHLY_SERVER_MAP
#include "db.h"
#include <exception>
#include <string>
#include <set>
#include <vector>
#include <list>
#include <iostream>
namespace smoothly{
namespace server{

class map:public datas{
    public:
        void updateNode(const std::string & uuid,int x,int y,std::set<ipair> & nwTable);
        void addNode(const std::string & uuid,const std::string & owner,int x,int y);
        void removeNode(const std::string & uuid);
        void getNode(int x,int y,std::set<std::string> & o);
        void getUsers(int x,int y,std::set<std::string> & o);
        
        std::string getNodeOwner(const std::string & uuid);
        void getUserNodes(const std::string & owner , std::set<ipair> & o);
        
        int visualField;
    private:
        static std::string getNodePrefix(int x,int y);
        void buildVisualFieldArray(int x,int y,std::set<ipair> & o);
        ipair getNodePosi(const std::string & uuid);
};

}//////server
}//////smoothly
#endif