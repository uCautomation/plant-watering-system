#ifndef SensorAndPump_H
#define SensorAndPump_H

#include <Arduino.h>

#include "ws_defs.h"
#include "WSRuntime.h"

#define SENSOR_START_DELAY_MS 10
#define PUMP_ON_MS            5000UL

static const char *noPercent = "-- ";

class SensorAndPump {
    private:
        bool _moduleIsUsed = true;

        int _vSensorPin, _sensorPin, _pumpCmdPin;
        int _dryValue, _lastMoisture;
        int _pumpOnMS;
        static const int _dryDeadBandDelta = 10;

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

        byte _dryPercentFromAbsValue(int analogValue)
        {
            assert_or_panic((analogValue >= 0) && (analogValue < (int)_analogReadSteps()));

            // maximum ADC representable value is (2^n - 1),
            // so the result below is always slightly < 100%
            return (byte)(100U * (uint16_t)analogValue / _analogReadSteps());
        }

        uint16_t _analogReadSteps()
        {
            return 2U << systemAnalogReadBits();
        }

        byte _dryPercentOnSampleNo(byte drySampleIndex)
        {
            // not used for now, will be used when multiple refs are stored
            (void)drySampleIndex;

            return _dryPercentFromAbsValue(_dryValue);
        }

        inline bool _lastMoistureIsTooDry()
        {
            return _lastMoisture + _dryDeadBandDelta < _dryValue;
        }

        int _readCurrentMoisture()
        {
            _sensorOn();
            delay(SENSOR_START_DELAY_MS);//wait for the sensor to stabilize
            _lastMoisture = analogRead(_sensorPin);//Read the SIG value form sensor
            _sensorOff();

            return _lastMoisture; //send current moisture value
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

        int getLastMoisture()
        {
            return _lastMoisture;
        }

        int getNormalizedDeltaToThreshold()
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

        byte getDryPercent()
        {
            return _dryPercentFromAbsValue(_dryValue);
        }

        byte getLastMoisturePercent()
        {
            return _dryPercentFromAbsValue(_lastMoisture);
        }

        char *getTooDryPercentAsStr(byte drySampleIndex)
        {
            if (drySampleIndex >= _maxDryValues) {
                return (char *)noPercent;
            }

            // TODO: workaround the fact we can't guarantee lifetimes?
            snprintf(_buf, _bufLen, "%.2d", _dryPercentOnSampleNo(drySampleIndex));

            // this should be safe since we have one buffer per SensorAndPump instance
            DEBUG("Dry %% >%s<", _buf);

            return _buf;
        }

        void setTooDry(int dryValue)
        {
            _dryValue = dryValue;
        }

        void giveWater()
        {
            if (isModuleUsed()) {
                digitalWrite(_pumpCmdPin, HIGH); // open valve to let water run
                delay(_pumpOnMS);
                digitalWrite(_pumpCmdPin, LOW); // close valve
            } else {
                DEBUG_P("Ignore unused plant watering cmd");
            }
        }

        void manualGiveWaterAndAdjustDry()
        {
            // Audo-adjust
            noInterrupts();
            int moistureNow = _readCurrentMoisture();
            // TODO: store more (3?) than 1 value and average all
            setTooDry( (_dryValue + moistureNow) / 2);
            interrupts();

            giveWater();
        }

        inline void setModuleUsed() { _moduleIsUsed = true; }

        inline void setModuleUnused() { _moduleIsUsed = false; }

        inline bool isModuleUsed() { return _moduleIsUsed; }

        bool tryAutoWater() {
            bool watered = false;
            if (_moduleIsUsed) {
                (void)_readCurrentMoisture();
                if (_lastMoistureIsTooDry()) {
                    giveWater();
                    watered = true;
                }
            }
            return watered;
        }
};

#endif
