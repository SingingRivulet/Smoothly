#ifndef SMOOTHLY_WEATHER
#define SMOOTHLY_WEATHER
#include "shadow.h"
namespace smoothly{

class weather:public shadow{
    public:
        weather();
        void updateWeather(int)override;
};

}
#endif // SMOOTHLY_WEATHER
