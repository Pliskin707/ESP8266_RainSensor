#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <espnow.h>
#include "../../espnow_types.hpp" // shared with the other project

class esp_now_comm_class
{
    private:
        static void _txCallback (uint8_t * mac_addr, uint8_t status);
        void _retrySend (void);

    public:
        bool begin (const uint8_t txRetries = 3);
        int add_peer (const mac address);
        int send (const mac destination, const void * data, const uint8_t len);
        int send (const void * data, const uint8_t len);
        static bool tx_success (void);
};

extern esp_now_comm_class esp_now_comm;    // \brief global instance for extended ESP-NOW functionality