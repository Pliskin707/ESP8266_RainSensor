#ifndef __BEEPER_CLASS_HPP__
#define __BEEPER_CLASS_HPP__

#include <Arduino.h>
#include "config.hpp"

class beeper_class
{
    private:
        uint32_t _beep_until = 0uL;

    public:
        void begin (void);
        void loop (void);
        beeper_class& turn_on_until (uint32_t millis_timestamp);
        beeper_class& turn_off (void);
};

extern beeper_class beeper;

#endif