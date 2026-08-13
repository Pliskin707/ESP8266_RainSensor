#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "ota/ota.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"

using namespace pliskin;

static bool mDNS_init_ok = false;
static bool wifi_connected = false;
WiFiClient client;
WiFiManager wm;

const char * get_uid (void)
{
  static char uid[21] = {};
  if (!uid[0])
    snprintf_P(uid, sizeof(uid), PSTR("RainSensor_%08X"), ESP.getChipId());

  return uid;
}

void setup() {
  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  #else
  Serial.begin(115200);
  #endif

  // WiFi Manager - Async mode
  WiFi.hostname(DEVICENAME);
  wm.setConfigPortalBlocking(false);  // Non-blocking mode
  wm.setConnectTimeout(20);           // Connection timeout in seconds
  wm.setConnectRetries(3);            // Number of retries
  
  // Setup WiFi event callbacks
  WiFi.onEvent([](WiFiEvent_t event) {
    if (event == WIFI_EVENT_STAMODE_GOT_IP) {
      wifi_connected = true;
      dprintf("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
      
      // Initialize mDNS only after WiFi is connected
      if (!mDNS_init_ok) {
        mDNS_init_ok = MDNS.begin(DEVICENAME);
        if (mDNS_init_ok) {
          dprintf("mDNS initialized\n");
        }
      }
      
      // Initialize OTA only after WiFi is connected
      ota::begin(DEVICENAME);
    } 
    else if (event == WIFI_EVENT_STAMODE_DISCONNECTED) {
      wifi_connected = false;
      dprintf("WiFi disconnected\n");
    }
  });
  
  wifi_set_sleep_type(NONE_SLEEP_T); // maybe remove/adjust this later when run on battery

  dprintf("\n\nUID: %s\n\n", get_uid());
  
  // Start WiFiManager (will start config portal if not connected)
  wm.autoConnect(DEVICENAME);
}

void loop() {
  const uint32_t time = millis();
  static uint32_t next = 0;

  // WiFiManager async processing
  wm.process();

  // Wifi status
  const bool connected = WiFi.isConnected();
  #ifndef DEBUG_PRINT
  digitalWrite(LEDPIN, !connected);
  #endif

  // mDNS
  if (mDNS_init_ok)
    MDNS.update();

  // OTA
  if (connected)
    ota::handle();

  // program logic
  if (time >= next)
  {
    next = time + 10000;
    dprintf("Systime: %lu ms; WLAN: %sconnected (as %s)\n", time, (connected ? "":"dis"), connected ? WiFi.localIP().toString().c_str() : "N/A");

    if (connected)
    {
        // TODO
    }
  }

  yield();
}