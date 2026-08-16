#ifndef __ALPACA_CLASS_HPP__
#define __ALPACA_CLASS_HPP__

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <ArduinoJson.h>
#include <vector>

class ascom_alpaca
{
    private:
        WiFiUDP _udp;
        WiFiServer _listener;
        std::vector<WiFiClient> _clients;
        uint16_t _port_device;
        bool _was_connected = false;
        void _handle_discovery (void);
        void _handle_new_clients (void);

    public:
        static const char * get_uid (void);
        void begin (const uint16_t port_discovery, const uint16_t port_device);
        void loop (const bool connected);
};

#endif