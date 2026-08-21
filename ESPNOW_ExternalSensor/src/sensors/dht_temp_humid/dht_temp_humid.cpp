#include "dht_temp_humid.hpp"

TempHumidSensorClass temp_humidity_sensor;

#define MIN_SUPPLY_DURATION_MS (750uL) // this value was tested with trial-and-error

void TempHumidSensorClass::begin(void)
{
    pinMode(PIN_HUMID_TEMP_SENSOR_SUPPLY, OUTPUT);
    digitalWrite(PIN_HUMID_TEMP_SENSOR_SUPPLY, HIGH);
    _pin_active_since = millis();
    DHT::begin();
}

void TempHumidSensorClass::delay_until_ready(void)
{
    if (!_pin_active_since)
        return;
        
    const uint32_t ms_since_supply = millis() - _pin_active_since;
    if (ms_since_supply > MIN_SUPPLY_DURATION_MS) 
        return;

    delay(MIN_SUPPLY_DURATION_MS - ms_since_supply);
    return;
}
