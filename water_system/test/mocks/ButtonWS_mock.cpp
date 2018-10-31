#include "ButtonWS.h"

ButtonWS::ButtonWS(int pin, isr butChISR) : _pin(pin), _debounceDelay(50) {};
void ButtonWS::changed(void) {};
bool ButtonWS::isPressed(ulong now) { return true; };
