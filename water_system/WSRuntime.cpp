#include <Arduino.h>

#include "WaterSystem.h"

byte g_panic_led = 0;

void panicLEDToggle() {
    g_panic_led = !g_panic_led;
    digitalWrite(LED_BUILTIN, g_panic_led);
}

void system_panic_no_return()
{
    DEBUG_P("PANIC!!!!");

    lcd.setBacklight(127);
    while(true) {
        panicLEDToggle();
        delay(200);
    }
}

void assert_or_panic(bool condition)
{
    if (!condition) {
        DEBUG_P("InternalErr: Assertion failure!");
        system_panic_no_return();
    }
}
