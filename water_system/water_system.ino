#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <limits.h>
#include <stdio.h>

#define DEBUG_ON
#include "DebugWS.h"

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define SENSOR_START_DELAY_MS 10
#define PUMP_ON_MS 200

#define MAX_MODULE_COUNT 4

typedef unsigned long ulong;

class SensorAndPump {
  public:
    SensorAndPump
    (
      int VsensorPin,
      int SensorPin,
      int PumpCmdPin,
      int DryValue=0,
      int WetValue=1024,
      int pumpOnMS=PUMP_ON_MS
    )
      :
      _vSensorPin(VsensorPin),
      _sensorPin(SensorPin),
      _pumpCmdPin(PumpCmdPin),
      _dryValue(DryValue),
      _wetValue(WetValue),
      _pumpOnMS(pumpOnMS)
      {
        pinMode(_vSensorPin, OUTPUT);
        // analog pins are ready for AnalogRead by default, so _sensorPin works out of the box
        pinMode(_pumpCmdPin, OUTPUT);
      }

    void SensorOn(void)
    {
      digitalWrite(_vSensorPin, HIGH);//turn sensor "On"
    }

    void SensorOff(void)
    {
      digitalWrite(_vSensorPin, LOW);//turn sensor "Off"
    }

    int GetCurrentMoisture()
    {
      this->SensorOn();
      delay(SENSOR_START_DELAY_MS);//wait for the sensor to stabilize
      _lastMoisture = analogRead(_sensorPin);//Read the SIG value form sensor
      this->SensorOff();

      return _lastMoisture;//send current moisture value
    }

    int SetTooDry(int dryValue)
    {
      _dryValue = dryValue;
    }

    int SetTooWet(int wetValue)
    {
      _wetValue = wetValue;
    }

    void GiveWater()
    {
      digitalWrite(_pumpCmdPin, HIGH); // open valve to let water run
      delay(_pumpOnMS);
      digitalWrite(_pumpCmdPin, LOW); // close valve
    }

  private:
    int _vSensorPin, _sensorPin, _pumpCmdPin;
    int _dryValue, _wetValue, _lastMoisture;
    int _pumpOnMS;
};

// Sensor+Pump modules
SensorAndPump sp[MAX_MODULE_COUNT] = {
  {4, A0, 5}, // D4 is Vsens, A0 = Sens, D5 is Valve cmd
  {6, A1, 7},
  {8, A2, 9},
  {10, A0, 11},
};

typedef enum {
  wss_start = 0,
  wss_sleep,
  wss_listing,
  wss_menusel,
  wss_manualwater,
  wss_list,
  wss_probe,
  wss_autowater,
  wss_panic,

  WSS_NOSTATE
} wss_type;

const byte nextButPin = 3;
const byte okButPin = 2;

void nextButISR(void);
void okButISR(void);
typedef void isr(void);

class Button {
  public:
    Button(int pin, isr butChISR) : _pin(pin), _debounceDelay(50)
    {
      _milli = millis();
      _lastmilli = _milli;
      pinMode(_pin, INPUT_PULLUP);
      delay(100);
      _state = digitalRead(_pin);
      DEBUG("Button %d state %d", _pin, _state);
      attachInterrupt(digitalPinToInterrupt(_pin), butChISR, CHANGE);
    }

    void changed(void)
    {
      _milli = millis();
    }

    bool isPressed(ulong now)
    {
      if (_milli ==_lastmilli) return false;
      byte s = digitalRead(_pin);
      //DEBUG("%s", s == LOW ? "LOW" : "HIGH");

      ulong td = timedelta(_milli, now);
      if (td < _debounceDelay) return false;
      //DEBUG("Debounced");

      //s = digitalRead(_pin);
      bool pressed = (s == LOW);

      //DEBUG("B%d (state=%d): Debounced, %spressed and %schanged!", _pin, s, pressed ? "" : "NOT ", s != _state ? "" : "NOT " );
      //DEBUG("Button %d : _lastmilli=%lu _milli=%lu, now=%lu", _pin, _lastmilli, _milli, now);

      _lastmilli = _milli;
      _state = s;

      return pressed;
    }

  private:
    const byte _pin;
    byte _state;
    ulong _lastmilli;
    volatile ulong _milli;
    const ulong _debounceDelay;
};

int g_panic_led = 0;

void panicLEDToggle() {
  g_panic_led = !g_panic_led;
  digitalWrite(LED_BUILTIN, g_panic_led);
}

class WaterSystemSM {
  public:
    class Button *nextBut, *okBut;

    WaterSystemSM() {
      _state = wss_start;

      int m = millis();
      nextBut = new Button(nextButPin, nextButISR);
      okBut = new Button(okButPin, okButISR);
    };

    void Init(void) {
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

    bool TOTransition(ulong tdelta)
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

    ulong timeout() { return _timeout; }

    bool nextChangedTransition() {
      DEBUG("TODO: Try Next button changed transition: _state=%d", _state);
      panicLEDToggle();

      return false;
    }

    bool okChangedTransition() {
      DEBUG("TODO: Try OK button changed transition: _state=%d", _state);
      panicLEDToggle();

      return false;
    }

  private:

    wss_type _state;
    wss_type _to_next_state[WSS_NOSTATE] = {
      wss_listing, //  wss_start = 0,
      wss_sleep, //  wss_sleep,
      wss_sleep, //  wss_listing,
      wss_sleep, //  wss_menusel,
      wss_manualwater, //  wss_manualwater,
      wss_sleep, //  wss_list,
      wss_sleep, //  wss_probe,
      wss_sleep, //  wss_autowater,
      wss_panic //  wss_panic,
    };
    ulong _timeout = 1000;

    ulong _state_to[WSS_NOSTATE] = {
      1000, //  wss_start = 0,
      5000, //  wss_sleep,
      2000, //  wss_listing,
      5000, //  wss_menusel,
      5000, //  wss_manualwater,
      2000, //  wss_list,
      1000, //  wss_probe,
      1000, //  wss_autowater,
      1000  //  wss_panic,
    };

    bool _lcd = false;

    void _to_transition(wss_type nextstate)
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

    void _list()
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
};

ulong last;

WaterSystemSM *pWSSM;


void nextButISR(void)
{
  pWSSM->nextBut->changed();
}

void okButISR(void)
{
  pWSSM->okBut->changed();
}


void setup() {
  Serial.begin(9600);
  panicLEDToggle();
  delay(500);

  pWSSM = new WaterSystemSM();
  pWSSM->Init();

  last = millis();

}

ulong timedelta(ulong refts, ulong now)
{
  if (now >= refts)
    return now - refts;
  else // overflow
    return ULONG_MAX - refts + now;
}

void loop() {

  ulong now = millis();
  ulong td = timedelta(last, now);

  if (td >= pWSSM->timeout()) {
    DEBUG("last, now, td = %lu, %lu, %lu", last, now, td);

    if (pWSSM->TOTransition(td)) {
      last = now;
    }
  }

  if (pWSSM->nextBut->isPressed(now)) {
    DEBUG("Next pressed");
    if (pWSSM->nextChangedTransition()) {
      last = now;
    }
  }

  if (pWSSM->okBut->isPressed(now)) {
    DEBUG("OK pressed");
    if (pWSSM->okChangedTransition()) {
      last = now;
    }
  }
}
