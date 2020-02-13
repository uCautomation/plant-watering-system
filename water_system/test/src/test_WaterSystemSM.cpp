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

void auxPutWSSMInListAllState(WaterSystemSM &pWssm, testTimeMilli &ms)
{
    (void)pWssm.stateUpdated(ms.tickAndGet(startTimeOutMilli)); // going from start into list_all
}

TEST(WaterSystemSM, SleepOnListAllTimeOut) {
    testTimeMilli ms;
    WaterSystemSM *t;

    t = new WaterSystemSM(ms.get());
    auxPutWSSMInListAllState(*t, ms);

    // check we're in sleep after timeout
    ms.tick(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_sleep, t->State());
};

void auxPutWSSMInSleep(WaterSystemSM &wssm, testTimeMilli &ms)
{
    auxPutWSSMInListAllState(wssm, ms);
    (void)wssm.stateUpdated(ms.tickAndGet(sleepTimeOutMillis()));
}

TEST(WaterSystemSM, OnOkInSleepGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInSleep(*t, ms);

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
    auxPutWSSMInSleep(*t, ms);

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
    auxPutWSSMInListAllState(*t, ms);

    ms.tickUpTo(sleepTimeOutMillis());
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed before sleep

    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_sys_status, t->State()); // ... sends us in sys status
};

void auxPutWSSMInSysStatusState(WaterSystemSM &wssm, MockButtonWS &mockNextBut, testTimeMilli &ms)
{
    auxPutWSSMInListAllState(wssm, ms);
    mockNextBut.tAppendExpectPush(true);
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

TEST(WaterSystemSM, OnNextInSysStatusGoesToCtrlAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInSysStatusState(*t, mockNextBut, ms);

    (void)t->stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
    mockNextBut.tAppendExpectPush(true); // simulate Next pressed in sys status

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_ctrl_all, t->State()); // ... sends us in ctrl all
};

void auxPutWSSMInCtrlAll(WaterSystemSM &wssm, MockButtonWS &mockNextBut, testTimeMilli &ms)
{
    auxPutWSSMInSysStatusState(wssm, mockNextBut, ms);
    mockNextBut.tAppendExpectPush(true);
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

TEST(WaterSystemSM, OnNextInCtrlAllGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInCtrlAll(*t, mockNextBut, ms);

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
    auxPutWSSMInListAllState(*t, ms);

    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_menu_all_x, t->State()); // ... sends us in MenuAllCLose state
};

void auxPutWSSMInMenuAllXState(WaterSystemSM &wssm, MockButtonWS &mockOkBut, testTimeMilli &ms)
{
    auxPutWSSMInListAllState(wssm, ms);
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

TEST(WaterSystemSM, OnOkInMenuAllXGoesToListAll) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInMenuAllXState(*t, mockOkBut, ms);

    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_all, t->State()); // ... sends us back in list_all state
};

TEST(WaterSystemSM, OnNextInMenuAllXGoesToMenuAllP1) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInMenuAllXState(*t, mockOkBut, ms);

    mockNextBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_menu_all_p1, t->State()); // ... sends us back in list_all state
};

void auxPutWSSMInMenuAllP1State(WaterSystemSM &wssm, MockButtonWS &mockNextBut, MockButtonWS &mockOkBut, testTimeMilli &ms)
{
    auxPutWSSMInMenuAllXState(wssm, mockOkBut, ms);
    mockNextBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

/// wss_menu_all_p1 -OK-> wss_list_one(_p1)
TEST(WaterSystemSM, OnOkInMenuAllP1GoesToListOneP1) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInMenuAllP1State(*t, mockNextBut, mockOkBut, ms);

    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed on 'X'

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_list_one_p1, t->State()); // ... sends us into list one (first sensor) state
};

void auxPutWSSMInListOneP1(WaterSystemSM &wssm, MockButtonWS &mockNextBut, MockButtonWS &mockOkBut, testTimeMilli &ms)
{
    auxPutWSSMInMenuAllP1State(wssm, mockNextBut, mockOkBut, ms);
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

/// wss_list_one(_p1) -OK-> wss_menu_one_x
TEST(WaterSystemSM, OnOkInListOneP1GoesToMenuOneX) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInListOneP1(*t, mockNextBut, mockOkBut, ms);

    mockOkBut.tAppendExpectPush(true); // Ok -> go in menu

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_menu_one_x, t->State());
}

void auxPutWSSMInMenuOneXState(WaterSystemSM &wssm, MockButtonWS &mockNextBut, MockButtonWS &mockOkBut, testTimeMilli &ms)
{
    auxPutWSSMInListOneP1(wssm, mockNextBut, mockOkBut, ms);
    mockOkBut.tAppendExpectPush(true); // simulate Ok pressed before sleep
    (void)wssm.stateUpdated(ms.tickAndGet(halfOfSleepTimeoutMillis()));
}

/// wss_menu_one_x(_p1) -Next-> wss_menu_one_water
TEST(WaterSystemSM, OnNextInMenuOneXGoesToMenuOneWater) {
    testTimeMilli ms;
    MockButtonWS mockOkBut = MockButtonWS(okButPin, okButISR);
    MockButtonWS mockNextBut = MockButtonWS(nextButPin, nextButISR);
    WaterSystemSM *t = new WaterSystemSM(ms.get(), &mockOkBut, &mockNextBut);
    auxPutWSSMInMenuOneXState(*t, mockNextBut, mockOkBut, ms);

    mockNextBut.tAppendExpectPush(true); // simulate Next pressed (goto first)

    ms.tickUpTo(sleepTimeOutMillis());
    EXPECT_EQ(true, t->stateUpdated(ms.get()));
    EXPECT_EQ(wss_menu_one_water, t->State()); // ... sends us into list one (first sensor) state
};
