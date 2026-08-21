#include "EspNowComm.hpp"

esp_now_comm_class esp_now_comm;
static uint8_t _txRetries = 0;
static bool _tx_success = false;
static struct
{
    uint8_t txRetriesRemaining;
    uint8_t len;
    mac dest;
    const uint8_t * data;
} _txPackage = {0, 0, {0}, nullptr};

bool esp_now_comm_class::begin (const uint8_t txRetries)
{
    if (esp_now_init() == 0)
    {
        _txRetries = txRetries;
        esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);  // priority is given to the station interface (not the SoftAP)
        esp_now_register_send_cb(&this->_txCallback);

        // try to save power
        // doc recommends a multiple of 100ms
        // esp_wifi_connectionless_module_set_wake_interval(200); // ! seems to not be exposed
        return true;
    }

    return false;
}

int esp_now_comm_class::add_peer (const mac address)
{
    return esp_now_add_peer((uint8_t *) address, ESP_NOW_ROLE_IDLE, 11, nullptr, 0);   // from the documentation: "The peer's Role does not affect any function, but only stores the Role information for the application layer."
}

int esp_now_comm_class::send (const mac destination, const void * data, const uint8_t len)
{
    _txPackage.txRetriesRemaining = _txRetries;
    _txPackage.len = len;
    memcpy(_txPackage.dest, destination, sizeof(mac));
    _txPackage.data = (const uint8_t *) data;

    return esp_now_send((uint8_t *) _txPackage.dest, (uint8_t *) _txPackage.data, _txPackage.len);
}

int esp_now_comm_class::send (const void * data, const uint8_t len)
{
    return this->send(nullptr, data, len);
}

bool esp_now_comm_class::tx_success(void)
{
    return _tx_success;
}

void esp_now_comm_class::_txCallback (uint8_t * mac_addr, uint8_t status)
{
    _tx_success = (status == 0);
    if ((status != 0) && _txPackage.txRetriesRemaining)
        esp_now_comm._retrySend();
}

void esp_now_comm_class::_retrySend (void)
{
    if (_txPackage.txRetriesRemaining)
        _txPackage.txRetriesRemaining--;

    esp_now_send((uint8_t *) _txPackage.dest, (uint8_t *) _txPackage.data, _txPackage.len);
}