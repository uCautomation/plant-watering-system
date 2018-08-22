#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <limits.h>
#include <stdio.h>

#define _DEBUG_
#ifdef _DEBUG_
#define DEBUG(fmt, args...) do { \
  char buf[200]; \
  sprintf(buf, "DBG: " fmt, ##args); \
  Serial.println(buf); \
} while(0)

#else
#define DEBUG(fmt, args...)
#endif

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
  {2, A0, 3}, // D2 is Vsens, A0 = Sens, D3 is Valve cmd
  {4, A1, 5},
  {6, A2, 7},
  {7, A3, 9},
};

typedef enum {
  wss_start = 0,
  wss_active,
  wss_listing,
  wss_menusel,
  wss_manualwater,
  wss_list,
  wss_probe,
  wss_autowater,
  wss_panic,

  WSS_NOSTATE
} wss_type;

class WaterSystemSM {
  public:
    WaterSystemSM() {
      _state = wss_start;
    };
    void Init(void) {
        int error;

        Serial.begin(9600);
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

    ulong timeout() { return _timeout;}

  private:

    int _panic_led = 0;
    wss_type _state;
    wss_type _to_next_state[WSS_NOSTATE] = {
      wss_listing, //  wss_start = 0,
      wss_active, //  wss_active,
      wss_active, //  wss_listing,
      wss_active, //  wss_menusel,
      wss_manualwater, //  wss_manualwater,
      wss_active, //  wss_list,
      wss_active, //  wss_probe,
      wss_active, //  wss_autowater,
      wss_panic //  wss_panic,
    };
    ulong _timeout = 1000;

    ulong _state_to[WSS_NOSTATE] = {
      1000, //  wss_start = 0,
      5000, //  wss_active,
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
          _panic_led = !_panic_led;
          digitalWrite(LED_BUILTIN, _panic_led);
          if (_state != wss_start)
            lcd.setBacklight(0);
          break;
          ;;

        case wss_listing:
          _list();
          break;
          ;;

        case wss_active:
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
      lcd.setBacklight(255);lcd.home(); //lcd.clear();

      char buf[51] = ".    .    |    .    |    .    |    .    ";
      for( int i=0; i<MAX_MODULE_COUNT; i++) {
        int moist = sp[i].GetCurrentMoisture();
        sprintf(buf, "S%.1d=%d  ", i, moist);
        lcd.setCursor((i&0x1) * 8, i>>1);
        lcd.print(buf);
      }
    }
};

ulong last;

WaterSystemSM *pWSSM;

void setup() {

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
}
