#ifndef __BATTERY_H__
#define __BATTERY_H__

#include <stdint.h>

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define BATTERY_GPIO GPIO_NUM_0

#define BATTERY_ERR_ADC_INIT_FAILED      (-1)
#define BATTERY_ERR_ADC_MAP_FAILED       (-2)
#define BATTERY_ERR_ADC_CONFIG_FAILED    (-3)
#define BATTERY_ERR_ADC_READ_FAILED      (-4)

int32_t battery_read_mV(void);

#endif
