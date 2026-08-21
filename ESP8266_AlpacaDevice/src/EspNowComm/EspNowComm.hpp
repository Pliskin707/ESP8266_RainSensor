#ifndef __ESP_NOW_COMM_HPP__
#define __ESP_NOW_COMM_HPP__

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <espnow.h>
#include "../../espnow_types.hpp" // shared with the other project

class esp_now_comm_class
{
    private:


    public:
        bool begin (const uint8_t txRetries = 3);
        void loop (void) {}; // nothing for now
        static bool is_device_rain_connected (void);
        static bool is_rain_state_valid (void);
        static bool is_temperature_valid (void);
        static bool is_humidity_valid (void) ;
        static float is_raining (void);
        static float temperature (void);
        static float humidity (void);
        static float device_rain_battery (void);
};

/** TODO
 * 
 * [ ] if an Alpaca client requests "connect" or "disconnect" send a command to the rain sensor to deep-sleep shorter or longer
 */

extern esp_now_comm_class esp_now_comm;    // \brief global instance for extended ESP-NOW functionality  
#endif