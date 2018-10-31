#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <limits.h>
#include <stdio.h>

#include "ws_types.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"

#define DEBUG_ON
#include "DebugWS.h"

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

    void SetTooDry(int dryValue)
    {
      _dryValue = dryValue;
    }

    void SetTooWet(int wetValue)
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


ulong timedelta(ulong refts, ulong now);

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
      byte s = digitalRead(_pin);
      //DEBUG("%s", s == LOW ? "LOW" : "HIGH");

      ulong td = timedelta(_milli, now);
      if (td < _debounceDelay) return false;
      //DEBUG("Debounced");

      //s = digitalRead(_pin);
      bool pressed = (s == LOW);

      //DEBUG("B%d (state=%d): Debounced, %spressed and %schanged!", _pin, s, pressed ? "" : "NOT ", s != _state ? "" : "NOT " );
      //DEBUG("ButtonWS %d : _lastmilli=%lu _milli=%lu, now=%lu", _pin, _lastmilli, _milli, now);

      _lastmilli = _milli;
      _state = s;

      return pressed;
    }


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
