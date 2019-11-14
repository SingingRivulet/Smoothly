//地形生成
#ifndef SMOOTHLY_WORLD_TERRAIN
#define SMOOTHLY_WORLD_TERRAIN
#include "SimplexNoise.h"
#include "terrain_item.h"
#include <math.h>
namespace smoothly{
namespace world{
namespace terrain{
////////////////
class perlin3d:public SimplexNoise{
    public:
        inline perlin3d():SimplexNoise(){}
        inline float get(float x , float y , float z){
            return noise(x, y, z);
        }
};
class terrain{
    public:
        float   altitudeArg,
                altitudeK,
                hillArg,
                hillK,
                temperatureArg,
                temperatureK,
                humidityArg,
                humidityK,
                temperatureMin,
                temperatureMax,
                humidityMax,
                humidityMin;
    public:
        inline void setSeed(int seed){
            genRiver(seed);
        }
        
        struct river{
            int a,b,c,d,e,f,g,h,i,j;
            float w;
            inline float getD(float x,float y){
                float x2=x *x;
                float x4=x2*x2;
                float x8=x4*x4;

                float y2=y *y;
                float y4=y2*y2;
                float y8=y4*y4;
                
                return (
                    x8*a + y8*b + 
                    x4*c + y4*d + 
                    x2*e + y2*f +
                    x *g + y *h +
                    i +    j
                );
            }
            inline float getK(float x,float y){
                float de=fabs(getD(x,y));
                float s=sigmoid(de/w-4);
                return s;
            }
            inline float sigmoid(float x){
                return (1/(1+exp(-x)));
            }
        };
        
        river rivers[3];
        
        inline void genRiver(int seed){
            predictableRand randg;
            randg.setSeed(seed);
            for(int i=0;i<3;i++){
                rivers[i].a=(randg.rand())%1024-512;
                rivers[i].b=(randg.rand())%1024-512;
                rivers[i].c=(randg.rand())%1024-512;
                rivers[i].d=(randg.rand())%1024-512;
                rivers[i].e=(randg.rand())%1024-512;
                rivers[i].f=(randg.rand())%1024-512;
                rivers[i].g=(randg.rand())%1024-512;
                rivers[i].h=(randg.rand())%1024-512;
                rivers[i].i=(randg.rand())%1024-512;
                rivers[i].j=(randg.rand())%1024-512;
                rivers[i].w=(randg.rand())%8+2;
            }
        }
        
        inline float getRiver(float x , float y){
            float K=1;
            for(int i=0;i<3;i++){
                K*=rivers[i].getK(x,y);
            }
            return K;
        }
        
        inline float getHightf(float x , float y){
            return generator.get(x/altitudeArg , y/altitudeArg , 1024)*altitudeK;
        }
        inline float getHillHight(float x , float y){//山高
            return generator.get(x/hillArg , y/hillArg , 2048)*hillK;
        }
        inline float getAltitude(float x , float y){//海拔
            return getHightf(x/32 , y/32);
        }
        
        //真实高度=海拔+山高
        inline float getRealHight(int x , int y){
            return (getHillHight(x,y)+getAltitude(x,y))*getRiver(x,y);
            //return x+y;
        }
        
        inline float getTemperatureF(float x , float y){
            return generator.get(x/temperatureArg , y/temperatureArg , 4096)*temperatureK;
        }
        
        inline float getHumidityF(float x , float y){
            return generator.get(x/humidityArg , y/humidityArg , 8192)*humidityK;
        }
        
        inline float getDefaultCameraHight(float x , float y){
            return getRealHight(x,y)+1;
        }
        
        float genTerrain(
            float ** ,  //高度图边长=chunk边长+1
            int32_t x , int32_t y , //chunk坐标，真实坐标/32
            int pointNum
        );//返回最大值
        
        inline static short getLevel(float min,float max,float v){
            if(v<=min)
                return 0;
            if(v>=max)
                return 16;
            float l=max-min;
            float r=v-min;
            return (r/l)*16;
        }
        inline short getHumidityLevel(float x,float y){
            return getLevel(humidityMin,humidityMax,getHumidityF(x,y));
        }
        inline short getTemperatureLevel(float x,float y){
            return getLevel(temperatureMin,temperatureMax,getTemperatureF(x,y));
        }
    private:
        perlin3d generator;
};
////////////////
}//////terrain
}//////world
}//////smoothly
#endif
