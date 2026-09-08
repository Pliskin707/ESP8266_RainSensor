#ifndef __LED_STRIPE_CLASS_HPP__
#define __LED_STRIPE_CLASS_HPP__

#include "NeoPixelBusLg.h"

class led_stripe_class
{
    private:
        NeoPixelBusLg<NeoGrbFeature, NeoEsp8266DmaWs2812xMethod> _stripe{5};
        uint32_t _last_update = 0uL;

    public:
        void begin (void);
        void loop (void);
};

extern led_stripe_class led_stripe;

#endif