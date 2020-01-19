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

#define EXPECTED_START_TIMEOUT 1000UL

TEST(WaterSystemSM, Initial) {

    WaterSystemSM *t = new WaterSystemSM(0UL);

    EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {

    WaterSystemSM *t = new WaterSystemSM(0UL);

    // immediately time-out from start into wss_list_all
    EXPECT_EQ(true, t->stateUpdated(EXPECTED_START_TIMEOUT));
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, ListAllRemainsWithoutTimeout) {

    WaterSystemSM *t = new WaterSystemSM(0UL);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // immediately time-out from start into wss_list_all

    EXPECT_EQ(false,
              t->stateUpdated(
                  EXPECTED_START_TIMEOUT + (timeInMilli(SleepTimeOut) >> 1))); // timeout is one milli later
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, SleepOnListAllTimeOut) {

    WaterSystemSM *t = new WaterSystemSM(0UL);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all

    // check we're in sleep after timeout
    EXPECT_EQ(true,
              t->stateUpdated(
                  EXPECTED_START_TIMEOUT
                  + timeInMilli(SleepTimeOut)));
    EXPECT_EQ(wss_sleep, t->State());
};

TEST(WaterSystemSM, OnOkInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) + EXPECTED_START_TIMEOUT); // timeout, go to sleep

    mockOkBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) + 2UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) + EXPECTED_START_TIMEOUT); // timeout, go to sleep

    mockNextBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) + 2UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInListAllGoesToSysStatus) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all

    mockNextBut.tAppendExpectPush(true); // simulate Next pressed before sleep
    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) >> 2));
    EXPECT_EQ(wss_sys_status, t->State()); // ... sends us in sys status
};

TEST(WaterSystemSM, OnNextInSysStatusGoesToCtrlAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 2);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status

    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) >> 2));
    EXPECT_EQ(wss_ctrl_all, t->State()); // ... sends us in ctrl all
};

TEST(WaterSystemSM, OnNextInCtrlAllGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 2);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 1);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in ctrl all

    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) >> 2));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list all
};

// Menus testing
TEST(WaterSystemSM, OnOkInListAllGoesToMenuAllX) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep

    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) >> 2));
    EXPECT_EQ(wss_menu_all_x, t->State()); // ... sends us in MenuAllCLose state
};

TEST(WaterSystemSM, OnOkInMenuAllXGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 2); // going from list_all to MenuAllX
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    EXPECT_EQ(true, t->stateUpdated(timeInMilli(SleepTimeOut) >> 1));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us back in list_all state
};

/// start -> list_all -OK-> wss_menu_all_x -Next-> wss_menu_all_p1 -OK-> wss_list_one(_p1)
TEST(WaterSystemSM, OnNextOkInListAllMenuGoesToListOne) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(EXPECTED_START_TIMEOUT); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // Ok -> go in menu
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 2); // going from list_all to MenuAllX
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed (goto first)
    (void)t->stateUpdated(timeInMilli(SleepTimeOut) >> 1); // move in menu listall
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    EXPECT_EQ(true, t->stateUpdated(3 * (timeInMilli(SleepTimeOut) >> 2)));
    EXPECT_EQ(wss_list_one, t->State()); // ... sends us into list one (first sensor) state
};
