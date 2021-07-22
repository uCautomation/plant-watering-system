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
#include "WSMenu.h"


#define LCD_I2C_ADDRESS 0x27
extern LiquidCrystal_PCF8574 lcd;  // set the LCD address to 0x27 for a 16 chars and 2 line display

extern WSMenu ctrl_one_menu;
extern WSMenu list_all_menu;
extern WSMenu list_one_menu;

const byte lcdLineBufLen = 18; //TODo: there seems to be a buffer overrun

struct saneModuleIndex_t {
    byte moduleIndex;
};
class WaterSystem
{
    private:
        /* data */
        bool _internal_error = false;

        bool _some_module_selected;
        saneModuleIndex_t _selected_module = { 0 };

        LCDGlyph *_plant;
        LCDGlyph *_rain_plant;
        LCDGlyph *_right_arrow;
        LCDGlyph *_level_up;
        LCDGlyph *_level_mid;
        LCDGlyph *_level_low;
        LCDGlyph *_burger_menu;
        LCDGlyph *_skull;

        void initGlyphs(LiquidCrystal_PCF8574 &lcd);

        WSMenu *_p_current_menu;
        char _lcd_line0[lcdLineBufLen] = {0};
        char _lcd_line1[lcdLineBufLen] = {0};

        void _continueLine0AndWriteLine1();

        void _resetMenu();
        inline saneModuleIndex_t _saneModuleIndex(byte moduleIndex);
        // void listCtrlOne(saneModuleIndex_t currentModule);

        // Sensor+Pump modules
        SensorAndPump sp[MAX_MODULE_COUNT] = {
            {4, A0, 5}, // D4 is Vsens, A0 = Sens, D5 is Valve cmd
            {6, A1, 7},
            {8, A2, 9},
            {10, A3, 11},
        };

        bool _confirmIndexIsSane(byte moduleIndex, saneModuleIndex_t *pSaneIndex);
        bool _clearLcdAndListCurrentPlant(byte &selectedIdx);

        void _clearScreenNoCursor();
        void _clearScreenNoCursorNoBacklight();

        void _lcdWritePlantIconOrX(byte moduleIndex);

    public:

        WaterSystem();

        void setSystemInternalError();
        bool hasInternalError();

        void saveReferenceValuesToEEPROM();
        bool loadReferenceValuesFromEEPROM();

        // byte selectSaneModuleIndex(byte moduleIndex);
        void selectModuleIndex(saneModuleIndex_t saneIndex);
        void deactivateSelection();
        bool hasActiveModule(saneModuleIndex_t *pModuleIdx);

        void listAll();
        bool statusOne(byte moduleIndex);
        void showStatusCurrentOne();
        bool showCtrlOne(byte moduleIndex);
        void showCtrlCurrentOne();

        void manualWaterCurrent();

        // void showScreen();

        void showMenuCursor();
        void openMenu(WSMenu *pMenu);
        void selectNextMenuEntry();

        void showSysStatus();

        void showState(uint8_t stateNo);

        void toggleUsageForCurrent();
        void resetCalibrationForCurrentModule();

        void autoWater();

};

void addSleepMillis(uint16_t wdt_sleep_prescaler);
ulong allMillis(void);
ulong timedelta(ulong ref_timestamp, ulong now);

#endif // WATERSYSTEM_H
