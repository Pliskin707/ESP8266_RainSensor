#include "AscomAlpacaClass.hpp"
#include "../projutils/projutils.hpp"

static JsonDocument _alpaca_doc;

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

void ascom_alpaca::_handle_new_clients(void)
{
    auto client = _listener.accept();
    if (!client)
        return;

    _clients.push_back(client);
}

const char *ascom_alpaca::get_uid(void)
{
  static char uid[21] = {};
  if (!uid[0])
    snprintf_P(uid, sizeof(uid), PSTR("RainSensor_%08X"), ESP.getChipId());

  return uid;
}

void ascom_alpaca::begin(const uint16_t port_discovery, const uint16_t port_device)
{
    _udp.begin(port_discovery);
    _port_device = port_device;
    _listener.begin(port_device, 3);
}

void ascom_alpaca::loop(const bool connected)
{
    if (connected)
    {
        _handle_discovery();
    }

    if (_was_connected && !connected)
    {
        _listener.close();
    }

    _was_connected = connected;
}
