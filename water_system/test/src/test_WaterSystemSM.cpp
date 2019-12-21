#include "gtest/gtest.h"
#include "WaterSystemSM.h"
#include "ButtonWS.h"
#include <vector>

using std::vector;

class MockButtonWS : public ButtonWS
{
    private:
        std::vector<bool> _pressed_preseeds;
        bool _default_pressed = false;

    public:
        MockButtonWS(int pin, isr butChISR)
            : ButtonWS(pin, butChISR) {
            _pressed_preseeds = {};
        }

        void tAppendExpectPush(bool expect) {
            this->_pressed_preseeds.push_back(expect);
        }

        void tExtendExpectPush(std::vector<bool> expects) {
            auto old_len = this->_pressed_preseeds.end();

            this->_pressed_preseeds.insert(
                old_len,
                expects.begin(),
                expects.end() );
        }

        bool isPressed(ulong now) override {
            (void)now;
            bool res = this->_default_pressed;

            std::vector<bool> *pExpResults = &(this->_pressed_preseeds);

            if (pExpResults->size() > 0U) {
                res = pExpResults->back();
                pExpResults->pop_back();
            };

            return res;
        }
};

TEST(WaterSystemSM, Initial) {

    WaterSystemSM *t = new WaterSystemSM(0UL);

    EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
    WaterSystemSM *t = new WaterSystemSM(0UL);

    // immediately time-out from start into wss_list_all
    EXPECT_EQ(true, t->stateUpdated(1UL));
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, ListAllRemainsWithoutTimeout) {
    WaterSystemSM *t = new WaterSystemSM(0UL);

    // immediately time-out from start into wss_list_all
    (void)t->stateUpdated(1UL);

    EXPECT_EQ(false, t->stateUpdated(5000UL)); // timeout is at 5001
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, SleepOnListAllTimeOut) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    t->stateUpdated(1UL); // going from start into list_all
    (void)t->State();

    // still in list_all before initial timeout
    {
        EXPECT_EQ(false, t->stateUpdated(5000UL)); // timeout should be at 5001
        wss_type c_state = t->State();
        EXPECT_EQ(wss_list_all, c_state);
    }

    // check we're in sleep after timeout
    EXPECT_EQ(true, t->stateUpdated(5001UL));
    EXPECT_EQ(wss_sleep, t->State());
};

TEST(WaterSystemSM, OnOkInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    t->stateUpdated(1UL); // going from start into list_all
    (void)t->State();
    // still in list_all before initial timeout
    (void)t->stateUpdated(5001UL); // timeout should be at 5001, go to sleep

    mockOkBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(5002UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    t->stateUpdated(1UL); // going from start into list_all
    (void)t->State();
    // still in list_all before initial timeout
    (void)t->stateUpdated(5001UL); // timeout should be at 5001, go to sleep

    mockNextBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(5002UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};
