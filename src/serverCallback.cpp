#include "attaching.h"
namespace smoothly{

bool attaching::setAttachingCallback(
    const std::string & user , 
    subs * p, 
    bodyAttaching & att//不声明const是因为可以被回调函数修改
){
    
    return true;
}

}