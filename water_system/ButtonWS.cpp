#include <Arduino.h>

#include "ButtonWS.h"

#include "WaterSystem.h"

ButtonWS::ButtonWS(int pin, isr butChISR) : _pin(pin), _debounceDelay(50)
{
    _milli = millis();
    _lastmilli = _milli;
    pinMode(_pin, INPUT_PULLUP);
    delay(100);
    _state = digitalRead(_pin);
    DEBUG("ButtonWS %d state %d", _pin, _state);
    attachInterrupt(digitalPinToInterrupt(_pin), butChISR, CHANGE);
}

void ButtonWS::changed(void)
{
    _milli = millis();
}

bool ButtonWS::isPressed(ulong now)
{
    if (_milli ==_lastmilli) return false;

    ulong td = timedelta(_milli, now);
    if (td < _debounceDelay) return false;
    //DEBUG("Debounced");

    byte s = digitalRead(_pin);
    //DEBUG("%s", s == LOW ? "LOW" : "HIGH");

    //s = digitalRead(_pin);
    bool pressed = (s == LOW);

    //DEBUG("B%d (state=%d): Debounced, %spressed and %schanged!", _pin, s, pressed ? "" : "NOT ", s != _state ? "" : "NOT " );
    //DEBUG("ButtonWS %d : _lastmilli=%lu _milli=%lu, now=%lu", _pin, _lastmilli, _milli, now);

    _lastmilli = _milli;
    _state = s;

    return pressed;
}
