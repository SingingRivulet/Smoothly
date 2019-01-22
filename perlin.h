#ifndef SMOOTHLY_PERLIN
#define SMOOTHLY_PERLIN
#include <math.h>
namespace smoothly{
    class perlin3d{
        public:
            int seed;
            int num;
            float step;
            float height;
            
            perlin3d();
            ~perlin3d();
            
            float rand(int x , int y , int z);
            float smooth(int x,int y,int z);
            float interpolate(float a,float b,float x);
            float inoise(float x,float y,float z);
            float get(float x , float y , float z);
    };
}
#endif
