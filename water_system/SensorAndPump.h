#ifndef SensorAndPump_H
#define SensorAndPump_H

#include <Arduino.h>

#include "ws_defs.h"
#include "WSRuntime.h"

class LastMoistures {
    private:
        static const byte _SIZE = MAX_DRY_VALUES_PER_MODULE;
        int _moistures[_SIZE];
        byte _index = _SIZE - 1;

    public:

        LastMoistures() {
            LastMoistures(0U);
        }

        LastMoistures(int value) {
            for ( byte i=0; i<_SIZE; i++) {
                _index = (_index+1) % _SIZE;
                _moistures[_index] = value;
            }
        }

        void add(int value) {
            _index = (_index+1) % _SIZE;
            _moistures[_index] = value;
        }

        int getPrevious(byte offset) {
            return _moistures[(_index - (offset % _SIZE) + _SIZE) % _SIZE];
        }

        int getLast() {
            return getPrevious(0U);
        }

        // int count(void) { return _SIZE; }

        int average() {
            int s = 0;
            for (byte i = 0U; i<_SIZE; i++) {
                s += _moistures[i];
            }
            return s / _SIZE;
        }

        void setAll(int value) {
            for (byte i = 0; i < _SIZE; i++)
                _moistures[i] = value;
        }
};

#define SENSOR_START_DELAY_MS 20
#define PUMP_ON_MS            5000UL

// it seems my relay has inverse command
#define PUMP_OFF              HIGH
#define PUMP_ON               LOW

static const char *noPercent = "-- ";

class SensorAndPump {
    private:
        bool _moduleIsUsed = true;

        int _vSensorPin, _sensorPin, _pumpCmdPin;
        int _dryValue, _lastMoisture;
        LastMoistures _dryMoistures;
        int _pumpOnMS;
        static const int _dryDeadBandDelta = 10;

        static const byte _maxDryValues = MAX_DRY_VALUES_PER_MODULE;
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

        int8_t _dryPercentFromAbsValue(int analogValue)
        {
            assert_or_panic((analogValue >= 0) && (analogValue < (int)_analogReadSteps()));

            // maximum ADC representable value is (2^n - 1),
            // so the result below is always slightly < 100%
            int8_t mapped = (int8_t)map(analogValue, SEEN_SENSOR_DRY_LIMIT, SEEN_SENSOR_WET_LIMIT, 0, 100);
            return constrain(mapped, 0, 99);
        }

        uint16_t _analogReadSteps()
        {
            return 2U << systemAnalogReadBits();
        }

        byte _dryPercentOnSampleNo(byte drySampleIndex)
        {
            return (byte)_dryPercentFromAbsValue(_dryMoistures.getPrevious(drySampleIndex));
        }

        inline bool _lastMoistureIsTooDry()
        {
            return SENSOR_SIGN * (_dryValue - _lastMoisture) > _dryDeadBandDelta;
        }

        int _readCurrentMoisture()
        {
            _sensorOn();
            delay(SENSOR_START_DELAY_MS);//wait for the sensor to stabilize
            _lastMoisture = analogRead(_sensorPin);//Read the SIG value form sensor
            _sensorOff();

            DEBUG(" moisture(%p): %d", this, _lastMoisture);

            return _lastMoisture; //send current moisture value
        }

        void _setTooDry(int dryValue)
        {
            _dryMoistures.add(dryValue);
            _dryValue = _dryMoistures.average();
        }

        void _giveWater()
        {
            if (isModuleUsed()) {
                digitalWrite(_pumpCmdPin, PUMP_ON); // open valve to let water run
                delay(_pumpOnMS);
                digitalWrite(_pumpCmdPin, PUMP_OFF); // close valve
            } else {
                DEBUG_P("Ignore unused plant watering cmd");
            }
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
            digitalWrite(_pumpCmdPin, PUMP_OFF);

            _dryMoistures.setAll(DryValue);

            _lastMoisture = _dryValue; // Assume on start the plant is watered; initialize the value

        }

        int getLastMoisture()
        {
            return _lastMoisture;
        }

        int8_t getNormalizedDeltaToThreshold()
        {
            // We want to get a -9 to +9 range for the entire spectrum
            int delta = SENSOR_SIGN * (_lastMoisture - _dryValue);

            int mapped = map(delta, -MAX_ADC_VALUE, MAX_ADC_VALUE, -9, +9);
            return (int8_t)constrain(mapped, -9, +9);
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
            DEBUG_P("Dry %: "); DEBUG("%s", _buf);

            return _buf;
        }

        void manualGiveWaterAndAdjustDry()
        {
            // Auto-adjust
            noInterrupts();
            int moistureNow = _readCurrentMoisture();
            _setTooDry(moistureNow);
            interrupts();

            _giveWater();
        }

        inline void setModuleUsed() { _moduleIsUsed = true; }

        inline void setModuleUnused() { _moduleIsUsed = false; }

        inline void toggleModuleUsage() { _moduleIsUsed = !_moduleIsUsed; }

        inline bool isModuleUsed() { return _moduleIsUsed; }

        bool tryAutoWater() {
            bool watered = false;
            if (_moduleIsUsed) {
                (void)_readCurrentMoisture();
                if (_lastMoistureIsTooDry()) {
                    _giveWater();
                    watered = true;
                }
            }
            return watered;
        }

        void setValues(int referenceValues[MAX_DRY_VALUES_PER_MODULE]) {
            for (byte i=0; i<MAX_DRY_VALUES_PER_MODULE; i++) {

                _lastMoisture = referenceValues[i];

                _dryMoistures.add(_lastMoisture);
            };

            _dryValue = _dryMoistures.average();

        }

        int getDryAbsValue(byte index) {
            return _dryMoistures.getPrevious(index % MAX_DRY_VALUES_PER_MODULE);
        }

        void resetCalibration() {
            _dryValue = SEEN_SENSOR_DRY_LIMIT;
            _dryMoistures.setAll(_dryValue);
        }
};

#endif
