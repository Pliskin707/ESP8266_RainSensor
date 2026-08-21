#include "./EspNowComm.hpp"
#include "projutils/projutils.hpp"

#define TIMEOUT_CONNECTED   (70000uL) // normal update is every 30 seconds, allow one missing package

esp_now_comm_class esp_now_comm;

static uint32_t _last_rx_device_rain_sensor = 0uL;
static uint32_t _last_rx_valid_rain_state = 0uL;
static uint32_t _last_rx_valid_temperature = 0uL;
static uint32_t _last_rx_valid_humidity = 0uL;
static bool _is_raining = false;
static float _temperature = 999.0f;
static float _humidity = 999.0f;
static float _device_rain_battery = 0.0f;  // %

#include <math.h>

typedef struct 
{
    float percent;
    float voltage;
} SoC_LUT_Entry;

// SOC/Voltage curve at 25°C
const SoC_LUT_Entry soc_curve_nmc[] =
{
    // %    U[V]
    { 0.0f, 0.0f},
    {10.0f, 3.4585f},
    {20.0f, 3.5525f},
    {30.0f, 3.6011f},
    {40.0f, 3.6470f},
    {50.0f, 3.7261f},
    {60.0f, 3.8302f},
    {70.0f, 3.9118f},
    {80.0f, 3.9995f},
    {90.0f, 4.0823f},
    {100.0f,4.1607f}
};

const uint_fast8_t soc_curve_nmc_num_elements = sizeof(soc_curve_nmc) / sizeof(soc_curve_nmc[0]);

float getMaxVoltage (void) 
{
    return soc_curve_nmc[soc_curve_nmc_num_elements - 1].voltage;
}

float calc_SoC (const float cellVoltage)
{
    uint_fast8_t ndx; 

    // exceeds minimum voltage?
    if (cellVoltage <= soc_curve_nmc[0].voltage)
        return soc_curve_nmc[0].percent;

    // exceeds maximum voltage?
    if (cellVoltage >= soc_curve_nmc[soc_curve_nmc_num_elements - 1].voltage)
        return soc_curve_nmc[soc_curve_nmc_num_elements - 1].percent;

    // find the closest index (this could be done with better performance using binary search, but the array is small, so whatever)
    for (ndx = 1; ndx < soc_curve_nmc_num_elements; ndx++)
        if (cellVoltage < soc_curve_nmc[ndx].voltage)
            break;

    const SoC_LUT_Entry * const hiElement = soc_curve_nmc + ndx;
    const SoC_LUT_Entry * const loElement = hiElement - 1;

    // use linear interpolation: f(x) = m*x + b = (y2-y1)/(x2-x1) * (x - x1) + y1
    // note: in this case, we want x to be the voltage and y to be the SoC
    float gain = (hiElement->percent - loElement->percent) / (hiElement->voltage - loElement->voltage);
    float newX = cellVoltage - loElement->voltage;

    return fma(newX, gain, loElement->percent);
}

static void rxCallback (uint8_t * mac, uint8_t * data, uint8_t len)
{
    const uint32_t sys_time = millis();
    dprintf("\nReceived ESP-NOW from %02X:%02X:%02X:%02X:%02X:%02X with length %u", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], len);

    if (len < sizeof(uint32_t)) // magic pattern
        return;

    uint32_t magic_pattern;
    memcpy(&magic_pattern, data, sizeof(magic_pattern));    // use memcpy since the content may not be aligned 
    switch (magic_pattern)
    {
        case MAGIC_PATTERN_RAINSENSOR_PACKAGE:
        {
            if ((len - sizeof(magic_pattern)) == sizeof(espnow_rainsensor_package_t))
            {
                auto package = reinterpret_cast<const espnow_rainsensor_package_t * const>(data + sizeof(magic_pattern));
                _last_rx_device_rain_sensor = sys_time;
                if (package->battery_millivolt != 0xFFFF)
                {
                    _device_rain_battery = calc_SoC(static_cast<float>(package->battery_millivolt) / 1000.0f);
                    dprintf("\nBattery Voltage: %u mV -> %u %%", package->battery_millivolt, static_cast<uint8_t>(_device_rain_battery));
                }

                if (package->sensor_rain_ok >= boolean_signal_t::error)
                    _last_rx_valid_rain_state = 0uL;
                else if (package->sensor_rain_ok == boolean_signal_t::on_or_true)
                {
                    _last_rx_valid_rain_state = sys_time;
                    _is_raining = (package->is_raining == boolean_signal_t::on_or_true);
                }

                if (package->sensor_temp_ok >= boolean_signal_t::error)
                    _last_rx_valid_temperature = 0uL;
                else if (package->sensor_temp_ok == boolean_signal_t::on_or_true)
                {
                    _last_rx_valid_temperature = sys_time;
                    _temperature = to_float(package->temperature);
                }

                if (package->sensor_humid_ok >= boolean_signal_t::error)
                    _last_rx_valid_humidity = 0uL;
                else if (package->sensor_humid_ok == boolean_signal_t::on_or_true)
                {
                    _last_rx_valid_humidity = sys_time;
                    _humidity = to_float(package->humidity);
                }
            }
            else
            {
                dprintf("\nSize mismatch for rain sensor package: expected %lu received %lu bytes", sizeof(espnow_rainsensor_package_t), (len - sizeof(magic_pattern)))
            }
        }
        break;

        default:
        {
            dprintf("\nUnknown magic pattern: %08X", magic_pattern);
        }
        break;
    }
}

bool esp_now_comm_class::begin(const uint8_t txRetries)
{
    bool success = esp_now_init() == 0;

    if (success)
    {
        // esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
        esp_now_register_recv_cb(&rxCallback);
        dprintf("ESP-NOW init ok");
    }
    else
    {
        dprintf("ESP-NOW init failed!");
    }

    return success;
}

float esp_now_comm_class::is_raining (void) {return _is_raining;}
float esp_now_comm_class::temperature (void) {return _temperature;}
float esp_now_comm_class::humidity (void) {return _humidity;}
float esp_now_comm_class::device_rain_battery (void) {return _device_rain_battery;}

bool esp_now_comm_class::is_device_rain_connected(void)
{
    return ((millis() - _last_rx_device_rain_sensor) < TIMEOUT_CONNECTED) && _last_rx_device_rain_sensor;
}

bool esp_now_comm_class::is_rain_state_valid(void)
{
    return ((millis() - _last_rx_valid_rain_state) < TIMEOUT_CONNECTED) && _last_rx_valid_rain_state;
}

bool esp_now_comm_class::is_temperature_valid(void)
{
    return ((millis() - _last_rx_valid_temperature) < TIMEOUT_CONNECTED) && _last_rx_valid_temperature;
}

bool esp_now_comm_class::is_humidity_valid(void)
{
    return ((millis() - _last_rx_valid_humidity) < TIMEOUT_CONNECTED) && _last_rx_valid_humidity;
}
