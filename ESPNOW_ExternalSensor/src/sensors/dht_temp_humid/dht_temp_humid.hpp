#include <DHT.h>
#include "config.hpp"

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

class TempHumidSensorClass : public DHT
{
    private:
        uint32_t _pin_active_since = 0uL;

    public:
        TempHumidSensorClass() : DHT{PIN_HUMID_TEMP_SENSOR_DATA, DHTTYPE} {};
        void begin (void);
        void delay_until_ready (void);
};

extern TempHumidSensorClass temp_humidity_sensor;