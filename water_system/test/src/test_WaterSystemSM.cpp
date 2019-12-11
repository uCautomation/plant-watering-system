#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "WaterSystemSM.h"



TEST(WaterSystemSM, Initial) {

    WaterSystemSM *t = new WaterSystemSM(1UL);

    EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
    WaterSystemSM *t = new WaterSystemSM(0UL);
    t->stateUpdated(10000UL);

    EXPECT_EQ(wss_list_all, t->State());
};
