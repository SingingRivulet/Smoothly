#ifndef SMOOTHLY_SERVER_TECHNOLOGY
#define SMOOTHLY_SERVER_TECHNOLOGY

#include "package.h"

namespace smoothly{
namespace server{

class technology:public package{
    public:

        struct tech_conf_t{
                int need;
                int probability;//确定目标后解锁科技的概率
        };
        std::map<int,tech_conf_t> tech_conf;//科技配置表

        struct tech_user_t{
                technology * parent;
                std::string uuid;
                std::set<int> unlocked;//已经解锁的科技
                int target;//解锁目标
                int target_need;//解锁目标的前置科技
                int target_prob;//解锁目标的概率

                void init();
                void loadString(const std::string & str);
                void toString(std::string & str);

                bool checkTech(const RakNet::SystemAddress & addr,int id);//检查科技是否可用
                void tryUnlockTech(const RakNet::SystemAddress & addr, int activeId);//尝试解锁科技

                void setUnlockTarget(const RakNet::SystemAddress & addr,int id);
        };
        class cache_tech_user_t:public cache<tech_user_t>{//用cache存放
                void onExpire(const std::string &,tech_user_t & )override;
                void onLoad(const std::string &, tech_user_t & )override;
            public:
                technology * parent;
        }cache_tech_user;

        void sendAddr_unlockedTech(const RakNet::SystemAddress & addr, const std::string & uuid);

        virtual void sendAddr_unlockTech(const RakNet::SystemAddress & addr,bool newTech,int id)=0;
        virtual void sendAddr_techTarget(const RakNet::SystemAddress & addr,bool newTarget,int id)=0;

        bool checkTech(const RakNet::SystemAddress & addr,const std::string & uuid,int id);//只应该在任务执行时调用。因为此操作可能会导致其他科技解锁

        void setTechTarget(const RakNet::SystemAddress & addr,const std::string & uuid,int id);

        void loop()override;
        void release()override;

        technology();
};

}
}

#endif // TECHNOLOGY_H
