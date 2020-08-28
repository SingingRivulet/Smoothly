#ifndef SMOOTHLY_GRAPHBASE
#define SMOOTHLY_GRAPHBASE
#include <irrlicht.h>

namespace smoothly{

class graphBase{
    public:
        irr::s32 enableClipY,clipYUp;
        irr::f32 clipY;
        graphBase(){
            enableClipY = 0;
        }
};

}

#endif // SMOOTHLY_GRAPHBASE
