#include <Arduino.h>

bool Serial = true; //mock

int millis() {return 1234;};
void delay(int delay_ms) {};
void pinMode(int pin, int mode) {};
void digitalWrite(int pin, int level) {};

static int mockAnalogPin = 0U;
static int mockAnalogReadCount = 0U;
void setExpectAnalogRead(int anpin, int value)
{
  // TODO: Add support for multiple analog pins and values
  (void)anpin;

  mockAnalogPin = value;
  mockAnalogReadCount++;
};
int analogRead(int anpin) {
  if (mockAnalogReadCount>0) {
    mockAnalogReadCount--;
    return mockAnalogPin;
  }
  return 0;
};

void nextButISR(void) {};
void okButISR(void) {};

void interrupts() {};
void noInterrupts() {};

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain(long v, long min, long max) {
    return v < min ? min : v > max ? max : v;
}

