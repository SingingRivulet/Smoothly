#ifndef SMOOTHLY_UUID
#define SMOOTHLY_UUID
#include <string>
#include <uuid/uuid.h>
namespace smoothly{
    inline void getUUID(std::string & o){
        uuid_t u;
        char buf[64]; 
        uuid_generate(u);
        uuid_unparse(u,buf);
        o=buf;
    }
}
#endif
