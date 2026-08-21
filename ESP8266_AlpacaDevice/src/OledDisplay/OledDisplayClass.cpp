#include "OledDisplayClass.hpp"
#include "EspNowComm/EspNowComm.hpp"
#include "OledIcons.cpp"

OledDisplayClass oled{U8G2_R0};

void OledDisplayClass::begin(void)
{
    // not sure if this is set correctly, since the constructors of the parent class are used
    _dimmed = false;
    _last_update = millis();

    // setup the OLED display
    initDisplay();
    setPowerSave(0);
    clearDisplay();
    setDrawColor(1);

    setFont(FONT_LARGE);
    setFontDirection(0);
    drawStr(0, 32, "Hello");
    sendBuffer();
    _page = page_t::startup;
}

void OledDisplayClass::dim(const bool on_off)
{
    const uint8_t v = 3;    // value from 0 to 7, higher values more brighter
    const uint8_t p1 = 3;   // p1: 1..15, higher values, more darker, however almost no difference with 3 or more
    const uint8_t p2 = 1;   // p2: 1..15, higher values, more brighter

    if (on_off)
    {
        setContrast(0);
        sendF("ca", 0x0db, v << 4);
        sendF("ca", 0x0d9, (p2 << 4) | p1 );
    }
    else
    {
        setContrast(0xFF);
        sendF("ca", 0x0db, 7 << 4);
        sendF("ca", 0x0d9, (1 << 4) | 15 );
    }
}

void OledDisplayClass::loop(void)
{
    const uint32_t sys_time = millis();

    if ((sys_time - _last_update) > 200uL)
    {
        _last_update = sys_time;

        clearBuffer();
        setDrawColor(1);

        page_t next_page = _page;
        
        if ((sys_time - _last_page_change) >= 2000uL)
        {
            _last_page_change = sys_time;
            next_page = static_cast<page_t>(static_cast<uint8_t>(_page) + 1);
            if (_page == page_t::humidity) 
                next_page = page_t::rain_device_state;   // "overflow"
        }

        const uint8_t x_offset = 40;
        switch (next_page)
        {
            case page_t::rain_monitor:
            {
                drawXBMP(0, 0, 32, 32, cloud_showers_heavy_solid_full_bitmap);
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 8);
                print(F("Rain"));

                setFont(FONT_LARGE);
                setCursor(x_offset, 24);
                print(esp_now_comm.is_raining() ? F("YES"):F("NO"));
                setFont(FONT_SMALL);
                setCursor(x_offset, 32);
                printf_P(PSTR("(%svalid)"), (esp_now_comm.is_rain_state_valid() ? "":"in"));
            }
            break;

            case page_t::humidity:
            {
                drawXBMP(0, 0, 32, 32, water_solid_full_bitmap);
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 8);
                print(F("Humidity"));

                setFont(FONT_LARGE_MONO);
                setCursor(x_offset, 24);
                printf_P(PSTR("% 2u %%"), static_cast<uint8_t>(esp_now_comm.humidity()));
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 32);
                printf_P(PSTR("(%svalid)"), (esp_now_comm.is_humidity_valid() ? "":"in"));
            }
            break;
                
            case page_t::temperature:
            {
                div_t qr = div(static_cast<int>(esp_now_comm.temperature() * 10.0f), 10);

                drawXBMP(0, 0, 32, 32, temperature_three_quarters_solid_full_bitmap);
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 8);
                print(F("Temperature"));

                setFont(FONT_LARGE_MONO);
                setCursor(x_offset, 24);
                printf_P(PSTR("% 2d.%d C"), qr.quot, qr.rem);
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 32);
                printf_P(PSTR("(%svalid)"), (esp_now_comm.is_temperature_valid() ? "":"in"));
            }
            break;

            default:
                next_page = page_t::rain_device_state;
            case page_t::rain_device_state:
            {
                drawXBMP(0, 0, 32, 32, battery_empty_solid_full_bitmap);
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 8);
                print(F("Battery"));

                setFont(FONT_LARGE_MONO);
                setCursor(x_offset, 24);
                printf_P(PSTR("% 2u %%"), static_cast<uint8_t>(esp_now_comm.device_rain_battery()));
                
                setFont(FONT_SMALL);
                setCursor(x_offset, 32);
                printf_P(PSTR("Comm %s"), (esp_now_comm.is_device_rain_connected() ? "OK":"FAIL"));
            }
            break;
        }

        sendBuffer();
        _page = next_page;
    }
}
