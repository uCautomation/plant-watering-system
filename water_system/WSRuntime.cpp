#include <Arduino.h>

#include "WaterSystem.h"

int g_panic_led = 0;

void panicLEDToggle() {
    g_panic_led = !g_panic_led;
    digitalWrite(LED_BUILTIN, g_panic_led);
}

void system_panic_no_return()
{
    DEBUG("PANIC!!!!");
    lcd.setBacklight(0);
    while(true) {
        panicLEDToggle();
        delay(200);
    }
}
