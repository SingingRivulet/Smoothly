#include "weather.h"
#include <algorithm>

namespace smoothly{

weather::weather(){

}
template<typename T1,typename T2>
inline T2 interpolation(T1 x0,T2 y0,T1 x1,T2 y1,T1 x){
    return y0+(y1-y0)*(x-x0)/(x1-x0);
}

struct skyConfig{
    int time;
    float lightness,astrAtomScat;
    irr::core::vector3df astrLight,astrColor,glow;
};

static const skyConfig sunnySky[]={//晴天
    {-15    ,   0.01    ,   0.8     ,   irr::core::vector3df(9.1,7.1,5.1)  ,   irr::core::vector3df(6.1,4.1,2.1),irr::core::vector3df(0.1,0.1,0.1)},
    {7      ,   0.1     ,   0.8     ,   irr::core::vector3df(9.1,7.1,5.1)  ,   irr::core::vector3df(6.1,4.1,2.1),irr::core::vector3df(0.8,0.8,0.8)},//朝霞开始
    {30     ,   0.1     ,   0.8     ,   irr::core::vector3df(6.1,4.1,2.1)  ,   irr::core::vector3df(6.1,4.1,2.1),irr::core::vector3df(1.0,1.0,1.0)},//朝霞
    {90     ,   1.2     ,   0.8     ,   irr::core::vector3df(1.2,1.2,1.2)  ,   irr::core::vector3df(6.1,6.1,6.1),irr::core::vector3df(1.0,1.0,1.0)},//朝霞结束
    {300    ,   1.6     ,   0.8     ,   irr::core::vector3df(1.6,1.6,1.6)  ,   irr::core::vector3df(6.9,6.9,6.9),irr::core::vector3df(1.0,1.0,1.0)},//正午
    {540    ,   1.2     ,   0.8     ,   irr::core::vector3df(6.1,4.1,2.1)  ,   irr::core::vector3df(6.1,6.1,6.1),irr::core::vector3df(1.0,1.0,1.0)},//晚霞开始
    {580    ,   0.4     ,   0.8     ,   irr::core::vector3df(6.1,4.1,2.1)  ,   irr::core::vector3df(6.1,4.1,2.1),irr::core::vector3df(1.0,1.0,1.0)},//晚霞
    {599    ,   0.2     ,   0.8     ,   irr::core::vector3df(6.1,4.1,2.1)  ,   irr::core::vector3df(6.1,4.1,2.1),irr::core::vector3df(1.0,1.0,1.0)},
    {600    ,   0.1     ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//月出
    {612    ,   0.01    ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {700    ,   0.01    ,   5       ,   irr::core::vector3df(20 ,20 ,20 )  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {1100   ,   0.01    ,   5       ,   irr::core::vector3df(20 ,20 ,20 )  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {1170   ,   0.001   ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//日出预备
    {1215   ,   0.01    ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)}
};

static const skyConfig cloudySky[]={//阴天
    {-15    ,   0.01    ,   0.8     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.1,0.1,0.1)},
    {7      ,   0.1     ,   0.8     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.6,0.6,0.6)},//朝霞开始
    {30     ,   0.6     ,   0.4     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},//朝霞
    {90     ,   0.8     ,   0.4     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},//朝霞结束
    {300    ,   0.9     ,   0.4     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},//正午
    {540    ,   0.8     ,   0.4     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},//晚霞开始
    {580    ,   0.6     ,   0.4     ,   irr::core::vector3df(0.2,0.2,0.2)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},//晚霞
    {599    ,   0.6     ,   0.4     ,   irr::core::vector3df(0.4,0.4,0.4)  ,   irr::core::vector3df(1.0,1.0,1.0),irr::core::vector3df(0.8,0.8,0.8)},
    {600    ,   0.1     ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//月出
    {612    ,   0.01    ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {700    ,   0.01    ,   5       ,   irr::core::vector3df(20 ,20 ,20 )  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {1100   ,   0.01    ,   5       ,   irr::core::vector3df(20 ,20 ,20 )  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//晚霞结束
    {1170   ,   0.001   ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)},//日出预备
    {1215   ,   0.01    ,   0.1     ,   irr::core::vector3df(2.0,2.0,2.0)  ,   irr::core::vector3df(4.0,4.0,4.0),irr::core::vector3df(0.01,0.01,0.01)}
};

static float randf(uint t){
    uint s = 214013*t+2531011;
    uint n = s>>16&((1<<15)-1);
    return (n%32)/32.0;
}

void weather::updateWeather(int tm){

    int wcell = tm/300;
    int left  = wcell*300;
    int right = left+300;
    auto lw  = randf(left);
    auto rw  = randf(right);

    cloudy = interpolation(left,lw,right,rw,tm);

    int ntm = tm%1200;//一天对应当前时间
    cloudThre = 1-cloudy;
    if(ntm<600){
        //计算太阳位置
        float x = cos((tm/600.0)*M_PI);
        float y = sin((tm/600.0)*M_PI);
        astronomical.set(x,y,0);
    }else{
        //计算月亮位置
        float x = cos(((tm-600)/600.0)*M_PI);
        float y = sin(((tm-600)/600.0)*M_PI);
        astronomical.set(x,y,0);
    }

    float slightness,sastrAtomScat;
    irr::core::vector3df sastrLight,sastrColor,sglow;
    const skyConfig * c = &sunnySky[0];
    //printf("[time]%d cloudy=%f %f %f:l=%d r=%d n=%d\n",ntm,cloudy,lw,rw,left,right,tm);
    while(c->time<1200){
        const skyConfig * next = c+1;
        if(next->time > ntm && c->time < ntm){//自己处于此时刻与下一时刻之间
            slightness       = interpolation(c->time,c->lightness,next->time,next->lightness,ntm);
            sastrAtomScat    = interpolation(c->time,c->astrAtomScat,next->time,next->astrAtomScat,ntm);
            sastrLight       = interpolation(c->time,c->astrLight,next->time,next->astrLight,ntm);
            sastrColor       = interpolation(c->time,c->astrColor,next->time,next->astrColor,ntm);
            sglow            = interpolation(c->time,c->glow,next->time,next->glow,ntm);
            break;
        }else if(c->time == ntm){
            slightness       = c->lightness;
            sastrAtomScat    = c->astrAtomScat;
            sastrLight       = c->astrLight;
            sastrColor       = c->astrColor;
            sglow            = c->glow;
            break;
        }
        ++c;
    }

    float clightness,castrAtomScat;
    irr::core::vector3df castrLight,castrColor,cglow;
    c = &cloudySky[0];
    while(c->time<1200){
        const skyConfig * next = c+1;
        if(next->time > ntm && c->time < ntm){//自己处于此时刻与下一时刻之间
            clightness       = interpolation(c->time,c->lightness,next->time,next->lightness,ntm);
            castrAtomScat    = interpolation(c->time,c->astrAtomScat,next->time,next->astrAtomScat,ntm);
            castrLight       = interpolation(c->time,c->astrLight,next->time,next->astrLight,ntm);
            castrColor       = interpolation(c->time,c->astrColor,next->time,next->astrColor,ntm);
            cglow            = interpolation(c->time,c->glow,next->time,next->glow,ntm);
            break;
        }else if(c->time == ntm){
            clightness       = c->lightness;
            castrAtomScat    = c->astrAtomScat;
            castrLight       = c->astrLight;
            castrColor       = c->astrColor;
            cglow            = c->glow;
            break;
        }
        ++c;
    }

    lightness    =   interpolation(0.0f,slightness     , 1.0f,clightness,cloudy);
    astrAtomScat =   interpolation(0.0f,sastrAtomScat  , 1.0f,castrAtomScat,cloudy);
    astrLight    =   interpolation(0.0f,sastrLight     , 1.0f,castrLight,cloudy);
    astrColor    =   interpolation(0.0f,sastrColor     , 1.0f,castrColor,cloudy);
    glow         =   interpolation(0.0f,sglow          , 1.0f,cglow,cloudy);

    if(cloudy>0.7){
        setRain((1-cloudy)*3);
    }else{
        setRain(0);
    }

    float shadowK = std::max(0.7f-cloudy,0.f);
    shadowK = std::min(shadowK,0.4f);
    shadowK/= 0.4;
    shadowFactor = 0.6*shadowK+0.1;

    lightTarget = camera->getPosition();
    auto level = getRealHight(floor(lightTarget.X/32.f)*32,floor(lightTarget.Z/32.f)*32);
    if(level<32)
        level = 32;
    lightTarget.Y = level;

    auto ldir = astronomical;
    ldir.Z = -ldir.X;
    ldir.X = 0;
    ldir.normalize();
    lightDir = ldir;
    ldir*=200;
    ldir+=lightTarget;
    light->setPosition(ldir);

    video::SLight lconf;
    lconf.DiffuseColor.set(glow.X,glow.Y,glow.Z);
    light->setLightData(lconf);
}

}
