#ifndef SMOOTHLY_CONTROL
#define SMOOTHLY_CONTROL
#include "group_ai.h"
namespace smoothly{

class control:public group_ai{
    public:
        control();
        void processControl();
        void setGUIMode(bool m);
    private:
        class eventRecv:public irr::IEventReceiver{
            public:
                control * parent;
                virtual bool OnEvent (const irr::SEvent &event);
        }receiver;
        struct{
        }status;
        void addEventRecv();

    private:
        irr::gui::IGUIImage * menu_window;
        irr::gui::IGUIButton * menu_exit,* menu_cmd;
        irr::gui::IGUIEditBox * menu_cmd_line;
        void processCmd();
        void processCmd_mission(std::istringstream & iss);
        void processCmd_mail(std::istringstream & iss);
        void processCmd_ik(std::istringstream & iss);
        void processCmd_test(std::istringstream & iss);
};

}

#endif
