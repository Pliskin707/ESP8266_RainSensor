#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#define LEDPIN                          LED_BUILTIN
#define DEVICENAME                      "RainSensor"
#define PORT_ALPACA_DISCOVERY           (32227)
#define PORT_ALPACA_DEVICE              (5017)

#define PIN_WAKE                        (D0)    // aka. GPIO16    // connect this via a 10k resistor to RST for deep-sleep
#define PIN_SCL                         (D1)    // aka. GPIO5
#define PIN_SDA                         (D2)    // aka. GPIO4
#define PIN_FLASH                       (D3)    // aka. GPIO0   // *must* be high at boot -> do not use
#define PIN_RESERVED                    (D4)    // aka. GPIO2   // *must* be high at boot -> do not use
#define PIN_RAIN_SENSOR                 (D5)    // aka. GPIO14  // YL-38 with a comparator // ! these come in different footprints
#define PIN_HUMID_TEMP_SENSOR_DATA      (D6)    // aka. GPIO12  // DHT11 or DHT22
#define PIN_HUMID_TEMP_SENSOR_SUPPLY    (D7)    // aka. GPIO13

#endif