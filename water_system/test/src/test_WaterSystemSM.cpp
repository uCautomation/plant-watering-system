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

    WaterSystemSM *t = new WaterSystemSM(0UL, &mockOkBut, &mockNextBut);
    t->stateUpdated(1000UL);

    {
        // still in sleep before initial timeouot
        EXPECT_EQ(false, t->stateUpdated(2000UL));
        wss_type sleep_state = t->State();
        EXPECT_EQ(wss_start, sleep_state);
    }


    mockOkBut.tAppendExpectPush(true); // simulate button pressed
    // .. when in sleep
    EXPECT_EQ(true, t->stateUpdated(3500UL));

    EXPECT_EQ(wss_list_all, t->State());
};
