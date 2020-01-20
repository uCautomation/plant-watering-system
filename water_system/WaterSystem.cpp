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
char menuAll0[lcdLineBufLen] = { "P1 P2 P3 P4 == X" };
char menuAll1[lcdLineBufLen] = { ".. .. .. ..    ." };
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
char menuOne0[lcdLineBufLen] = { "P1 Now:.. Ref:.." };
char menuOne1[lcdLineBufLen] = { "WET(d:..)  W > X" };
// char menuOne0Fmt[] = { "P1 Now:%.2d Ref:%.2d" };
// char menuOne1Fmt[] = { "WET(d:%+.1d)  W > X" };
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
    DEBUG("Error: %d: LCD %s" "found.", error, 0 == error ? "" : "not ");

    if (error != 0) {
        setSystemInternalError();
        system_panic_no_return();

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
        DEBUG("InternError: rcv bad idx as sane %d", index);
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
    lcd.display();
    lcd.setBacklight(255); lcd.home(); lcd.clear();

    // TODO: use _lcd_line0
    // char buf[51] = ".    .    |    .    |    .    |    .    ";
    char buf[lcdLineBufLen] = { 0 };
    // TODO: populate _lcd_line1
    char line2[lcdLineBufLen] = { 0U };
    for (byte i=0; i<MAX_MODULE_COUNT; i++) {

        const int x = i * 3;
        lcd.setCursor(x, 0);
        // TODO: use the WaterSystem::_plant glyph
        sprintf(buf, "%.1d", i);
        lcd.write(_plant->location()); lcd.print(buf);

        lcd.setCursor(x, 1);
        int delta = sp[i].GetNormalizedDeltaToThreshold();
        sprintf(line2, "%.2d", delta);
        lcd.print(line2);


        DEBUG("listAll lines: %s %s", buf, line2);
    }

    // the menu item
    lcd.setCursor(12, 0);
    lcd.write(_burger_menu->location());
    lcd.print("  X");

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
    saneModuleIndex_t saneIdx;
    if (hasActiveModule(&saneIdx)) {
        sp[saneIdx.moduleIndex].ManualGiveWaterAndAdjustDry();
    } else {
        DEBUG_P("manualWater command, but no active module");
        system_panic_no_return();
    }
    delay(MIN_REWATER_INTERVAL_MS);
}

void WaterSystem::showMenuCursor()
{
    if (_p_current_menu != nullptr) {
        int column = _p_current_menu->getLcdCursorColumn();
        int line = _p_current_menu->getLcdCursorLine();

        DEBUG("showMenuCursor:%.4x col=%.2d, ln=%.2d", (uintptr_t)_p_current_menu, column, line);
        lcd.setCursor(column, line);
        lcd.blink();
    }
}

bool WaterSystem::listCtrlOne(byte currentModule)
{
    saneModuleIndex_t saneIndex = _saneModuleIndex(currentModule);
    if (currentModule != saneIndex.moduleIndex) {
        DEBUG_P("INTERNAL ERROR: listCtrlOne: no module!");
        setSystemInternalError();

        return false;
    };

    selectModuleIndex(saneIndex);
    listCurrentCtrlOne();
    return true;
}

char listCtrlOne0[lcdLineBufLen] = "P. Refs .. .. ..";
char listCtrlOne1[lcdLineBufLen] = ">  ..Use Reset X";

static const char listCtrlOne0Fmt[] = "%.1d Refs %.2s %.2s %.2s";
static const char listCtrlOne1Fmt[] = ">  %.2sUse Reset X";

void WaterSystem::listCurrentCtrlOne()
{
    lcd.clear();
    lcd.home();
    if (!_some_module_selected) {
        DEBUG_P("listCurrentCtrlOne: no module");
        lcd.write(_plant->location());
        lcd.write('?');
        return;
    }
    byte saneIdx = _selected_module.moduleIndex;

    snprintf(listCtrlOne0, lcdLineBufLen - 1, listCtrlOne0Fmt,
             saneIdx,
             sp[saneIdx].GetTooDryPercent(0),
             sp[saneIdx].GetTooDryPercent(1),
             sp[saneIdx].GetTooDryPercent(2)
             );

    snprintf(listCtrlOne1, lcdLineBufLen - 1, listCtrlOne1Fmt,
        sp[saneIdx].isModuleUsed() ? "In" : "No");

    // print to screen
    DEBUG("%s", listCtrlOne0);
    DEBUG("%s", listCtrlOne1);

    lcd.setBacklight(255); lcd.home(); lcd.clear(); lcd.noCursor();
    int ploc = _plant->location();
    DEBUG("Plant location >%d<\n", ploc);
    lcd.write(_plant->location());
    lcd.print(listCtrlOne0);
    lcd.setCursor(0, 1);
    lcd.print(listCtrlOne1);

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



ulong timedelta(ulong ref_timestamp, ulong now)
{
    if (now >= ref_timestamp)
        return now - ref_timestamp;
    else // overflow
        return ULONG_MAX - ref_timestamp + now;
}
