#ifndef __ALPACA_CLASS_HPP__
#define __ALPACA_CLASS_HPP__

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "sensors/RainSensorClass/RainSensorClass.hpp"

class ascom_alpaca
{
    private:
        WiFiUDP _udp;
        uint16_t _port_discovery;
        uint16_t _port_device;
        bool _was_connected = false;
        void _handle_discovery (void);

    public:
        static const char * get_uid (void);
        static uint32_t get_last_api_call_time (void);
        void begin (const uint16_t port_discovery, const uint16_t port_device);
        void loop (const bool connected);
};

extern ascom_alpaca alpaca;

#endif