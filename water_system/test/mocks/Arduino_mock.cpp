#include <Arduino.h>

bool Serial = true; //mock

int millis() {return 1234;};
void delay(int delay_ms) {};
void pinMode(int pin, int mode) {};
void digitalWrite(int pin, int level) {};
int analogRead(int anpin) {return 0;};
//void panicLEDToggle() {};

void nextButISR(void) {};

void okButISR(void) {};

