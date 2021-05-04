#include <LiquidCrystal_PCF8574.h>

#include "LcdGlyphs.h"

const byte plant[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00001000,
    0b00000100,
    0b00011111,
    0b00001110,
    0b00000000,
};

const byte rain_plant[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00000000,
    0b00010101,
    0b00000000,
    0b00001000,
    0b00000100,
    0b00011111,
    0b00001110,
    0b00000000,
};

const byte right_arrow[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00000000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00011001,
    0b00000010,
    0b00000100,
    0b00001000
};

const byte level_up[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111
};

const byte level_mid[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111
};

const byte level_low[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00011111
};

const byte burger_menu[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00011111,
    0b00000000,
    0b00011111,
    0b00000000,
    0b00011111,
    0b00000000
};

const byte skull[LCD_LINES_PER_GLYPH] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00001110,
    0b00010101,
    0b00011111,
    0b00001010,
    0b00000000,
    0b00000000
};


LCDGlyph::LCDGlyph(LiquidCrystal_PCF8574 &lcd, int location, const byte charmap_P[] PROGMEM)
{
    byte charmap[LCD_LINES_PER_GLYPH];

    memcpy_P((void *)charmap, charmap_P, sizeof(charmap));
    lcd.createChar(location, charmap);
    _custom_glyph_location = location;
}

int LCDGlyph::location()
{
    return _custom_glyph_location;
}
