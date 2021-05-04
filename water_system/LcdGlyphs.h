#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>

#define LCD_LINES_PER_GLYPH 8

extern const byte plant[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte rain_plant[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte right_arrow[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte level_up[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte level_mid[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte level_low[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte burger_menu[LCD_LINES_PER_GLYPH] PROGMEM;
extern const byte skull[LCD_LINES_PER_GLYPH] PROGMEM;

class LCDGlyph {
    private:
        int _custom_glyph_location;

    public:
        LCDGlyph(LiquidCrystal_PCF8574 &lcd, int location, const byte charmap_P[] PROGMEM);
        int location();
};
