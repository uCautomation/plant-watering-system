#include <Arduino.h>
#include <Wire.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"
#include "SensorAndPump.h"

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int g_panic_led = 0;

void panicLEDToggle() {
  g_panic_led = !g_panic_led;
  digitalWrite(LED_BUILTIN, g_panic_led);
}


    WaterSystemSM::WaterSystemSM() {
      _state = wss_start;

      int m = millis();
      nextBut = new ButtonWS(nextButPin, nextButISR);
      okBut = new ButtonWS(okButPin, okButISR);
    };

    void WaterSystemSM::Init(void) {
        int error;

        DEBUG("Init LCD...");

        while (! Serial);

        DEBUG("Dose: check for LCD");

        // See http://playground.arduino.cc/Main/I2cScanner
        Wire.begin();
        Wire.beginTransmission(0x27);
        error = Wire.endTransmission();
        DEBUG("Error: %d: LCD %s" "found.", error, 0 == error ? "" : "not ");

        if (error != 0) {
            _state = wss_panic;
            _timeout = 0;
            goto exitfn;
        }

        lcd.begin(16, 2); // initialize the lcd
        lcd.setBacklight(255);
        lcd.home(); lcd.clear();
        lcd.print("Water system 0.1");
        _timeout = _state_to[_state];

        exitfn:
            ;
    }

    bool WaterSystemSM::TOTransition(ulong tdelta)
    {

      DEBUG("TOTransition: _state=%d tdelta=%lu _timeout=%lu", _state, _timeout, tdelta);

      if (tdelta < _timeout) {
        DEBUG("no timeout");
        return false;
      }

      if (_to_next_state[_state] == _state) {
        DEBUG("same state");
        return true;
      }

      _to_transition(_to_next_state[_state]);
      DEBUG("TRANSITIONED to %d", _state);
      return true;
    }

    ulong WaterSystemSM::timeout() { return _timeout; }

    bool WaterSystemSM::nextChangedTransition() {
      DEBUG("TODO: Try Next ButtonWS changed transition: _state=%d", _state);
      panicLEDToggle();

      return false;
    }

    bool WaterSystemSM::okChangedTransition() {
      DEBUG("TODO: Try OK ButtonWS changed transition: _state=%d", _state);
      panicLEDToggle();

      return false;
    }

    wss_type WaterSystemSM::State() {
      return _state;
    };

  //private:

    void WaterSystemSM::_to_transition(wss_type nextstate)
    {
      switch (nextstate) {
        case wss_panic:
          panicLEDToggle();
          if (_state != wss_start)
            lcd.setBacklight(0);
          break;
          ;;

        case wss_listing:
          _list();
          break;
          ;;

        case wss_sleep:
          lcd.setBacklight(0);
          lcd.noDisplay();
          break;
          ;;
      }
      _state = nextstate;
      _timeout = _state_to[_state];
    }

    void WaterSystemSM::_list()
    {
      lcd.display();
      lcd.setBacklight(255);lcd.home(); lcd.clear();

      char buf[51] = ".    .    |    .    |    .    |    .    ";
      for( int i=0; i<MAX_MODULE_COUNT; i++) {
        int moist = sp[i].GetCurrentMoisture();
        sprintf(buf, "S%.1d=%d  ", i, moist);
        lcd.setCursor((i&0x1) * 8, i>>1);
        lcd.print(buf);
        DEBUG("_list(): %s", buf);
      }
    }
