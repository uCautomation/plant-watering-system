#include <LiquidCrystal_PCF8574.h>

#include "LcdGlyphs.h"

const int plant[LCD_LINES_PER_GLYPH] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00001000,
    0b00000100,
    0b00011111,
    0b00001110
};

const int rain_plant[LCD_LINES_PER_GLYPH] = {
    0b00010101,
    0b00000000,
    0b00010101,
    0b00000000,
    0b00001000,
    0b00000100,
    0b00011111,
    0b00001110
};

const int right_arrow[LCD_LINES_PER_GLYPH] = {
    0b00000000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00011001,
    0b00000010,
    0b00000100,
    0b00001000
};

const int level_up[LCD_LINES_PER_GLYPH] = {
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111
};

const int level_mid[LCD_LINES_PER_GLYPH] = {
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111
};

const int level_low[LCD_LINES_PER_GLYPH] = {
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00011111
};

const int burger_menu[LCD_LINES_PER_GLYPH] = {
    0b00000000,
    0b00000000,
    0b00011111,
    0b00000000,
    0b00011111,
    0b00000000,
    0b00011111,
    0b00000000
};

const int skull[LCD_LINES_PER_GLYPH] = {
    0b00000000,
    0b00000000,
    0b00001110,
    0b00010101,
    0b00011111,
    0b00001010,
    0b00000000,
    0b00000000
};


LCDGlyph::LCDGlyph(LiquidCrystal_PCF8574 &lcd, int location, const int charmap[])
{
    lcd.createChar(location, (int *)charmap); // drop const with cast; the called API is not const
    _custom_glyph_location = location;
}

inline int LCDGlyph::location()
{
    return _custom_glyph_location;
}
