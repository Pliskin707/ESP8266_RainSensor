#include "dht22.h"

bool dht22_read(float *temperature_c, float *humidity_pct)
{
    if (temperature_c == NULL || humidity_pct == NULL) {
        return false;
    }

    uint8_t raw[5] = {0};
    int64_t start_us = 0;

    gpio_set_direction(DHT22_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT22_GPIO, 0);
    esp_rom_delay_us(1200);
    gpio_set_level(DHT22_GPIO, 1);
    gpio_set_direction(DHT22_GPIO, GPIO_MODE_INPUT);
    esp_rom_delay_us(30);

    if (gpio_get_level(DHT22_GPIO) != 0) {
        return false;
    }

    start_us = esp_timer_get_time();
    while (gpio_get_level(DHT22_GPIO) == 0) {
        if (esp_timer_get_time() - start_us > 1000) {
            return false;
        }
    }

    start_us = esp_timer_get_time();
    while (gpio_get_level(DHT22_GPIO) == 1) {
        if (esp_timer_get_time() - start_us > 1000) {
            return false;
        }
    }

    for (int bit_index = 0; bit_index < 40; ++bit_index) {
        while (gpio_get_level(DHT22_GPIO) == 0) {
            if (esp_timer_get_time() - start_us > 1000) {
                return false;
            }
        }

        int64_t bit_start_us = esp_timer_get_time();
        while (gpio_get_level(DHT22_GPIO) == 1) {
            if (esp_timer_get_time() - bit_start_us > 1000) {
                return false;
            }
        }

        uint32_t bit_duration_us = (uint32_t)(esp_timer_get_time() - bit_start_us);
        raw[bit_index / 8] <<= 1;
        if (bit_duration_us > 40) {
            raw[bit_index / 8] |= 1;
        }
    }

    uint8_t humidity = raw[0];
    uint8_t temp_int = raw[2];
    uint8_t temp_frac = raw[3];
    uint8_t checksum = raw[4];

    if (((uint8_t)(humidity + temp_int + temp_frac)) != checksum) {
        return false;
    }

    *humidity_pct = (float)humidity;
    *temperature_c = (float)((int16_t)((temp_int << 8) | temp_frac)) / 10.0f;

    return true;
}