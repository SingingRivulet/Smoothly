#ifndef SMOOTHLY_GRAPHBASE
#define SMOOTHLY_GRAPHBASE
#include <irrlicht.h>

namespace smoothly{

class graphBase{
    public:
        irr::s32 enableClipY,clipYUp;
        irr::f32 clipY;
        bool haveReflection;
        bool haveRefraction;
        bool halfFrameWater;
        graphBase(){
            enableClipY = 0;
            haveReflection = true;
            haveRefraction = true;
            halfFrameWater = true;
        }
};

}

#endif // SMOOTHLY_GRAPHBASE
