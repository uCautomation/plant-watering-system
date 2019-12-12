#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "WaterSystemSM.h"
#include "ButtonWS.h"

using ::testing::Return;
using ::testing::_;

class MockButtonWS : public ButtonWS::ButtonWS {
    public:
        MockButtonWS(int pin, isr butChISR) : ButtonWS::ButtonWS(pin, butChISR) {}
        MOCK_METHOD(void, changed, ());
        MOCK_METHOD(bool, isPressed, (ulong now));
};



TEST(WaterSystemSM, Initial) {

    WaterSystemSM *t = new WaterSystemSM(1UL);

    EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
    WaterSystemSM *t = new WaterSystemSM(0UL);
    t->stateUpdated(10000UL);

    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, FromSleepOKListsAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    EXPECT_CALL(mockOkBut, isPressed(_))
        .Times(2)
        .WillOnce(Return(false))
        .WillOnce(Return(true));


    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    t->stateUpdated(10000UL);

    /*wss_list_all*/ (void)t->State();
    t->stateUpdated(1000UL);

    EXPECT_EQ(wss_list_all, t->State());
};
