#ifndef SMOOTHLY_CONTROL
#define SMOOTHLY_CONTROL
#include "fire.h"
namespace smoothly{

class control:public fire{
    public:
        control();
        void processControl();
    private:
        class eventRecv:public irr::IEventReceiver{
            public:
                control * parent;
                virtual bool OnEvent (const irr::SEvent &event);
        }receiver;
        struct{
        }status;
        void addEventRecv();
};

}

#endif
