#ifndef SMOOTHLY_SERVER_BAG
#define SMOOTHLY_SERVER_BAG
#include "body.h"
#include "../utils/uuid.h"
#include "../utils/cJSON.h"
#include <unordered_set>

#include <raknet/RakPeerInterface.h>

namespace smoothly{
namespace server{

class bag:public body{
    public:
        ////////////////////////////////////////////////////////////
        struct bag_tool_conf{//工具
                int  id;
                int  weight;    //重量
                int  durability;//耐久度
                int  power;     //能量/电量/弹匣
                std::set<int> wearing;
        };
        std::map<int,bag_tool_conf*> tool_config;
        ////////////////////////////////////////////////////////////
        struct bag_resource_conf{//资源
                int id;
                int weight;
        };
        std::map<int,bag_resource_conf*> resource_config;
        ////////////////////////////////////////////////////////////
        struct bag_tool{
                bag_tool_conf * conf;
                int durability;
                int power;//能量/电量/弹匣
                std::string inbag;
                bag * parent;
                void toString(std::string & str);
                void loadString(const std::string & str);//使用前请先设置parent

                void reload(){
                    power = conf->power;
                }

                inline bag_tool(){
                    conf        = NULL;
                    durability  = 0;
                    inbag.clear();
                    parent      = NULL;
                    power       = 0;
                }
                inline bag_tool(const bag_tool & i){
                    conf        = i.conf;
                    durability  = i.durability;
                    inbag       = i.inbag;
                    parent      = i.parent;
                    power       = i.power;
                }
                inline const bag_tool & operator=(const bag_tool & i){
                    conf        = i.conf;
                    durability  = i.durability;
                    inbag       = i.inbag;
                    parent      = i.parent;
                    power       = i.power;
                    return * this;
                }
        };
        class cache_tools_t:public cache<bag_tool>{//用cache存放
                void onExpire(const std::string &,bag_tool & )override;
                void onLoad(const std::string &, bag_tool & )override;
            public:
                bag * parent;
        }cache_tools;
        std::string createTool(int id);//创建一个新的工具
        ////////////////////////////////////////////////////////////
        struct bag_inner{
                std::map<int,int>                           resources;  //基础资源，弹药（id=>数量）
                std::unordered_set<std::string>             tools;      //工具（存放uuid）
                int weight,maxWeight;
                bag * parent;
                std::string                                 uuid;       //自己的uuid(load的时候不会被设置，需要手动设置)
                std::string                                 usingTool;  //使用中的工具

                inline bag_inner(){
                    resources.clear();
                    tools.clear();
                    weight = 0;
                    maxWeight = 0;
                    parent = NULL;
                    uuid.clear();
                    usingTool.clear();
                }
                inline bag_inner(const bag_inner & i){
                    resources   =   i.resources;
                    tools       =   i.tools;
                    weight  =   i.weight;
                    maxWeight = i.maxWeight;
                    parent      =   i.parent;
                    uuid        =   i.uuid;
                    usingTool   =   i.usingTool;
                }
                inline const bag_inner & operator=(const bag_inner & i){
                    resources   =   i.resources;
                    tools       =   i.tools;
                    weight  =   i.weight;
                    maxWeight = i.maxWeight;
                    parent      =   i.parent;
                    uuid        =   i.uuid;
                    usingTool   =   i.usingTool;
                    return * this;
                }

                //这三个记得处理好throw
                bool addResource(int id,int num);
                void addTool(const std::string & uuid);
                void removeTool(const std::string & uuid);

                void toString(std::string & str);
                void loadString(const std::string & str);
        };
        class cache_bag_inner_t:public cache<bag_inner>{//用cache存放
                void onExpire(const std::string &,bag_inner & )override;
                void onLoad(const std::string &, bag_inner & )override;//bag找不到会自动创建，config找不到会throw
            public:
                bag * parent;
        }cache_bag_inner;
        std::map<int,int> maxWeights;

        bool consume(const RakNet::SystemAddress & addr,const std::string & bag_uuid,const std::string & tool_uuid,int num,int pwn)noexcept;//消耗耐久
        bool consume(const RakNet::SystemAddress & addr, const std::string & tool_uuid, int num, int pwn)noexcept;//消耗耐久

        bag();
        ~bag();

        void sendBagToAddr(const RakNet::SystemAddress & addr, const std::string & uuid);
        bool addResource(const RakNet::SystemAddress & addr, const std::string & uuid,int id,int delta);
        void useTool(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID);//toolUUID为空或不存在时卸下
        void reloadTool(const RakNet::SystemAddress & addr, const std::string & uuid, const std::string & toolUUID);

        virtual void sendAddr_bag(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & text)=0;
        virtual void sendAddr_bag_resourceNum(const RakNet::SystemAddress & addr,const std::string & uuid,int id,int num)=0;
        virtual void sendAddr_bag_toolDur(const RakNet::SystemAddress & addr,const std::string & uuid,int dur,int pwr)=0;
        virtual void sendAddr_bag_tool_add(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID)=0;
        virtual void sendAddr_bag_tool_remove(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID)=0;
        virtual void sendAddr_bag_tool_use(const RakNet::SystemAddress & addr,const std::string & uuid,const std::string & toolUUID)=0;

        void release()override;
        void loop()override;
};

}
}
#endif // SMOOTHLY_SERVER_BAG
