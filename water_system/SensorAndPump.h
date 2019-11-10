#ifndef SensorAndPump_H
#define SensorAndPump_H

#include <Arduino.h>

#include "ws_defs.h"

#define SENSOR_START_DELAY_MS 10
#define PUMP_ON_MS 200

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

    void ManualGiveWaterAndAdjustDry()
    {
      // Audo-adjust
      noInterrupts();
      int moistureNow = GetCurrentMoisture();
      // TODO: store more (3?) than 1 value and average all
      SetTooDry( (_dryValue + moistureNow) / 2);
      interrupts();

      GiveWater();
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
  {10, A3, 11},
};

#endif