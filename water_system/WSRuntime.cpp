#include <Arduino.h>

#include "WaterSystem.h"

byte g_panic_led = 0U;

void panicLEDToggle() {
    g_panic_led = !g_panic_led;
    digitalWrite(LED_BUILTIN, g_panic_led);
}

void panicLEDOff() {
    g_panic_led = 0U;
    digitalWrite(LED_BUILTIN, g_panic_led);
}

void system_panic_wo_lcd_no_return()
{
    DEBUG_P("PANIC!!!!");

    while(true) {
        panicLEDToggle();
        delay(200);
    }
}

void system_panic_no_return()
{
    lcd.setBacklight(127);
    lcd.print(F(" PANIC! \7"));
    system_panic_wo_lcd_no_return();
}

void assert_or_panic(bool condition)
{
    if (!condition) {
        DEBUG_P("InternalErr: Assertion failure!");
        system_panic_no_return();
    }
}
