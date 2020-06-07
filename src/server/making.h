#ifndef SMOOTHLY_SERVER_MAKING
#define SMOOTHLY_SERVER_MAKING

#include "technology.h"

namespace smoothly{
namespace server{

class making:public technology{
    public:
        making();
        struct making_config_t{
                int outId;//生产出来的id
                bool isTool;
                std::vector<std::pair<int,int> > needResource;//id=>num
                int needTech;
        };
        std::map<int,making_config_t> making_config;
        bool make(const RakNet::SystemAddress & addr,const std::string & user,const std::string & bag,int id,const vec3 & outPosition);
};

}
}

#endif // SMOOTHLY_SERVER_MAKING
