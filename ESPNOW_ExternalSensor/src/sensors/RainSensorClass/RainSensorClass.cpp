#include "RainSensorClass.hpp"
#include "config.hpp"

rain_sensor_class rain_sensor{PIN_RAIN_SENSOR};

void rain_sensor_class::begin(void)
{
    pinMode(_pin_digital_in, INPUT);
}

bool rain_sensor_class::is_raining(void) const
{
    return !digitalRead(_pin_digital_in);
}
