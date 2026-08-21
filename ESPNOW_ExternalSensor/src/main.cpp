#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "ota/ota.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"
#include "espnow/EspNowComm.hpp"
#include "sensors/RainSensorClass/RainSensorClass.hpp"

/*
 there are two modes for this controller:

 1. Sensor Mode:
    Device wakes, reads the sensor(s), transmits the data via ESP-NOW and goes to deep-sleep to conserve battery

 2. Update Mode:
    Device wakes, connects to a WiFi and cyclically handles OTA until either finished or nothing happened for 5 minutes, then reboots
*/
void setup() {
  // TODO remove after wiring to RST pin is done (this simulates the "deep-sleep")
  delay(2000uL);

  rain_sensor.begin();

  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  #else
  Serial.begin(115200);
  #endif

  dprintf("\nHello world");

  // WiFi must be turned on to use ESP NOW, but a connection is not required
  WiFi.mode(WIFI_STA);
  
  esp_now_comm.begin();

  static espnow_rainsensor_package_t package; // static so the memory stays valid (for re-transmission; esp_now_comm only stores the address)
  memset(&package, 0xFF, sizeof(package));
  
  // from DHT11/DHT22 sensor
  // TODO fill with real values
  package.temperature    = to_analog_signal(1.23f);
  package.humidity       = to_analog_signal(1.23f);
  package.sensor_temp_ok = package.sensor_humid_ok = boolean_signal_t::off_or_false;

  // from rain sensor
  package.is_raining     = static_cast<boolean_signal_t>(rain_sensor.is_raining());
  package.sensor_rain_ok = boolean_signal_t::on_or_true;

  // from the one and only ADC input (do not use the `ESP.getVcc()` function since this device may be powered by the solar panel if the battery is full or disconnected)
  package.battery_millivolt = static_cast<uint16_t>((analogRead(A0) * 4500) / 1024);

  // broadcast
  mac destination = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_comm.send(destination, &package, sizeof(package));

  dprintf("\t\tData send after %lu us", micros());
  
  // TODO replace with deep-sleep after wiring to RST pin is done
  delay(500uL);
  ESP.reset();
}

void loop() {
}