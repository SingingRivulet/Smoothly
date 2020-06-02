#ifndef SMOOTHLY_SERVER_PACKAGE
#define SMOOTHLY_SERVER_PACKAGE
#include "bag.h"
#include <functional>
#include <unordered_set>
namespace smoothly{
namespace server{

class package:public bag{
    //背包中的物品被丢了出来，变成了可以拾取的包裹
    public:
        package();
        struct package_item{
                int skin;//显示时使用的皮肤
                std::string uuid;
                vec3 position;
                struct resource_t{
                        int id,num;
                        resource_t(){
                            id = 0;
                            num = 0;
                        }
                        resource_t(const resource_t & i){
                            id = i.id;
                            num = i.num;
                        }
                        resource_t(int i,int n){
                            id = i;
                            num = n;
                        }
                };
                std::vector<resource_t> resource;
                std::unordered_set<std::string> tool;
                void toString(std::string & s);
                void loadString(const std::string & s);
                inline void clear(){
                    skin = 0;
                    uuid.clear();
                    position = vec3(0,0,0);
                    resource.clear();
                    tool.clear();
                }
                inline package_item():uuid(),position(),resource(),tool(){
                    skin = 0;
                }
                inline package_item(const package_item & i):uuid(i.uuid),position(i.position),resource(i.resource),tool(i.tool){
                    skin = i.skin;
                }
                inline const package_item & operator=(const package_item & i){
                    skin = i.skin;
                    uuid = i.uuid;
                    position = i.position;
                    resource = i.resource;
                    tool = i.tool;
                    return * this;
                }
        };
        std::string getPackagePrefix(int x,int y);
        std::string getPackageKey(int x,int y,const std::string & uuid);
        void putPackage(package_item & p);//放一个包裹到地图上
        void putPackage(const RakNet::SystemAddress & addr,package_item & p,const std::string & bag);//从包里扔出
        void putPackage_resource(const RakNet::SystemAddress & addr,const std::string & bag,int skin,const vec3 & position,int id,int num);
        void putPackage_tool(const RakNet::SystemAddress & addr, const std::string & bag, int skin, const vec3 & position, const std::string & tool);
        bool pickupPackage(int cx, int cy, const std::string & uuid, package_item & p);//捡起包裹，然后从地图上删除
        void pickupPackage(const RakNet::SystemAddress & addr,int cx, int cy, const std::string & uuid,const std::string & bag);//捡起放包里
        bool readPackage(int cx, int cy, const std::string & uuid, package_item & p);//查看包裹
        void fetchPackageByChunk(int cx, int cy, std::function<void(package_item &)> callback);
        inline void sendAddr_chunkPackage(const RakNet::SystemAddress & addr,int cx, int cy){
            fetchPackageByChunk(cx,cy,[&](package_item & p){
                std::string t;
                p.toString(t);
                sendAddr_packageAdd(addr,cx,cy,p.uuid,t);
            });
        }

        std::map<int,int> collection_skin;
        std::map<int,std::map<int,std::pair<int,int> > > collection_config;//被采集的id => { 资源id => (最小,浮动) }
        void destroyTerrainItem(float x,float y,int id)override;//采集资源
        void addCollectionConfig(const char * str);
        void loadCollectionConfig();

        virtual void boardcast_packageRemove(int x,int y,const std::string & uuid)=0;
        virtual void boardcast_packageAdd(int x,int y,const std::string & uuid,const std::string & text)=0;
        virtual void sendAddr_packageAdd(const RakNet::SystemAddress & addr,int x,int y,const std::string & uuid,const std::string & text)=0;
};

}
}
#endif // SMOOTHLY_SERVER_PACKAGE
