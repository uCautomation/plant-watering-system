#ifndef WATERSYSTEM_H
#define WATERSYSTEM_H

#include <limits.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>


#define DEBUG_ON
#include "DebugWS.h"

#include "LcdGlyphs.h"
#include "SensorAndPump.h"
#include "ws_types.h"


#define LCD_I2C_ADDRESS 0x27
extern LiquidCrystal_PCF8574 lcd;  // set the LCD address to 0x27 for a 16 chars and 2 line display


class WaterSystem
{
private:
    /* data */
    bool _internal_error = false;

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

    // Sensor+Pump modules
    SensorAndPump sp[MAX_MODULE_COUNT] = {
        {4, A0, 5}, // D4 is Vsens, A0 = Sens, D5 is Valve cmd
        {6, A1, 7},
        {8, A2, 9},
        {10, A3, 11},
    };


    WaterSystem();

    void setSystemInternalError();
    bool hasInternalError();

    void activateSelection();
    void deactivateSelection();

    bool selectNextModule();
    bool hasActiveModule(byte *pModuleIdx);

    void system_list();
};


ulong timedelta(ulong ref_timestamp, ulong now);


#endif // WATERSYSTEM_H
