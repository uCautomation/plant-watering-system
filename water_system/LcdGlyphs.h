#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>

#define LCD_LINES_PER_GLYPH 8

extern const int plant[LCD_LINES_PER_GLYPH];
extern const int rain_plant[LCD_LINES_PER_GLYPH];
extern const int right_arrow[LCD_LINES_PER_GLYPH];
extern const int level_up[LCD_LINES_PER_GLYPH];
extern const int level_mid[LCD_LINES_PER_GLYPH];
extern const int level_low[LCD_LINES_PER_GLYPH];
extern const int burger_menu[LCD_LINES_PER_GLYPH];
extern const int skull[LCD_LINES_PER_GLYPH];

class LCDGlyph {
    private:
        int _custom_glyph_location;
    public:
        LCDGlyph(LiquidCrystal_PCF8574 &lcd, int location, const int charmap[]);
        inline int location();
};
