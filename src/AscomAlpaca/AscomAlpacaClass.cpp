#include "AscomAlpacaClass.hpp"
#include "../projutils/projutils.hpp"

static JsonDocument _alpaca_doc;
static ESP8266WebServer _server;

static uint32_t _get_server_transaction_id (void)
{
    static uint32_t server_transaction_id = 0;

    // valid values are [1..UINT32_MAX]
    if(!++server_transaction_id)
        ++server_transaction_id;

    return server_transaction_id;
}

static uint32_t _get_client_transaction_id (void)
{
    int client_transaction_id = 0;

    if (_server.hasArg("ClientTransactionID"))
        client_transaction_id = _server.arg("ClientTransactionID").toInt();

    return client_transaction_id;
}

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

static void send_api_versions()
{
    _alpaca_doc.clear();

    auto versions = _alpaca_doc["Value"].to<JsonArray>();
    versions.add(1);

    _alpaca_doc["ClientTransactionID"] = _get_client_transaction_id();
    _alpaca_doc["ServerTransactionID"] = _get_server_transaction_id();

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_api_description()
{
    _alpaca_doc.clear();

    auto value = _alpaca_doc["Value"].to<JsonObject>();
    value["ServerName"] = ascom_alpaca::get_uid();
    value["Manufacturer"] = "Pliskin707";
    value["ManufacturerVersion"] = "1.0.0.0";
    value["Location"] = "nearby ;)";

    _alpaca_doc["ClientTransactionID"] = _get_client_transaction_id();
    _alpaca_doc["ServerTransactionID"] = _get_server_transaction_id();

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_configured_devices()
{
    _alpaca_doc.clear();

    auto value = _alpaca_doc["Value"].to<JsonArray>();
    auto item = value.add<JsonObject>();
    item["DeviceName"] = "RainSensor";
    item["DeviceType"] = "ObservingConditions";
    item["DeviceNumber"] = 0;
    auto unique_id = String(ascom_alpaca::get_uid());
    unique_id += "_0";
    item["UniqueID"] = unique_id;

    _alpaca_doc["ClientTransactionID"] = _get_client_transaction_id();
    _alpaca_doc["ServerTransactionID"] = _get_server_transaction_id();

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_connected_state()
{
    _alpaca_doc.clear();

    // TODO this has parameters; parse and use them!
    // TODO this can be PUT or GET; handle them
    _alpaca_doc["ClientTransactionID"] = _get_client_transaction_id();
    _alpaca_doc["ServerTransactionID"] = _get_server_transaction_id();
    _alpaca_doc["ErrorNumber"] = 0;
    _alpaca_doc["ErrorMessage"] = "";
    _alpaca_doc["Value"] = true;

    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(_alpaca_doc, serialized);
    _server.send(200, "application/json", serialized);
}

static void send_rain_rate()
{
    dprintf("\n rain rate called");

    // TODO this has parameters; parse and use them!
    _alpaca_doc.clear();

    _alpaca_doc["ClientTransactionID"] = _get_client_transaction_id();
    _alpaca_doc["ServerTransactionID"] = _get_server_transaction_id();
    _alpaca_doc["ErrorNumber"] = 0;
    _alpaca_doc["ErrorMessage"] = "";
    _alpaca_doc["Value"] = 0.1;

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
    // _server.on("/setup/v1/observingconditions/0/setup", nullptr);    // <- this is called if you press the gears symbol in N.I.N.A.
    _server.on("/api/v1/observingconditions/0/connected", send_connected_state);
    _server.on("/api/v1/observingconditions/0/rainrate", send_rain_rate);
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
