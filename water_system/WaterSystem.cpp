#include <Arduino.h>

#include "ws_defs.h"
#include "WaterSystem.h"
#include "WSRuntime.h"


LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void WaterSystem::initGlyphs(LiquidCrystal_PCF8574 &lcd)
{
    int i=0;

    _plant = new LCDGlyph(lcd, i++, plant);
    _rain_plant = new LCDGlyph(lcd, i++, rain_plant);
    _right_arrow = new LCDGlyph(lcd, i++, right_arrow);
    _level_up = new LCDGlyph(lcd, i++, level_up);
    _level_mid = new LCDGlyph(lcd, i++, level_mid);
    _level_low = new LCDGlyph(lcd, i++, level_low);
    _burger_menu = new LCDGlyph(lcd, i++, burger_menu);
    _skull = new LCDGlyph(lcd, i++, skull);

}

WaterSystem::WaterSystem(/* args */)
{
    _some_module_selected = false;

    DEBUG("Init LCD...");

    while (! Serial) {};

    DEBUG("Dose: check for LCD");

    // See http://playground.arduino.cc/Main/I2cScanner
    Wire.begin();
    Wire.beginTransmission(LCD_I2C_ADDRESS);
    int error = Wire.endTransmission();
    DEBUG("Error: %d: LCD %s" "found.", error, 0 == error ? "" : "not ");

    if (error != 0) {
        system_panic_no_return();

    } else {
        initGlyphs(lcd);
        lcd.begin(16, 2); // initialize the lcd
        lcd.setBacklight(255);
        lcd.home(); lcd.clear();
        lcd.print("Water system 0.1");
    }
}

inline void WaterSystem::activateSelection()
{
    _selected_module = saneModuleIndex();
    _some_module_selected = true;
}

inline byte WaterSystem::saneModuleIndex()
{
    return _selected_module % MAX_MODULE_COUNT;
}

inline void WaterSystem::deactivateSelection()
{
    _some_module_selected = false;
}

bool WaterSystem::hasActiveModule(byte *pModuleIdx)
{
    *pModuleIdx = saneModuleIndex();
    return _some_module_selected;
}

bool WaterSystem::selectNextModule()
{
    noInterrupts();
    _selected_module++;
    _selected_module = saneModuleIndex();
    interrupts();
    return _some_module_selected;
}


ulong timedelta(ulong ref_timestamp, ulong now)
{
  if (now >= ref_timestamp)
    return now - ref_timestamp;
  else // overflow
    return ULONG_MAX - ref_timestamp + now;
}
