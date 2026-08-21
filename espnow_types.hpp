#ifndef __ESPNOW_TYPES_HPP__
#define __ESPNOW_TYPES_HPP__

#include <stdint.h>

#define MAGIC_PATTERN_RAINSENSOR_PACKAGE (0b10000010011001100010000000100111uL) // random generated

enum class boolean_signal_t : uint8_t
{
    off_or_false = 0,
    on_or_true = 1,
    error = 2,
    NA = 3
};

typedef int16_t analog_signal_t;
constexpr float to_float (const analog_signal_t& signal) {return static_cast<float>(signal) * 0.1f;}
constexpr analog_signal_t to_analog_signal (const float& value) {return ((value <= to_float(INT16_MIN)) ? INT16_MIN : ((value >= to_float(INT16_MAX)) ? INT16_MAX : static_cast<analog_signal_t>(value * 10.0f)));}

typedef struct __attribute__((packed)) espnow_rainsensor_package
{
    analog_signal_t temperature;        // factor 0.1 °C
    analog_signal_t humidity;           // factor 0.1 %
    uint16_t battery_millivolt;         // 0xFFFF = N/A
    boolean_signal_t sensor_temp_ok  :2;
    boolean_signal_t sensor_humid_ok :2;
    boolean_signal_t sensor_rain_ok  :2;
    boolean_signal_t is_raining      :2;
} espnow_rainsensor_package_t;

typedef uint8_t mac[6];

#endif