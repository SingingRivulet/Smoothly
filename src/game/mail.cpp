#include "mail.h"
#include <QString>
#include <QStringList>

namespace smoothly{

mail::mail():mailLastPage(128,264,128+64,264+32),mailNextPage(192,264,192+64,264+32){
    haveNewMail = false;
    showMails = false;
    showMailText = false;
    mail_focus = 0;
    mail_focus_line = -1;
    mail_page = 1;
    mailPackage_button = -1;
    mailPackagePickingup = false;
    //mails.push_back(mail_t("{\"title\":\"测试\",\"text\":\"测试1\"}"));
    //mails.push_back(mail_t("{\"title\":\"测试\",\"text\":\"测试2\",\"package\":\"aaaa\"}"));
    resetShowingMails();
}

void mail::msg_newMail(){
    haveNewMail = true;
}

void mail::msg_mail(const char * text){
    mails.push_back(mail_t(text));
}

void mail::msg_mailPackagePickedUp(){
    mailPackagePickingup = false;
}

void mail::getMail(){
    mails.clear();
    cmd_getMail();
}

void mail::onDraw(){
    fire::onDraw();
    try{
        if(showMails){
            haveNewMail = false;
            if(showMailText){
                mailPackage_button = -1;
                auto & m = mails.at(mail_focus);
                printString(m.title,30);
                auto h = printString(m.text);
                if(!m.mailPackage.empty()){
                    mailPackage_button = h+10;
                    printString(L"此邮件有附件，点击收取",mailPackage_button);
                    if(mailPackagePickingup){
                        printString(L"收取中……",h+30);
                    }
                }
            }else{
                if(mails.empty()){
                    printString(L"无新邮件",64);
                }else{
                    int start = (mail_page-1)*10;
                    resetShowingMails();

                    ttf->draw(L"<<" , mailLastPage,video::SColor(255,255,255,255));
                    ttf->draw(L">>" , mailNextPage,video::SColor(255,255,255,255));

                    for(int i=0;i<10;++i){
                        auto & m = mails.at(i+start);

                        int h = i*20+64;

                        if(i==mail_focus_line)
                            ttf->draw(m.title.c_str() , core::rect<s32>(64,h,width,height),video::SColor(255,255,255,255));
                        else
                            ttf->draw(m.title.c_str() , core::rect<s32>(64,h,width,height),video::SColor(255,128,128,128));

                        showingMails[i] = i+start;
                    }
                }
            }
        }else if(haveNewMail){
            ttf->draw(L"收到新邮件，按N查看" , core::rect<s32>(width-150,height-202,width,height-172),video::SColor(255,255,255,255));
        }
    }catch(...){}
}
void mail::pickupFocusMailPackage(){
    mailPackagePickingup = true;
    try{
        auto & m = mails.at(mail_focus);
        cmd_pickupMailPackage(mainControl.c_str() , m.mailPackage.c_str());
    }catch(...){}
}
void mail::setMailFocusByScreen(int x, int y){
    mail_focus = -1;
    mail_focus_line = -1;
    if(x>64 && y>64 && y<64+20*10){
        int c = (y-64)/20;
        if(c>=0 && c<10){
            mail_focus = showingMails[c];
            mail_focus_line = c;
        }
    }
}

void mail::mail_t::loadString(const char * s){
    auto json = cJSON_Parse(s);

    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_String){
                if(strcmp(c->string,"uuid")==0){
                    uuid        = c->valuestring;
                }else if(strcmp(c->string,"title")==0){
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
                    title       = conv.from_bytes(c->valuestring);
                }else if(strcmp(c->string,"text")==0){
                    text.clear();
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
                    QStringList list = QString(c->valuestring).split("\n");
                    for(int i = 0; i< list.size();++i){
                        text.push_back(conv.from_bytes(list.at(i).toStdString()));
                    }
                }else if(strcmp(c->string,"package")==0){
                    mailPackage = c->valuestring;
                }
            }
            c = c->next;
        }

        cJSON_Delete(json);
    }
}

}
