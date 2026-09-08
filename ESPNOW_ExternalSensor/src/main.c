#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_system.h"

#include "sensors/dht22/dht22.h"
#include "sensors/battery/battery.h"

#include "../../espnow_types.hpp"

#define SLEEP_DURATION_NORMAL       (10uLL * 1000000uLL)            // 10 sec
#define SLEEP_DURATION_BAT_CRITICAL (60uLL * 60uLL * 1000000uLL)    // 1 hr

static struct __attribute__((packed))
{
    uint32_t magic_pattern;
    espnow_rainsensor_package_t package;
} espnow_payload;

void app_main(void)
{
    const int32_t bat_mV = battery_read_mV();
    if (bat_mV >= 0)
    {
        // valid reading
        espnow_payload.package.battery_millivolt = (uint16_t) ((bat_mV >= UINT16_MAX) ? (UINT16_MAX - 1) : bat_mV); // all bits set = N/A, so limit to 0xFFFE
    }

    const bool battery_critical = (bat_mV <= 2700);
    if (!battery_critical)
    {
        // TODO power the sensors (does this need a transistor?)

        float temperature_c = 0.0f;
        float humidity_pct = 0.0f;

        memset(&espnow_payload, 0xFF, sizeof(espnow_payload));
        espnow_payload.magic_pattern = MAGIC_PATTERN_RAINSENSOR_PACKAGE;

        if (dht22_read(&temperature_c, &humidity_pct)) {

            // temperature_c: degrees Celsius
            espnow_payload.package.sensor_temp_ok   = on_or_true;
            espnow_payload.package.temperature      = to_analog_signal(temperature_c);

            // humidity_pct: relative humidity in percent
            espnow_payload.package.sensor_humid_ok  = on_or_true;
            espnow_payload.package.humidity         = to_analog_signal(humidity_pct);
        }
        else
        {
            espnow_payload.package.sensor_temp_ok  = off_or_false;
            espnow_payload.package.sensor_humid_ok = off_or_false;
        }

        // TODO rain sensor
        // read analog value (espnow package typedef needs to be adjusted)
        // read digital value
    }
    
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());

    uint8_t broadcast_addr[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t payload = 0xFE;
    esp_now_send(broadcast_addr, &payload, sizeof(payload));

    esp_sleep_enable_timer_wakeup(battery_critical ? SLEEP_DURATION_BAT_CRITICAL : SLEEP_DURATION_NORMAL);
    esp_deep_sleep_start();
}