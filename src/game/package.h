#ifndef SMOOTHLY_PACKAGE_H
#define SMOOTHLY_PACKAGE_H
#include "terrainDispather.h"
namespace smoothly{

class package:public terrainDispather{
    public:
        package();
        ~package();

        struct package_config_t{
                int id;
                irr::scene::IMesh * mesh;
        };
        std::map<int,package_config_t*> package_configs;

        struct package_t{
                package_config_t * conf;
                int cx,cy;
                irr::scene::ISceneNode * node;
                std::string uuid;
        };
        std::map<std::string,package_t> packages;

        irr::scene::ISceneNode * packageRoot;

        void addPackage(int id, int x, int y, const vec3 & posi, const std::string & uuid);
        void removePackage(const std::string & uuid);
        void pickupPackage();
        virtual void pickupPackageToBag(int x,int y,const std::string & uuid)=0;

        void msg_package_add(int32_t x,int32_t y,const char * uuid,const char * text)override;
        void msg_package_remove(int32_t x,int32_t y,const char * uuid)override;

        void loop()override;

        irr::scene::ISceneNode * selectedPackageSceneNode;
};

}
#endif // SMOOTHLY_PACKAGE_H
