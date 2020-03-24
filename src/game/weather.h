#ifndef SMOOTHLY_WEATHER
#define SMOOTHLY_WEATHER
#include "cloud.h"
namespace smoothly{

class weather:public cloud{
    public:
        weather();
        void updateWeather(int)override;
};

}
#endif // SMOOTHLY_WEATHER
