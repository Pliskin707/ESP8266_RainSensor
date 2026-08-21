#ifndef __OLED_DISPLAY_CLASS_HPP__
#define __OLED_DISPLAY_CLASS_HPP__

#include <U8g2lib.h>

#define FONT_SMALL              u8g2_font_resoledmedium_tr
#define FONT_SMALL_MONO         u8g2_font_profont12_mf
#define FONT_LARGE              u8g2_font_chargen_92_tf
#define FONT_LARGE_MONO         u8g2_font_chargen_92_mf

class OledDisplayClass : public U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C
{
    public:
        enum class page_t : uint8_t
        {
            startup = 0,
            rain_device_state,
            rain_monitor,
            temperature,
            humidity
        };

    private:
        bool _dimmed = false;
        uint32_t _last_update = 0uL;
        uint32_t _last_page_change = 0uL;
        page_t _page = page_t::startup;

    public:
        using U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C::U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C; // use the orignal constructors
        void begin (void);
        void dim (const bool on_off);
        void loop (void);
};

extern OledDisplayClass oled;

#endif