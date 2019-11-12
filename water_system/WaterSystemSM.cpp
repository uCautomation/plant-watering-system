#include <Arduino.h>
#include <Wire.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int g_panic_led = 0;

void panicLEDToggle() {
  g_panic_led = !g_panic_led;
  digitalWrite(LED_BUILTIN, g_panic_led);
}


    WaterSystemSM::WaterSystemSM(ulong current_milli) {
        _state = wss_start;
        _last_transition_milli = current_milli;

        nextBut = new ButtonWS(nextButPin, nextButISR);
        okBut = new ButtonWS(okButPin, okButISR);


        DEBUG("Init LCD...");

        while (! Serial) {};

        DEBUG("Dose: check for LCD");

        // See http://playground.arduino.cc/Main/I2cScanner
        Wire.begin();
        Wire.beginTransmission(0x27);
        int error = Wire.endTransmission();
        DEBUG("Error: %d: LCD %s" "found.", error, 0 == error ? "" : "not ");


        if (error != 0) {
            _state = wss_panic;
            _timeout = 0;

        } else {
            lcd.begin(16, 2); // initialize the lcd
            lcd.setBacklight(255);
            lcd.home(); lcd.clear();
            lcd.print("Water system 0.1");

            _timeout = _state_to[_state];
        }
    }

    bool WaterSystemSM::stateUpdated(ulong current_milli) {

        if (okBut->isPressed(current_milli)) {

            DEBUG("OK pressed");

            noInterrupts();
            _state = _okBut_next_state[_state];
            _last_transition_milli = current_milli;
            interrupts();

            return true;
        }

        if (nextBut->isPressed(current_milli)) {

            DEBUG("Next pressed");

            noInterrupts();
            _state = _nextBut_next_state[_state];
            _last_transition_milli = current_milli;
            interrupts();

            return true;
        }

        ulong time_delta = timedelta(_last_transition_milli, current_milli);
        if (time_delta >= _timeout) {

            DEBUG("Timeout from state %u", _state);

            noInterrupts();
            wss_type _next_state = _to_next_state[_state];
            bool state_changed = (_next_state != _state);

            if (state_changed) {
                DEBUG("state changes via timeout");
                _state = _next_state;
                _last_transition_milli = current_milli;
            } else {
                DEBUG("same state");
            }
            interrupts();

            return state_changed;
        }

        DEBUG("No state change");

        return false;
    }

    wss_type WaterSystemSM::State() {
      return _state;
    };

//private:

