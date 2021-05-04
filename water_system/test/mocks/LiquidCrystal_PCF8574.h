#ifndef LIQUIDCRYSTAL_PCF8574_H
#define LIQUIDCRYSTAL_PCF8574_H

#include "Arduino.h"

class LiquidCrystal_PCF8574 {
	public:
		LiquidCrystal_PCF8574(int addr) {};
		void begin(int x, int y) {};
		void display() {};
		void noDisplay() {};
		void setBacklight(int level) {};
		void setCursor(int x, int y) {};
		void home() {};
		void clear() {};
		void print(const char *) {};
		void cursor() {};
		void noCursor() {};
		void blink() {};
		void noBlink() {};
		void createChar(int location, byte charmap[]) {};
		void write(int c) {};
};

#endif // LIQUIDCRYSTAL_PCF8574_H
