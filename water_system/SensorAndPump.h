#ifndef SensorAndPump_H
#define SensorAndPump_H

#include <Arduino.h>

#include "ws_defs.h"

#define SENSOR_START_DELAY_MS 10
#define PUMP_ON_MS 200

class SensorAndPump {
  private:
    int _vSensorPin, _sensorPin, _pumpCmdPin;
    int _dryValue, _lastMoisture;
    int _pumpOnMS;

  public:
    SensorAndPump
    (
      int VsensorPin,
      int SensorPin,
      int PumpCmdPin,
      int DryValue=511,
      int pumpOnMS=PUMP_ON_MS
    )
      :
      _vSensorPin(VsensorPin),
      _sensorPin(SensorPin),
      _pumpCmdPin(PumpCmdPin),
      _dryValue(DryValue),
      _pumpOnMS(pumpOnMS)
      {
        pinMode(_vSensorPin, OUTPUT);
        // analog pins are ready for AnalogRead by default, so _sensorPin works out of the box
        pinMode(_pumpCmdPin, OUTPUT);

        _lastMoisture = _dryValue; // Assume on start the plant is watered; initialize the value
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

      return _lastMoisture; //send current moisture value
    }

    int GetLastMoisture()
    {
      return _lastMoisture;
    }

    int GetNormalizedDeltaToThreshold()
    {
      // We want to get a -9 to +9 range for the entire spectrum
      int delta = _lastMoisture - _dryValue;

      if (delta == 0) {
        return 0;

      } else if (delta < 0) {
        // normalize in the interval [0, _dryValue),
        // result must be negative
        return (delta * 9) / _dryValue;

      } else {
        // normalize in the interval (_dryValue, MAX_ADC_VAL)
        return (delta * 9) / (MAX_ADC_VALUE - _dryValue);

      }
    }

    void SetTooDry(int dryValue)
    {
      _dryValue = dryValue;
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
};

#endif
