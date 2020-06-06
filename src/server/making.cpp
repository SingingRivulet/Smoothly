#include "making.h"
#include <QFile>

namespace smoothly{
namespace server{

making::making(){

}

bool making::make(const RakNet::SystemAddress & addr, const std::string & user, const std::string & bag, int id, const vec3 & outPosition){
    try{
        auto confit = making_config.find(id);
        if(confit!=making_config.end()){

            making_config_t & conf = confit->second;

            bag_inner & b =  cache_bag_inner[bag];

            for(auto need:conf.needResource){
                auto res = b.resources.find(need.first);
                if(res==b.resources.end()){
                    return false;//资源不存在
                }
                if(res->second < need.second){
                    return false;//资源不够
                }
            }
            if(!checkTech(addr,user,conf.needTech))
                return false;//科技未解锁

            //开始制造
            for(auto need:conf.needResource){
                addResource(addr , bag , need.first , -abs(need.second));
            }
            package_item pkg;
            if(conf.isTool){
                auto tuuid = createTool(conf.outId);
                pkg.tool.insert(tuuid);
            }else{
                pkg.resource.push_back(package_item::resource_t(conf.outId,1));
            }
            pkg.skin = 0;
            pkg.position = outPosition;
            putPackage(pkg);
            return true;
        }
    }catch(...){}
    return false;
}


}
}
