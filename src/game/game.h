#ifndef SMOOTHLY_GAME
#define SMOOTHLY_GAME
#include "control.h"
namespace smoothly {

class game:public control{
    public:
        game(const char * addr,unsigned short port);
        ~game();
        bool gameLoop();
        inline bool ok(){
            //printf("[debug]check status\n");
            return (device->run() && driver && dynamicsWorld && running);
        }
};

}
#endif
