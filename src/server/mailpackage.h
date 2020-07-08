#ifndef SMOOTHLY_SERVER_MAILPACKAGE
#define SMOOTHLY_SERVER_MAILPACKAGE

#include "package.h"

namespace smoothly{
namespace server{

class mailPackage:public package{
    // 邮件/礼包兑换码
    public:
        mailPackage();
        struct mailPackage_t:package_base{
                void toString(std::string & s);
                void loadString(const std::string & s);
                void clear(){
                    this->resource.clear();
                    this->tool.clear();
                }
                mailPackage_t():package_base(){}
                mailPackage_t(const mailPackage_t & i):package_base(i.resource , i.tool){
                }
                const mailPackage_t & operator=(const mailPackage_t & i) {
                    resource = i.resource;
                    tool     = i.tool;
                    return * this;
                }
        };

        void pickupMailPackage(const std::string & body,const std::string & mpuuid);
        bool getMailPackage(const std::string & mpuuid, mailPackage_t &);

        std::string putMailPackage(mailPackage_t &);
        void putMailPackage(const std::string & mpuuid,mailPackage_t &);

        void removeMailPackage(const std::string & mpuuid);

        std::string getTimeUUID();
        void getUserMail(const std::string & user,std::function<void(const std::string &)> callback,const std::string & startAt = std::string(),int num = 20);
        void addMail(const std::string & user,const std::string & uuid,const std::string & text);
        void deleteMail(const std::string & user,const std::string & uuid);

        void addMail(const std::string & user,const std::string & title,const std::string & text,const std::string & package);

        void sendMails(const RakNet::SystemAddress & addr , const std::string & user);

        virtual void sendUser_newMail(const std::string & user)=0;
        virtual void sendAddr_mail(const RakNet::SystemAddress & addr , const std::string & text)=0;
};

}
}

#endif // SMOOTHLY_SERVER_MAILPACKAGE
