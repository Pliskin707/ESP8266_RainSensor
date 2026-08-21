#ifndef __RAINSENS_HPP__
#define __RAINSENS_HPP__

#include <Arduino.h>

class rain_sensor_class
{
    private:
        const uint8_t _pin_digital_in;

    public:
        rain_sensor_class(const uint8_t pin) : _pin_digital_in(pin) {};
        void begin (void);
        bool is_raining (void) const;
};

extern rain_sensor_class rain_sensor;

#endif