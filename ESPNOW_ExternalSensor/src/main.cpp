#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "ota/ota.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"
#include "espnow/EspNowComm.hpp"
#include "sensors/RainSensorClass/RainSensorClass.hpp"
#include "sensors/dht_temp_humid/dht_temp_humid.hpp"

/*
 there are two modes for this controller:

 1. Sensor Mode:
    Device wakes, reads the sensor(s), transmits the data via ESP-NOW and goes to deep-sleep to conserve battery

 2. Update Mode:
    Device wakes, connects to a WiFi and cyclically handles OTA until either finished or nothing happened for 5 minutes, then reboots
*/
void setup() {
  rain_sensor.begin();
  temp_humidity_sensor.begin();

  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  #else
  Serial.begin(115200);
  #endif

  dprintf("\nHello world");

  static struct __attribute__((packed))
  {
    uint32_t magic_pattern;
    espnow_rainsensor_package_t package;
  } espnow_payload; // static so the memory stays valid (for re-transmission; esp_now_comm only stores the address)
  memset(&espnow_payload, 0xFF, sizeof(espnow_payload));
  espnow_payload.magic_pattern = MAGIC_PATTERN_RAINSENSOR_PACKAGE;
  
  // from DHT22 sensor
  // timing can be optimized by intentionally waiting a bit before reading
  temp_humidity_sensor.delay_until_ready(); // if this is removed it takes more than 2 seconds until values are available
  temp_humidity_sensor.read(true);

  float temperature = -999;
  // try to read the sensor (this may take a while so do not enable WiFi yet)
  for (const uint32_t timeout_at = millis() + 10000uL; millis() < timeout_at;)
  {
    temperature = temp_humidity_sensor.readTemperature();
    if (!isnan(temperature))
      break;

    delay(20);
  }

  espnow_payload.package.temperature = to_analog_signal(temperature);
  if (isnan(temperature))
    espnow_payload.package.sensor_temp_ok = boolean_signal_t::off_or_false;
  else
    espnow_payload.package.sensor_temp_ok = boolean_signal_t::on_or_true;

  float humidity = -999;
  for (const uint32_t timeout_at = millis() + 3000uL; millis() < timeout_at;)
  {
    humidity = temp_humidity_sensor.readHumidity();
    if (!isnan(humidity))
      break;

    delay(20);
  }
  
  espnow_payload.package.humidity = to_analog_signal(humidity);
  if (isnan(humidity))
    espnow_payload.package.sensor_humid_ok = boolean_signal_t::off_or_false;
  else
    espnow_payload.package.sensor_humid_ok = boolean_signal_t::on_or_true;
  
  dprintf("Temp = %d, Humid = %d", (isnan(temperature) ? -99999 : static_cast<int>(temperature)), (isnan(humidity) ? -99999 : static_cast<int>(humidity)));

  // from rain sensor
  espnow_payload.package.is_raining     = static_cast<boolean_signal_t>(rain_sensor.is_raining());
  espnow_payload.package.sensor_rain_ok = boolean_signal_t::on_or_true;

  // from the one and only ADC input (do not use the `ESP.getVcc()` function since this device may be powered by the solar panel if the battery is full or disconnected)
  espnow_payload.package.battery_millivolt = static_cast<uint16_t>((analogRead(A0) * 4500) / 1024);

  // WiFi must be turned on to use ESP NOW, but a connection is not required
  WiFi.mode(WIFI_STA);
  esp_now_comm.begin();

  // broadcast
  mac destination = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  auto send_start = micros();
  esp_now_comm.send(destination, &espnow_payload, sizeof(espnow_payload));

  
  // for (const uint32_t timeout_at = millis() + 200uL; (millis() < timeout_at) && !esp_now_comm.tx_success();)
  //   yield();
  delay(1); //  for broadcasts, the tx callback is not called (no `tx_success()`) so wait a fixed duration instead (test with unicast measured ~823 µs, sporadic ~1544 µs)

  dprintf("\t\tData send after %lu us. Send duration: %lu us", micros(), micros() - send_start);
  ESP.deepSleep(30uLL * 1000000uLL, RF_DEFAULT);
}

void loop() {
}