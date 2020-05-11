#ifndef SMOOTHLY_SERVER_BAG
#define SMOOTHLY_SERVER_BAG
#include "body.h"
#include "../utils/uuid.h"
#include "../utils/cJSON.h"
#include <unordered_set>
namespace smoothly{
namespace server{

class bag:public body{
    public:
        ////////////////////////////////////////////////////////////
        struct bag_tool_conf{//工具
                int  id;
                int  weight;    //重量
                int  durability;//耐久度
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
                std::string inbag;
                bag * parent;
                void toString(std::string & str);
                void loadString(const std::string & str);//使用前请先设置parent
                inline bag_tool(){

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
                int durability,maxDurability;
                bag * parent;
                std::string                                 uuid;       //自己的uuid(load的时候不会被设置，需要手动设置)

                inline bag_inner(){
                    resources.clear();
                    tools.clear();
                    durability = 0;
                    maxDurability = 0;
                    parent = NULL;
                    uuid.clear();
                }
                inline bag_inner(const bag_inner & i){
                    resources   =   i.resources;
                    tools       =   i.tools;
                    durability  =   i.durability;
                    maxDurability = i.maxDurability;
                    parent      =   i.parent;
                    uuid        =   i.uuid;
                }
                inline const bag_inner & operator=(const bag_inner & i){
                    resources   =   i.resources;
                    tools       =   i.tools;
                    durability  =   i.durability;
                    maxDurability = i.maxDurability;
                    parent      =   i.parent;
                    uuid        =   i.uuid;
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
        std::map<int,int> maxDurabilities;

        bool consume(const std::string & bag_uuid,const std::string & tool_uuid,int num)noexcept;//消耗耐久
        bool consume(const std::string & tool_uuid,int num)noexcept;//消耗耐久

        bag();
        ~bag();

        void release()override;
        void loop()override;
};

}
}
#endif // SMOOTHLY_SERVER_BAG
