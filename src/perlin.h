#ifndef SMOOTHLY_PERLIN
#define SMOOTHLY_PERLIN
#include <math.h>
#include "SimplexNoise.h"
namespace smoothly{
    class perlin3d:public SimplexNoise{
        public:
            inline perlin3d():SimplexNoise(){}
            inline float get(float x , float y , float z){
                return noise(x, y, z);
            }
    };
}
#endif
