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

enum {
    startTimeOutMilli = 1000UL,
    testOneMilli = 1UL
};

class testTimeMilli {
    private:
        ulong _millis = 0UL;

    public:
        void tick(ulong deltaMs) {
            _millis += deltaMs;
        }

        void tick() {
            tick(testOneMilli);
        }

        void tickUpTo(ulong deltaMs) {
            assert(deltaMs > 0UL);
            tick(deltaMs - 1UL);
        }

        ulong get() {
            return _millis;
        }

        ulong tickAndGet(ulong deltaMs) {
            tick(deltaMs);
            return get();
        }

        ulong tickAndGet() {
            return tickAndGet(testOneMilli);
        }
};

constexpr ulong sleepTimeOutMillis() {
    return timeInMilli(SleepTimeOut);
}

constexpr ulong halfOfSleepTimeoutMillis() {
    return sleepTimeOutMillis() >> 1;
}

TEST(WaterSystemSM, Initial) {

    WaterSystemSM *t = new WaterSystemSM(0UL);

    EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {

    WaterSystemSM *t = new WaterSystemSM(0UL);

    // immediately time-out from start into wss_list_all
    EXPECT_EQ(true, t->stateUpdated(startTimeOutMilli));
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, ListAllRemainsWithoutTimeout) {
    testTimeMilli ms;

    WaterSystemSM *t = new WaterSystemSM(ms.get());
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // immediately time-out from start into wss_list_all

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(false, t->stateUpdated(ms.get())); // timeout is one milli later
    EXPECT_EQ(wss_list_all, t->State());
};

TEST(WaterSystemSM, SleepOnListAllTimeOut) {
    testTimeMilli ms;

    WaterSystemSM *t = new WaterSystemSM(ms.get());
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all

    // check we're in sleep after timeout
    ms.tick(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_sleep, t->State());
};

TEST(WaterSystemSM, OnOkInSleepGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    (void)t->stateUpdated(ms.tickAndGet(sleepTimeOutMillis())); // timeout, go to sleep

    ms.tick();
    mockOkBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInSleepGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    (void)t->stateUpdated(ms.tickAndGet(sleepTimeOutMillis())); // timeout, go to sleep

    ms.tick();
    mockNextBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list_all
};

TEST(WaterSystemSM, OnNextInListAllGoesToSysStatus) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all

    ms.tickUpTo(sleepTimeOutMillis());
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed before sleep
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_sys_status, t->State()); // ... sends us in sys status
};

TEST(WaterSystemSM, OnNextInSysStatusGoesToCtrlAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_ctrl_all, t->State()); // ... sends us in ctrl all
};

TEST(WaterSystemSM, OnNextInCtrlAllGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in list all
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in ctrl all

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us in list all
};

// Menus testing
TEST(WaterSystemSM, OnOkInListAllGoesToMenuAllX) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_menu_all_x, t->State()); // ... sends us in MenuAllCLose state
};

TEST(WaterSystemSM, OnOkInMenuAllXGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis())); // going from list_all to MenuAllX
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us back in list_all state
};

/// start -> list_all -OK-> wss_menu_all_x -Next-> wss_menu_all_p1 -OK-> wss_list_one(_p1)
TEST(WaterSystemSM, OnNextOkInListAllMenuGoesToListOneP1) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);

    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    (void)t->stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
    mockOkBut.tAppendExpectPush(true); // Ok -> go in menu
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis())); // going from list_all to MenuAllX
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed (goto first)
    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis())); // move in menu listall
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_one_p1, t->State()); // ... sends us into list one (first sensor) state
};
