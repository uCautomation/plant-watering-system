#include <Arduino.h>

#include "ws_defs.h"
#include "WaterSystem.h"
#include "WSRuntime.h"
#include "WSMenu.h"


LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//   0123456789abcdef
//  +----------------+
// 0|P1 P2 P3 P4 == X|
// 1|+2 -3 +5 -9    S|
//  +----------------+
#define LIST_ALL_MENU_STEP    3
#define LIST_ALL_MENU_ENTRIES 6
// char menuAll0[lcdLineBufLen] = { "P1 P2 P3 P4 == X" };
// char menuAll1[lcdLineBufLen] = { ".. .. .. ..    ." };
// const char menuAllFmt1[] = { "%+.1d %+.1d %+.1d %+.1d    %c" };
WSMenu list_all_menu(
    /* .MenuColumnStep = */ LIST_ALL_MENU_STEP,
    /* .NoOfMenuItems = */ LIST_ALL_MENU_ENTRIES
    );


//   0123456789abcdef
//  +----------------+
// 0|P1 Now:52 Ref:50|
// 1|WET(d:+2)  ☔ > X|
//  +----------------+
#define LIST_ONE_MENU_STEP         2
#define LIST_ONE_MENU_ENTRIES      3
#define LIST_ONE_MENU_START_COLUMN 0xb
#define LIST_ONE_MENU_LINE         1
WSMenu list_one_menu(
    /* .MenuColumnStep = */ LIST_ONE_MENU_STEP,
    /* .NoOfMenuItems = */ LIST_ONE_MENU_ENTRIES,
    /* .MenuStartsAtColumn = */ LIST_ONE_MENU_START_COLUMN,
    /* .MenuLine = */ LIST_ONE_MENU_LINE
    );

//   0123456789abcdef
//  +----------------+
// 0|P1 Refs 47 53 51|
// 1|>  NoUse Reset X|
//  +----------------+
WSMenu ctrl_one_menu(
    /* .MenuColumnStep = */ 6,
    /* .NoOfMenuItems = */ 3,
    /* .MenuStartsAtColumn = */ 3,
    /* .MenuLine = */ 1
    );


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
    _internal_error = false;
    _p_current_menu = nullptr;

    DEBUG_P("Init LCD...");

    while (!Serial) {};

    DEBUG_P("Check 4 LCD");

    // See http://playground.arduino.cc/Main/I2cScanner
    Wire.begin();
    Wire.beginTransmission(LCD_I2C_ADDRESS);
    int error = Wire.endTransmission();
    if (error) {
        DEBUG_P("ERROR, LCD NOT FOUND!\n");
    } else {
        DEBUG_P("Found LCD :)\n");
    }

    if (error != 0) {
        setSystemInternalError();
        system_panic_wo_lcd_no_return();

    } else {
        initGlyphs(lcd);
        lcd.begin(16, 2); // initialize the lcd
        lcd.setBacklight(255);
        lcd.home(); lcd.clear();
        lcd.print(F("Water system 0.1\n"));

        lcd.setCursor(0, 1);
        for (int i=0; i<8; i++)
            lcd.write(i);
    }
}

void WaterSystem::setSystemInternalError()
{
    DEBUG_P(" !!! INTERNAL ERROR !!!");
    _internal_error = true;
}

bool WaterSystem::hasInternalError()
{
    return _internal_error;
}


inline saneModuleIndex_t WaterSystem::_saneModuleIndex(byte moduleIndex)
{
    return saneModuleIndex_t { (byte)(moduleIndex % MAX_MODULE_COUNT) };
}

void WaterSystem::selectModuleIndex(saneModuleIndex_t saneIndex)
{
    byte index = saneIndex.moduleIndex;
    if (_saneModuleIndex(index).moduleIndex != saneIndex.moduleIndex) {
        DEBUG_P("InternalError: rcv bad idx as sane"); DEBUG("%d", index);
        setSystemInternalError();
        system_panic_no_return();
    };

    noInterrupts();
    _selected_module = saneIndex;
    _some_module_selected = true;
    interrupts();
}

// byte WaterSystem::selectSaneModuleIndex(byte moduleIndex)
// {
//     noInterrupts();
//     _selected_module = _saneModuleIndex(moduleIndex);
//     _some_module_selected = true;
//     interrupts();
//     return _selected_module.moduleIndex;
// }

void WaterSystem::deactivateSelection()
{
    _some_module_selected = false;
}

void WaterSystem::selectNextMenuEntry()
{
    if (_p_current_menu != nullptr) {
        _p_current_menu->nextMenuEntry();
        showMenuCursor();
    }
}

bool WaterSystem::hasActiveModule(saneModuleIndex_t *pModuleIdx)
{
    if (!_some_module_selected) {
        return false;
    };

    noInterrupts();
    pModuleIdx->moduleIndex = _selected_module.moduleIndex;
    interrupts();
    return true;
}

void WaterSystem::listAll()
{
    _clearScreenNoCursor();

    ZERO_INIT_ARRAY(_lcd_line0);
    ZERO_INIT_ARRAY(_lcd_line1);

    for (byte i=0; i<MAX_MODULE_COUNT; i++) {

        const int x = i * 3;
        lcd.setCursor(x, 0);
        sprintf(_lcd_line0, "%.1d", i);
        lcd.write(_plant->location()); lcd.print(_lcd_line0);

        lcd.setCursor(x, 1);
        int delta = sp[i].getNormalizedDeltaToThreshold();
        sprintf(_lcd_line1, "%+.1d", delta);
        lcd.print(_lcd_line1);


        DEBUG_P("listAll delta\t value");
        DEBUG("\t\t%s\t%s", _lcd_line0, _lcd_line1);
    }

    // the menu item
    lcd.setCursor(12, 0);
    lcd.write(_burger_menu->location());
    lcd.print(F("  X"));

    lcd.setCursor(15, 1);
    if (hasInternalError())
        lcd.write(_skull->location());

    lcd.noBlink();

}

#if 0
void WaterSystem::showScreen()
{
    lcd.display();
    lcd.setBacklight(255);
    lcd.clear();
    lcd.home();
    lcd.print(_lcd_line0);
    lcd.setCursor(0, 1);
    lcd.print(_lcd_line1);
    lcd.noCursor();
    lcd.noBlink();
}
#endif

void WaterSystem::_resetMenu()
{
    if (_p_current_menu != nullptr) {
        _p_current_menu->resetMenu();
    }
}

void WaterSystem::manualWaterCurrent()
{
    _clearScreenNoCursor();
    DEBUG_P("Manual watering\n");
    lcd.print(F("Manual watering\n"));
    lcd.setCursor(0, 1);
    lcd.write(_plant->location());

    saneModuleIndex_t saneIdx;
    if (hasActiveModule(&saneIdx)) {
        lcd.write('0' + saneIdx.moduleIndex);
        delay(HUMAN_PERCEPTIBLE_MS);
        lcd.setCursor(0, 1);
        lcd.write(_rain_plant->location());
        sp[saneIdx.moduleIndex].manualGiveWaterAndAdjustDry();
    } else {
        DEBUG_P("manualWater command, but no active module");
        lcd.write('?');
        setSystemInternalError();
        lcd.print(F(" PANIC! \7"));
        system_panic_no_return();
    }
    delay(MIN_REWATER_INTERVAL_MS);
}

void WaterSystem::showMenuCursor()
{
    if (_p_current_menu != nullptr) {
        int column = _p_current_menu->getLcdCursorColumn();
        int line = _p_current_menu->getLcdCursorLine();

        DEBUG_P("showMenuCursor:menuPtr\t col\t ln\n");
        DEBUG("\t\t%.4x\t%.2d\t%.2d", (uintptr_t)_p_current_menu, column, line);
        lcd.setCursor(column, line);
        lcd.blink();
    }
}

bool WaterSystem::_confirmIndexIsSane(byte moduleIndex, saneModuleIndex_t *pSaneIndex)
{
    *pSaneIndex = _saneModuleIndex(moduleIndex);
    if (moduleIndex != pSaneIndex->moduleIndex) {
        setSystemInternalError();
        return false;
    }

    return true;
}

bool WaterSystem::statusOne(byte moduleIndex)
{
    saneModuleIndex_t saneIndex;
    if (!_confirmIndexIsSane(moduleIndex, &saneIndex)) {
        DEBUG_P("INTERNAL ERROR: statusOne: no module!");
        return false;
    };

    selectModuleIndex(saneIndex);
    showStatusCurrentOne();
    return true;

}

void WaterSystem::_continueLine0AndWriteLine1()
{
    // print to screen continuing on line0
    lcd.print(_lcd_line0);
    lcd.setCursor(0, 1);
    lcd.print(_lcd_line1);
}

//   0123456789abcdef
//  +----------------+
// 0|P1 Now:52 Ref:50|
// 1|WET(d:+2)  ☔ > X|
//  +----------------+
static char menuOne0Fmt[] = { " Now:%.2d Ref:%.2d" };
static char menuOne1Fmt[] = { "WET(d:%+.1d)  %c > X" };
void WaterSystem::showStatusCurrentOne()
{
    // Initialize to silence -Wmaybe-uninitialized, incorrectly triggered here
    // and is NOT silenced by
    //   #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    byte saneIdx = MAX_MODULE_COUNT;
    if (!_clearLcdAndListCurrentPlant(saneIdx)) {
        DEBUG_P("showStatusCurrentOne: no module");
        return;
    }

    snprintf(_lcd_line0, lcdLineBufLen - 1, menuOne0Fmt,
        sp[saneIdx].getLastMoisturePercent(),
        sp[saneIdx].getDryPercent());

    snprintf(_lcd_line1, lcdLineBufLen - 1, menuOne1Fmt,
        sp[saneIdx].getNormalizedDeltaToThreshold(), _rain_plant->location());

    _continueLine0AndWriteLine1();
}

bool WaterSystem::showCtrlOne(byte moduleIndex)
{
    saneModuleIndex_t saneIndex;
    if (_confirmIndexIsSane(moduleIndex, &saneIndex)) {
        DEBUG_P("INTERNAL ERROR: showCtrlOne: no module!");
        return false;
    };

    selectModuleIndex(saneIndex);
    showCtrlCurrentOne();
    return true;
}

void WaterSystem::_clearScreenNoCursor()
{
    lcd.clear();
    lcd.noCursor();
    lcd.setBacklight(255);
    lcd.display();
}

// "P. Refs .. .. ..";
// ">  ..Use Reset X";

static const char ctrlOne0Fmt[] =  " Refs %.2s %.2s %.2s";
static const char ctrlOne1Fmt[] = ">  %.2sUse Reset X";

bool WaterSystem::_clearLcdAndListCurrentPlant(byte &selectedIdx)
{
    _clearScreenNoCursor();

    lcd.write(_plant->location());
    if (!_some_module_selected) {
        lcd.write('?');
    } else {
        selectedIdx = _selected_module.moduleIndex;
        lcd.write(selectedIdx + '0');
    }
    return _some_module_selected;
}

void WaterSystem::showCtrlCurrentOne()
{
    byte saneIdx;
    if (!_clearLcdAndListCurrentPlant(saneIdx)) {
        DEBUG_P("showCtrlCurrentOne: no module");
        return;
    }

    snprintf(_lcd_line0, lcdLineBufLen - 1, ctrlOne0Fmt,
             sp[saneIdx].getTooDryPercentAsStr(0),
             sp[saneIdx].getTooDryPercentAsStr(1),
             sp[saneIdx].getTooDryPercentAsStr(2)
             );

    snprintf(_lcd_line1, lcdLineBufLen - 1, ctrlOne1Fmt,
        sp[saneIdx].isModuleUsed() ? "In" : "No");

    DEBUG("%s", _lcd_line0);
    DEBUG("%s", _lcd_line1);

    // print to screen continuing on line0
    _continueLine0AndWriteLine1();

}

void WaterSystem::openMenu(WSMenu *pMenu)
{
    _p_current_menu = pMenu;
    // FIXME: reprint LCD lines
    // menus which are openend and selected need a separate wrapper API
    _resetMenu();
    // showScreen(); // probably not needed
    showMenuCursor();
}

void WaterSystem::showSysStatus()
{
    _clearScreenNoCursor();

    char msg[lcdLineBufLen] = {0};

    if (_some_module_selected) {
        snprintf(msg, lcdLineBufLen,
            "Logs: module %d",
            _selected_module.moduleIndex);
    } else {
        memcpy_P(msg, F("Logs: [NoSelect]"), sizeof(msg));
    }
    lcd.print(msg);
    lcd.setCursor(0, 1);
    lcd.print(F("TODO:Sys/Mod Log"));

}

/// Mostly a debug function to display not yet implemented states
void WaterSystem::showState(uint8_t stateNo)
{
    _clearScreenNoCursor();
    snprintf(_lcd_line0, lcdLineBufLen, " _ state = %.d _", stateNo);
    lcd.print(_lcd_line0);
}

void WaterSystem::autoWater()
{
    _clearScreenNoCursor();
    //TODO: no LCD in autowater?
    // lcd.setBacklight(0);

    DEBUG_P("Auto water...\n");
    lcd.print(F("Auto water...\n"));
    delay(HUMAN_PERCEPTIBLE_MS);

    // we don't want to care about timeout sync-ing, or buttons,
    // but I think it's safe to timeout or have buttons pressed
    // since we shouldn't call the state machine's
    // stateUpdated() function during the execution of this
    // function
    //
    // TODO: maybe detachInterrupt/attachInterrupt is better
    // to disable buttons?

    #define AUTOWATER_STATUS_LEN_PER_PLANT 3U
    for (uint8_t i = 0; i < MAX_MODULE_COUNT; i++) {
        DEBUG("AW%u", i);

        // just add each plant on the second row during processing
        // e.g.:  0(rain) 1(rain) 2(skip) 3(disabled)
        lcd.setCursor(i*AUTOWATER_STATUS_LEN_PER_PLANT, 1);
        lcd.write('0' + i);
        delay(HUMAN_PERCEPTIBLE_MS);


        SensorAndPump *module = &sp[i];
        if (module->isModuleUsed()) {

            delay(HUMAN_PERCEPTIBLE_MS);

            //TODO: log "auto watered i result"
            if (module->tryAutoWater()) {
                lcd.write(_rain_plant->location());
                DEBUG_P("watered\n");
            } else {
                lcd.write(_plant->location());
                DEBUG_P("skipped (not dry)\n");
            };
        } else {
            lcd.write('X');
            DEBUG_P("Skipped (disabled)\n");
        };
        delay(HUMAN_PERCEPTIBLE_MS);
    }

    DEBUG_P("Finished autowatering cycle.\n");
    delay(HUMAN_PERCEPTIBLE_MS);
}

ulong timedelta(ulong ref_timestamp, ulong now)
{
    if (now >= ref_timestamp)
        return now - ref_timestamp;
    else // overflow
        return ULONG_MAX - ref_timestamp + now;
}
