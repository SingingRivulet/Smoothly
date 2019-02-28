#ifndef SMOOTHLY_SERVER_USER
#define SMOOTHLY_SERVER_USER
namespace smoothly{
    class userServer{
        public:
            leveldb::DB * db;
            void init(const char * path);
            void destroy();
            
            void setUserPosition(const irr::core::vector3df & position,const std::string & uuid);
    };
}
#endif