#include "BeeperClass.hpp"

beeper_class beeper;

void beeper_class::begin(void)
{
    pinMode(PIN_BEEPER, OUTPUT);
    turn_on_until(millis() + 500uL);
}

void beeper_class::loop(void)
{
    digitalWrite(PIN_BEEPER, millis() < _beep_until);
}

beeper_class& beeper_class::turn_on_until(uint32_t millis_timestamp)
{
    // sanity check
    const uint32_t sys_time = millis();

    if ((sys_time < millis_timestamp) && ((millis_timestamp - sys_time) > 60000uL))
        _beep_until = sys_time + 60000uL;   // do not beep forever
    else
        _beep_until = millis_timestamp;

    return *this;
}

beeper_class& beeper_class::turn_off(void)
{
    _beep_until = 0uL;
    digitalWrite(PIN_BEEPER, LOW);
    return *this;
}
