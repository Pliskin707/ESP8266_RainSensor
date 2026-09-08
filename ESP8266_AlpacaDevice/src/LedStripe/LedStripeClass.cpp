#include "LedStripeClass.hpp"

led_stripe_class led_stripe;

void led_stripe_class::begin(void)
{
    _stripe.Begin();
    _stripe.SetPixelColor(2, RgbColor(200, 100, 0));
}

void led_stripe_class::loop(void)
{
    const uint32_t sys_time = millis();
    if ((sys_time - _last_update) > 1000uL)
    {
        _stripe.RotateLeft(1);
        _stripe.Show();
        _last_update = sys_time;
    }
}
