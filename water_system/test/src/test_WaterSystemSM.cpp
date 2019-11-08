#include "gtest/gtest.h"
#include "WaterSystemSM.h"

TEST(WaterSystemSM, Initial) {
  WaterSystemSM *t = new WaterSystemSM();

  EXPECT_EQ(wss_start, t->State());
};

TEST(WaterSystemSM, TimeoutFromInitial) {
  WaterSystemSM *t = new WaterSystemSM();
  t->Init(); // wss_start - why?
  t->timeoutTransition(10000);

  EXPECT_EQ(wss_listing, t->State());

  //EXPECT_EQ(wss_sleep, t->State());
};
