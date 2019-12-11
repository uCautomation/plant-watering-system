#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "WaterSystemSM.h"

TEST(WaterSystemSM, Initial) {
  WaterSystemSM *t = new WaterSystemSM(1ul);

  EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
  WaterSystemSM *t = new WaterSystemSM(0ul);
  t->stateUpdated(10000ul);

  EXPECT_EQ(wss_list_all, t->State());

  //EXPECT_EQ(wss_sleep, t->State());
};
