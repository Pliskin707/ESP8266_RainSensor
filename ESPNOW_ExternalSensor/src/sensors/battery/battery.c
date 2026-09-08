#include "battery.h"

int32_t battery_read_mV(void)
{
    static adc_oneshot_unit_handle_t adc_handle = NULL;
    static adc_channel_t adc_channel = 0;
    static bool adc_ready = false;

    if (!adc_ready) {
        adc_oneshot_unit_init_cfg_t init_cfg = {
            .unit_id = ADC_UNIT_1,
        };

        if (adc_oneshot_new_unit(&init_cfg, &adc_handle) != ESP_OK) {
            return BATTERY_ERR_ADC_INIT_FAILED;
        }

        adc_unit_t unit_id = ADC_UNIT_1;
        if (adc_oneshot_io_to_channel(BATTERY_GPIO, &unit_id, &adc_channel) != ESP_OK) {
            adc_oneshot_del_unit(adc_handle);
            adc_handle = NULL;
            return BATTERY_ERR_ADC_MAP_FAILED;
        }

        adc_oneshot_chan_cfg_t channel_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };

        if (adc_oneshot_config_channel(adc_handle, adc_channel, &channel_cfg) != ESP_OK) {
            adc_oneshot_del_unit(adc_handle);
            adc_handle = NULL;
            return BATTERY_ERR_ADC_CONFIG_FAILED;
        }

        adc_ready = true;
    }

    int raw_value = 0;
    if (adc_oneshot_read(adc_handle, adc_channel, &raw_value) != ESP_OK) {
        return BATTERY_ERR_ADC_READ_FAILED;
    }

    const uint32_t adc_max_count = 4095UL;
    const uint32_t adc_ref_mV = 3300UL;
    const uint32_t divider_factor = 2UL;

    uint32_t battery_mV = ((uint32_t)raw_value * adc_ref_mV) / adc_max_count;
    battery_mV *= divider_factor;

    return (int32_t)battery_mV;
}
