#include "gtest/gtest.h"
#include "SensorAndPump.h"
#include "Arduino_mock.h"

#define tPinVSensor 4
#define tPinAnalogSensor 5
#define tPinVPump 6

TEST(SensorAndPump, Initial) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    EXPECT_TRUE(t->isModuleUsed());
}

TEST(SensorAndPump, ManualWaterOnce) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    // expect next analogRead to be 100
    setExpectAnalogRead(tPinAnalogSensor, 100);

    t->manualGiveWaterAndAdjustDry();

    EXPECT_EQ(100, t->getLastMoisture());
}

TEST(SensorAndPump, fullHumidity) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    int humidity = SEEN_SENSOR_WET_LIMIT;

    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();

    EXPECT_EQ(99, t->getDryPercent());
}

TEST(SensorAndPump, noHumidity) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    int humidity = SEEN_SENSOR_DRY_LIMIT;

    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, humidity);
    t->manualGiveWaterAndAdjustDry();

    EXPECT_EQ(0, t->getDryPercent());
}

TEST(SensorAndPump, halfHumidity) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    int halfHumidity = (SEEN_MIN_LIMIT + SEEN_MAX_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    int8_t delta_percent = 50 - t->getDryPercent();

    // accept only at most a 1% delta due to rounding errors
    EXPECT_GE(1, delta_percent);
    EXPECT_LE(0, delta_percent);
}

TEST(SensorAndPump, halfHumidity2) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    // default value is 511
    EXPECT_EQ(511, t->getLastMoisture());

    int halfHumidity = (SEEN_MIN_LIMIT + SEEN_MAX_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, SEEN_MIN_LIMIT);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, SEEN_MAX_LIMIT);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    int8_t delta_percent = 50 - t->getDryPercent();

    // accept only at most a 1% delta due to rounding errors
    EXPECT_GE(1, delta_percent);
    EXPECT_LE(0, delta_percent);
}

TEST(SensorAndPump, noAutoWateringIfAtTreshold) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    // default value is 511
    EXPECT_EQ(511, t->getLastMoisture());

    int halfHumidity = (SEEN_MIN_LIMIT + SEEN_MAX_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    // the system should read the exact treshold value
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);

    // sensor should be considered wet (due to _dryDeadBandDelta)
    EXPECT_FALSE(t->tryAutoWater());
}

TEST(SensorAndPump, autoWateringIfJustDry) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    // default value is 511
    EXPECT_EQ(511, t->getLastMoisture());

    int halfHumidity = (SEEN_MIN_LIMIT + SEEN_MAX_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    // TODO: _dryDeadBandDelta should be configurable
    const int DRY_OFFSET = halfHumidity / 2;

    // the system should read a dry value
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity - SENSOR_SIGN * DRY_OFFSET);

    // sensor should be considered dry
    EXPECT_TRUE(t->tryAutoWater());
}
