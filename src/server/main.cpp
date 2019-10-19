#include "server.h"
int main(){
    smoothly::server::server S;
    S.start(39065,256);
    while(1){
        S.recv();
        RakSleep(30);
    }
    S.release();
    return 0;
}