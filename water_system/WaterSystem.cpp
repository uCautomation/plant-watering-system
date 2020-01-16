#include <Arduino.h>

#include "ws_defs.h"
#include "WaterSystem.h"
#include "WSRuntime.h"
#include "WSMenu.h"


LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const byte lcdLineBufLen = 17;

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
char menuCtrlOne0[lcdLineBufLen] = "P1 Refs .. .. ..";
char menuCtrlOne1[lcdLineBufLen] = ">  ..Use Reset X";
// const char menuCtrlOne0[] = "P1 Refs %.2d %.2d %.2d";
// const char menuCtrlOne1[] = ">  %.2sUse Reset X";
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

    DEBUG("Init LCD...");

    while (!Serial) {};

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

void WaterSystem::setSystemInternalError()
{
    _internal_error = true;
}

bool WaterSystem::hasInternalError()
{
    return _internal_error;
}

byte WaterSystem::selectSaneModuleIndex(byte moduleIndex)
{
    _selected_module = moduleIndex % MAX_MODULE_COUNT;
    return _selected_module;
}

void WaterSystem::activateSelection()
{
    _selected_module = saneModuleIndex();
    _some_module_selected = true;
}

byte WaterSystem::saneModuleIndex()
{
    return _selected_module % MAX_MODULE_COUNT;
}

void WaterSystem::deactivateSelection()
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

void WaterSystem::system_list()
{
    lcd.display();
    lcd.setBacklight(255); lcd.home(); lcd.clear();

    char buf[51] = ".    .    |    .    |    .    |    .    ";
    char line2[17] = { 0U };
    for (int i=0; i<MAX_MODULE_COUNT; i++) {

        const int x = i * 3;
        lcd.setCursor(x, 0);
        // TODO: use the WaterSystem::_plant glyph
        sprintf(buf, "P%.1d ", i);
        lcd.print(buf);

        lcd.setCursor(x, 1);
        int delta = sp[i].GetNormalizedDeltaToThreshold();
        sprintf(line2, "%.2d ", delta);
        lcd.print(line2);


        DEBUG("system_list(): %s %s", buf, line2);
    }

    // the menu item
    lcd.setCursor(12, 0);
    sprintf(buf, "== X");
    lcd.print(buf);

    lcd.setCursor(15, 1);
    if (hasInternalError())
        lcd.write(_skull->location());

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

void WaterSystem::_resetMenu()
{
    if (_p_current_menu != nullptr) {
        _p_current_menu->resetMenu();
    }
}

void WaterSystem::_showMenuCursor()
{
    if (_p_current_menu != nullptr) {
        lcd.setCursor(getLcdCursorColumn(), getLcdCursorLine());
        lcd.blink();
    }
}

void WaterSystem::openMenu()
{
    this->_resetMenu();
    // showScreen(); // probably not needed
    this->_showMenuCursor();
}


#endif


ulong timedelta(ulong ref_timestamp, ulong now)
{
    if (now >= ref_timestamp)
        return now - ref_timestamp;
    else // overflow
        return ULONG_MAX - ref_timestamp + now;
}
