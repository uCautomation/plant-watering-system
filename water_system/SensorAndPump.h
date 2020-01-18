#ifndef SensorAndPump_H
#define SensorAndPump_H

#include <Arduino.h>

#include "ws_defs.h"
#include "WSRuntime.h"

#define SENSOR_START_DELAY_MS 10
#define PUMP_ON_MS            200

static const char *noPercent = "-- ";

class SensorAndPump {
    private:
        bool _moduleIsUsed = true;

        int _vSensorPin, _sensorPin, _pumpCmdPin;
        int _dryValue, _lastMoisture;
        int _pumpOnMS;

        static const byte _maxDryValues = 1;
        static const byte _maxPercentStrLen = 4;
        constexpr static const byte _bufLen = _maxDryValues * _maxPercentStrLen + 1;
        char _buf[_bufLen] = { 0 };

        void _sensorOn(void)
        {
            digitalWrite(_vSensorPin, HIGH);//turn sensor "On"
        }

        void _sensorOff(void)
        {
            digitalWrite(_vSensorPin, LOW);//turn sensor "Off"
        }

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

        int GetCurrentMoisture()
        {
            this->_sensorOn();
            delay(SENSOR_START_DELAY_MS);//wait for the sensor to stabilize
            _lastMoisture = analogRead(_sensorPin);//Read the SIG value form sensor
            _sensorOff();

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

        byte _dryPercent(byte saneDryValue) {
            // not used for now
            (void)saneDryValue;

            // maximum ADC representable value is (2^n - 1),
            // so the result below is always slightly < 100%
            return (byte)(100 * _dryValue / (2 << systemAnalogReadBits()));
        }

        char *GetTooDryPercent(byte dryValueRefIndex)
        {
            if (dryValueRefIndex >= _maxDryValues) {
                return (char *)noPercent;
            }

            // TODO: workaround the fact we can't guarantee lifetimes?
            snprintf(_buf, _bufLen, "%.2d", _dryPercent(dryValueRefIndex));

            // this should be safe since we have one buffer per SensorAndPump instance
            DEBUG(" Too dry percent >%s<", _buf);

            return _buf;
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

        inline void setModuleUsed() { _moduleIsUsed = true; }

        inline void setModuleUnused() { _moduleIsUsed = false; }

        inline bool isModuleUsed()
        {
            return _moduleIsUsed;
        }
};

#endif
