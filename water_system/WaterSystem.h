#ifndef WATERSYSTEM_H
#define WATERSYSTEM_H

#include <limits.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>


#define DEBUG_ON
#include "DebugWS.h"

#include "LcdGlyphs.h"
#include "ws_types.h"


#define LCD_I2C_ADDRESS 0x27
extern LiquidCrystal_PCF8574 lcd;  // set the LCD address to 0x27 for a 16 chars and 2 line display


class WaterSystem
{
private:
    /* data */
    bool _some_module_selected;
    byte _selected_module = 0;

    LCDGlyph *_plant;
    LCDGlyph *_rain_plant;
    LCDGlyph *_right_arrow;
    LCDGlyph *_level_up;
    LCDGlyph *_level_mid;
    LCDGlyph *_level_low;
    LCDGlyph *_burger_menu;
    LCDGlyph *_skull;

    void initGlyphs(LiquidCrystal_PCF8574 &lcd);

    byte saneModuleIndex();
public:
    WaterSystem();
    void activateSelection();
    void deactivateSelection();

    bool selectNextModule();
    bool hasActiveModule(byte *pModuleIdx);
};


ulong timedelta(ulong ref_timestamp, ulong now);


#endif // WATERSYSTEM_H
