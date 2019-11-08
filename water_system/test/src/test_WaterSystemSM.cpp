#include "gtest/gtest.h"
#include "WaterSystemSM.h"

TEST(WaterSystemSM, Initial) {
  WaterSystemSM *t = new WaterSystemSM();

  EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
  WaterSystemSM *t = new WaterSystemSM(0ul);
  t->timeoutTransition(10000ul);

  EXPECT_EQ(wss_listing, t->State());

  //EXPECT_EQ(wss_sleep, t->State());
};
