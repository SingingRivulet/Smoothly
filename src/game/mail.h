#ifndef SMOOTHLY_MAIL
#define SMOOTHLY_MAIL

#include "fire.h"

namespace smoothly{

class mail:public fire{
    public:
        mail();
        void msg_newMail()override;
        void msg_mail(const char * text)override;
        void msg_mailPackagePickedUp()override;

        struct mail_t{
                std::string uuid,mailPackage;
                std::wstring title;
                std::vector<std::wstring> text;
                void loadString(const char * s);
                mail_t(){}
                mail_t(const char * s){
                    loadString(s);
                }
                mail_t(const mail_t & i){
                    uuid            = i.uuid;
                    title           = i.title;
                    text            = i.text;
                    mailPackage     = i.mailPackage;
                }
                const mail_t & operator=(const mail_t & i){
                    uuid            = i.uuid;
                    title           = i.title;
                    text            = i.text;
                    mailPackage     = i.mailPackage;
                    return * this;
                }
        };
        std::vector<mail_t> mails;
        bool haveNewMail;
        void getMail();
        inline void deleteMail(const std::string & uuid){
            cmd_deleteMail(uuid.c_str());
            showMailText = false;
            getMail();
        }
        inline void deleteMail(){
            try{
                deleteMail(mails.at(mail_focus).uuid);
            }catch(...){}
        }

        void onDraw()override;
        int mail_page,mail_focus,mail_focus_line,mailPackage_button;
        bool showMails,showMailText,mailPackagePickingup;
        int showingMails[10];

        void setMailFocusByScreen(int x,int y);
        void pickupFocusMailPackage();

        void resetShowingMails(){
            for(int i=0;i<10;++i){
                showingMails[i] = -1;
            }
        }

        irr::core::rect<s32> mailLastPage;
        irr::core::rect<s32> mailNextPage;
};

}

#endif // SMOOTHLY_MAIL
