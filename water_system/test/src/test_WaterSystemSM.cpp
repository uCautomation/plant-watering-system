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
    (void)t->stateUpdated(1UL); // immediately time-out from start into wss_list_all

    EXPECT_EQ(false, t->stateUpdated(SleepTimeOut)); // timeout is at 5001
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, SleepOnListAllTimeOut) {

    WaterSystemSM *t = new WaterSystemSM(0UL);
    (void)t->stateUpdated(1UL); // going from start into list_all
    // still in list_all before initial timeout
    (void)t->stateUpdated(SleepTimeOut); // timeout should be at 5001

    // check we're in sleep after timeout
    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut + 1UL));
    EXPECT_EQ(wss_sleep, t->State());
};

TEST(WaterSystemSM, OnOkInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all
    (void)t->stateUpdated(SleepTimeOut + 1UL); // timeout, go to sleep

    mockOkBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut + 2UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInSleepGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all
    (void)t->stateUpdated(SleepTimeOut + 1UL); // timeout, go to sleep

    mockNextBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut + 2UL));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInListAllGoesToSysStatus) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all

    mockNextBut.tAppendExpectPush(true); // simulate Next pressed before sleep
    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut >> 2));
    EXPECT_EQ(wss_sys_status, t->State()); // ... sends us in sys status
};

TEST(WaterSystemSM, OnNextInSysStatusGoesToCtrlAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(SleepTimeOut >> 2);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status

    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut >> 2));
    EXPECT_EQ(wss_ctrl_all, t->State()); // ... sends us in ctrl all
};

TEST(WaterSystemSM, OnNextInCtrlAllGoesToListAll) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(SleepTimeOut >> 2);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status
    (void)t->stateUpdated(SleepTimeOut >> 1);
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in ctrl all

    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut >> 2));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list all
};

// Menus testing
TEST(WaterSystemSM, OnOkInListAllGoesToMenuSel) {
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(1UL); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep

    EXPECT_EQ(true, t->stateUpdated(SleepTimeOut >> 2));
    EXPECT_EQ(wss_menusel, t->State()); // ... sends us in menusel state
    // TODO: test menu is ListAll's menu
};
