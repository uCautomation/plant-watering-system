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

    int humidity = SEEN_SENSOR_DRY_LIMIT;
    #ifndef SENSOR_USES_DIRECT_PROPORTION
    humidity = SEEN_SENSOR_DRY_LIMIT - humidity;
    #endif

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

    int humidity = SEEN_SENSOR_WET_LIMIT;
    #ifndef SENSOR_USES_DIRECT_PROPORTION
    humidity = SEEN_SENSOR_DRY_LIMIT;
    #endif

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

    // default value is 511
    EXPECT_EQ(511, t->getLastMoisture());

    int halfHumidity = (SEEN_SENSOR_WET_LIMIT + SEEN_SENSOR_DRY_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    EXPECT_EQ(50, t->getDryPercent());
}

// TODO: fix test for inverse sensors
TEST(DISABLED_SensorAndPump, halfHumidity) {
    SensorAndPump *t = new SensorAndPump(
        tPinVSensor, tPinAnalogSensor, tPinVPump);

    // default value is 511
    EXPECT_EQ(511, t->getLastMoisture());

    int halfHumidity = (SEEN_SENSOR_WET_LIMIT + SEEN_SENSOR_DRY_LIMIT) / 2;

    setExpectAnalogRead(tPinAnalogSensor, SEEN_SENSOR_WET_LIMIT);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, SEEN_SENSOR_DRY_LIMIT);
    t->manualGiveWaterAndAdjustDry();
    setExpectAnalogRead(tPinAnalogSensor, halfHumidity);
    t->manualGiveWaterAndAdjustDry();

    EXPECT_EQ(50, t->getDryPercent());
}
