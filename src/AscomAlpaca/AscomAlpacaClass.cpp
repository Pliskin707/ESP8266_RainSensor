#include "AscomAlpacaClass.hpp"
#include "../projutils/projutils.hpp"

static ESP8266WebServer _server;
static float _average_period_hours = 0.0;   // `0.0` must always be accepted (instantaneous value)
static float _rain_rate = 0.1;
static bool _is_safe = true;

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

static JsonDocument _prepare_json_response (void)
{
    JsonDocument doc;

    doc["ClientTransactionID"] = _get_client_transaction_id();
    doc["ServerTransactionID"] = _get_server_transaction_id();
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";

    return doc;
}

static void _set_response_error (JsonDocument& doc, const int error_number, const char * const description = nullptr)
{
    doc["ErrorNumber"] = error_number;

    if (error_number)
    {
        if (description)
            doc["ErrorMessage"] = description;
        else
            doc["ErrorMessage"] = "UnknownError";
    }
    else
        doc["ErrorMessage"] = "";
}

static void _send_json_response (JsonDocument& doc)
{
    _server.sendHeader("Content-Type", "application/json");
    String serialized;
    serializeJson(doc, serialized);
    _server.send(200, "application/json", serialized);
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
            JsonDocument doc;
            doc["AlpacaPort"] = _port_device;

            _udp.beginPacket(ip, port);
            serializeJson(doc, _udp);
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
    JsonDocument doc;

    auto versions = doc["Value"].to<JsonArray>();
    versions.add(1);

    doc["ClientTransactionID"] = _get_client_transaction_id();
    doc["ServerTransactionID"] = _get_server_transaction_id();

    _send_json_response(doc);
}

static void send_api_description()
{
    JsonDocument doc;

    auto value = doc["Value"].to<JsonObject>();
    value["ServerName"] = ascom_alpaca::get_uid();
    value["Manufacturer"] = "Pliskin707";
    value["ManufacturerVersion"] = "1.0.0.0";
    value["Location"] = "nearby ;)";

    doc["ClientTransactionID"] = _get_client_transaction_id();
    doc["ServerTransactionID"] = _get_server_transaction_id();

    _send_json_response(doc);
}

static void send_configured_devices()
{
    JsonDocument doc;

    auto value = doc["Value"].to<JsonArray>();

    // Observing Conditions
    auto item = value.add<JsonObject>();
    item["DeviceName"] = "RainSensor";
    item["DeviceType"] = "ObservingConditions";
    item["DeviceNumber"] = 0;
    auto unique_id = String(ascom_alpaca::get_uid());
    unique_id += "_Conditions";
    item["UniqueID"] = unique_id;

    // Safety Monitor
    item = value.add<JsonObject>();
    item["DeviceName"] = "RainMonitor";
    item["DeviceType"] = "SafetyMonitor";
    item["DeviceNumber"] = 0;
    unique_id = String(ascom_alpaca::get_uid());
    unique_id += "_Monitor";
    item["UniqueID"] = unique_id;

    doc["ClientTransactionID"] = _get_client_transaction_id();
    doc["ServerTransactionID"] = _get_server_transaction_id();

    _send_json_response(doc);
}

static void send_connected_state()
{
    auto doc = _prepare_json_response();

    // TODO this has parameters; parse and use them!
    // TODO this can be PUT or GET; handle them
    doc["Value"] = true;

    _send_json_response(doc);
}

static void send_rain_rate()
{
    dprintf("\n rain rate called");

    // TODO this has parameters; parse and use them!
    auto doc = _prepare_json_response();
    doc["Value"] = _rain_rate;

    _send_json_response(doc);
}

static void send_device_description()
{
    auto doc = _prepare_json_response();
    doc["Value"] = "ESP8266 based device using a simple digital rain sensor";

    _send_json_response(doc);
}

static void send_driver_info()
{
    auto doc = _prepare_json_response();
    doc["Value"] = "https://github.com/Pliskin707/ESP8266_RainSensor";

    _send_json_response(doc);
}

static void send_driver_version()
{
    auto doc = _prepare_json_response();
    doc["Value"] = "1.0"; // see https://ascom-standards.org/newdocs/camera.html#Camera.DriverVersion

    _send_json_response(doc);
}

static void send_interface_version()
{
    auto doc = _prepare_json_response();
    doc["Value"] = 4; // see https://ascom-standards.org/newdocs/camera.html#Camera.InterfaceVersion

    _send_json_response(doc);
}

static void send_supported_actions()
{
    auto doc = _prepare_json_response();
    auto value = doc["Value"].to<JsonArray>(); // a list of all non-standard actions the device can perform
    value.add("GetBatteryLevel"); // TODO implement (see https://ascom-standards.org/newdocs/camera.html#Camera.Action )

    _send_json_response(doc);
}

static void send_average_period()
{
    auto doc = _prepare_json_response();
    doc["Value"] = _average_period_hours;

    _send_json_response(doc);
}

static void change_average_period()
{
    if (!_server.hasArg("AveragePeriod"))
    {
        _server.send(400, "Missing parameter \"AveragePeriod\"");
        return;
    }

    auto doc = _prepare_json_response();

    int average_period = _server.arg("AveragePeriod").toFloat();
    if ((average_period < 0.0) || (average_period > 24.0))
    {
        _set_response_error(doc, 0x401, "Only values between 0..24 are allowed");
    }
    else
    {
        dprintf("Average period change to %.3f hours", average_period);
        _average_period_hours = average_period;
    }

    _send_json_response(doc);
}

static void send_device_state_conditions()
{
    auto doc = _prepare_json_response();
    auto value = doc["Value"].to<JsonArray>();
    auto item = value.add<JsonObject>();
    item["Name"] = "RainRate";
    item["Value"] = _rain_rate;

    _send_json_response(doc);
}

static void send_device_state_monitor()
{
    auto doc = _prepare_json_response();
    auto value = doc["Value"].to<JsonArray>();
    auto item = value.add<JsonObject>();
    item["Name"] = "IsSafe";
    item["Value"] = _is_safe;

    _send_json_response(doc);
}

static void send_not_implemented()
{
    auto doc = _prepare_json_response();

    char description[200];
    const auto& uri = _server.uri();
    const auto& property_name = uri.substring(uri.lastIndexOf('/') + 1);
    snprintf_P(description, sizeof(description), PSTR("%s is not implemented"), property_name.c_str());
    description[sizeof(description) - 1] = 0;
    _set_response_error(doc, 0x400, description);

    _send_json_response(doc);
}

void ascom_alpaca::begin(const uint16_t port_discovery, const uint16_t port_device)
{
    _port_discovery = port_discovery;
    _port_device = port_device;

    _server.on("/management/apiversions", send_api_versions);
    _server.on("/management/v1/description", send_api_description);
    _server.on("/management/v1/configureddevices", send_configured_devices);

    _server.on("/setup/v1/observingconditions/0/setup", send_not_implemented);    // <- this is called if you press the gears symbol in N.I.N.A.
    _server.on("/api/v1/observingconditions/0/connected", send_connected_state);
    _server.on("/api/v1/observingconditions/0/interfaceversion", send_interface_version);
    _server.on("/api/v1/observingconditions/0/description", send_device_description);
    _server.on("/api/v1/observingconditions/0/driverinfo", send_driver_info);
    _server.on("/api/v1/observingconditions/0/driverversion", send_driver_version);
    _server.on("/api/v1/observingconditions/0/supportedactions", send_supported_actions);
    _server.on("/api/v1/observingconditions/0/devicestate", send_device_state_conditions);

    _server.on("/api/v1/observingconditions/0/rainrate", send_rain_rate);
    _server.on("/api/v1/observingconditions/0/averageperiod", HTTP_GET, send_average_period);
    _server.on("/api/v1/observingconditions/0/averageperiod", HTTP_PUT, change_average_period);
    _server.on("/api/v1/observingconditions/0/cloudcover", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/dewpoint", send_not_implemented); // maybe later with a DHT11 or DHT22 (also requires `send_device_state_conditions()` to be updated)
    _server.on("/api/v1/observingconditions/0/humidity", send_not_implemented); // maybe later with a DHT11 or DHT22 (also requires `send_device_state_conditions()` to be updated)
    _server.on("/api/v1/observingconditions/0/pressure", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/skybrightness", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/skyquality", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/skytemperature", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/starfwhm", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/temperature", send_not_implemented); // maybe later with a DHT11 or DHT22 (also requires `send_device_state_conditions()` to be updated)
    _server.on("/api/v1/observingconditions/0/winddirection", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/windgust", send_not_implemented);
    _server.on("/api/v1/observingconditions/0/windspeed", send_not_implemented);

    _server.on("/setup/v1/safetymonitor/0/setup", send_not_implemented);    // <- this is called if you press the gears symbol in N.I.N.A.
    _server.on("/api/v1/safetymonitor/0/connected", send_connected_state);
    _server.on("/api/v1/safetymonitor/0/interfaceversion", send_interface_version);
    _server.on("/api/v1/safetymonitor/0/description", send_device_description);
    _server.on("/api/v1/safetymonitor/0/driverinfo", send_driver_info);
    _server.on("/api/v1/safetymonitor/0/driverversion", send_driver_version);
    _server.on("/api/v1/safetymonitor/0/supportedactions", send_supported_actions);
    _server.on("/api/v1/safetymonitor/0/devicestate", send_device_state_monitor);
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
