/**
 * @file dht22.h
 * @brief DHT22 temperature and humidity sensor interface
 *
 * This header file provides the interface for reading data from a DHT22 sensor.
 * It includes function declarations and necessary definitions for sensor operation.
 */

#ifndef __DHT22_H__
#define __DHT22_H__

#include "esp_rom_sys.h"
#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_timer.h"

/**
 * @brief GPIO pin number used for DHT22 communication
 *
 * This macro defines the default GPIO pin (GPIO_NUM_10) that will be used
 * for data exchange with the DHT22 sensor.
 */
#define DHT22_GPIO GPIO_NUM_10

/**
 * @brief Reads temperature and humidity data from the DHT22 sensor
 *
 * This function initializes communication with the DHT22 sensor, reads the raw data,
 * validates it, and converts it to meaningful temperature and humidity values.
 *
 * @param[out] temperature_c Pointer to a float variable that will store the temperature in Celsius
 * @param[out] humidity_pct Pointer to a float variable that will store the relative humidity percentage
 * @return bool Returns true if data was successfully read and validated, false otherwise
 *         Possible failure reasons include:
 *         - NULL pointers passed as arguments
 *         - Sensor not responding (no low signal after start)
 *         - Timeout during communication
 *         - Invalid checksum in received data
 */
bool dht22_read(float *temperature_c, float *humidity_pct);

#endif // __DHT22_H__
