#include "terrain.h"
namespace smoothly{
//噪声发生器，不解释
perlin3d::perlin3d(){
    num=4;
    step=4;
    height=4;
}

perlin3d::~perlin3d(){

}

float perlin3d::rand(int x , int y , int z){
    return 
        (((
           (x&(48271*seed))+
           (y&(23456+seed))+
           (z^(98704^seed))
        )&0x7ffffff))/10000.0d;
}

float perlin3d::smooth(int x,int y,int z){
    float t;
    t =rand(x,y,z)/2+rand(x+1,y,z)/4+rand(x-1,y,z)/8;
    t+=rand(x,y,z)/2+rand(x,y+1,z)/4+rand(x,y-1,z)/8;
    t+=rand(x,y,z)/2+rand(x,y,z+1)/4+rand(x,y,z-1)/8;
    return t;
}

float perlin3d::interpolate(float a,float b,float x){
    float ft=x*3.1415926535;
    float f=(1-cos(ft))*0.5;
    return a*(1-f)+b*f;
}

float perlin3d::inoise(float x,float y,float z){
    int x0=(int)x;
    int x1=x0+1;
    int y0=(int)y;
    int y1=y0+1;
    int z0=(int)z;
    int z1=z0+1;
    auto v1=smooth(x0,y0,z0);
    auto v2=smooth(x1,y0,z0);
    auto v3=smooth(x0,y1,z0);
    auto v4=smooth(x1,y1,z0);
    auto v5=smooth(x0,y0,z1);
    auto v6=smooth(x1,y0,z1);
    auto v7=smooth(x0,y1,z1);
    auto v8=smooth(x1,y1,z1);
    auto i1=interpolate(v1,v2,y-y0);
    auto i2=interpolate(v3,v4,y-y0);
    auto i3=interpolate(v5,v6,y-y0);
    auto i4=interpolate(v7,v8,y-y0);
    i1=interpolate(i1,i2,z-z0);
    i3=interpolate(i3,i4,z-z0);
    return interpolate(i1,i3,x-x0);
}

float perlin3d::get(float x , float y , float z){
    float   total=0.0f;
    int     n    =this->num;
    float   p    =this->height;

    int     f    =this->step;
    float   a    =1;
    int     i;
    for(i=0;i<n;i++){
        total+=inoise(x*f,y*f,z*f)*a;
        f=f*2;
        a=a*p;
    }
    return total;
}

}//namespace smoothly
