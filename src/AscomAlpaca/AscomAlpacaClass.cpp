#include "AscomAlpacaClass.hpp"
#include "../projutils/projutils.hpp"

static JsonDocument _alpaca_doc;
static ESP8266WebServer _server;

void ascom_alpaca::_handle_discovery(void)
{
    int packetSize = _udp.parsePacket();
    if (packetSize > 0) 
    {
        char packetBuffer[256];
        int len = _udp.readBytesUntil(0, packetBuffer, sizeof(packetBuffer) - 1);

        if (len > 0) 
        {
            packetBuffer[len] = '\0';
            String payload = String(packetBuffer);
            if(payload.equalsIgnoreCase(F("alpacadiscovery1")))
            {
            const auto ip = _udp.remoteIP();
            const auto port = _udp.remotePort();

            dprintf("\nDiscovery request received from %s:%u", ip.toString().c_str(), port);

            // respond
            _alpaca_doc.clear();
            _alpaca_doc["AlpacaPort"] = _port_device;

            _udp.beginPacket(ip, port);
            serializeJson(_alpaca_doc, _udp);
            auto success = _udp.endPacket(); // BUG this doesn't seem to send anything despite returning success
            if (!success)
            {
                dprintf("\nSend failed with %d", _udp.getWriteError());
                _udp.clearWriteError();
            }
            else
            {
                dprintf("\nSend success");
            }
            }
        }
    }
}

const char *ascom_alpaca::get_uid(void)
{
  static char uid[21] = {};
  if (!uid[0])
    snprintf_P(uid, sizeof(uid), PSTR("RainSensor_%08X"), ESP.getChipId());

  return uid;
}

static void dummy()
{
    dprintf("dummy called");
    _server.send(500, "text/plain", "not implemented");
}

static void send_api_versions()
{
    dprintf("\napiversions called");

    _alpaca_doc.clear();
    auto versions = _alpaca_doc["Value"].to<JsonArray>();
    versions.add(1);

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_api_description()
{
    dprintf("\napi description called");

    _alpaca_doc.clear();
    auto value = _alpaca_doc["Value"].to<JsonObject>();
    value["ServerName"] = ascom_alpaca::get_uid();
    value["Manufacturer"] = "Pliskin707";
    value["ManufacturerVersion"] = "1.0.0.0";
    value["Location"] = "nearby ;)";

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_configured_devices()
{
    dprintf("\ncfg devices called");

    _alpaca_doc.clear();
    auto value = _alpaca_doc["Value"].to<JsonArray>();
    auto item = value.add<JsonObject>();
    item["DeviceName"] = "RainSensor";
    item["DeviceType"] = "ObservingConditions";
    item["DeviceNumber"] = 0;
    auto unique_id = String(ascom_alpaca::get_uid());
    unique_id += "_0";
    item["UniqueID"] = unique_id;

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

void ascom_alpaca::begin(const uint16_t port_discovery, const uint16_t port_device)
{
    _port_discovery = port_discovery;
    _port_device = port_device;

    _server.on("/management/apiversions", send_api_versions);
    _server.on("/management/v1/description", send_api_description);
    _server.on("/management/v1/configureddevices", send_configured_devices);
    _server.on("/api/v1", dummy);
}

void ascom_alpaca::loop(const bool connected)
{
    if (connected)
    {
        if (!_was_connected)
        {
            _udp.begin(_port_discovery);
            _server.begin(_port_device);
        }

        _handle_discovery();
        _server.handleClient();
    }
    else if (_was_connected)
    {
        _server.close();
    }

    _was_connected = connected;
}
