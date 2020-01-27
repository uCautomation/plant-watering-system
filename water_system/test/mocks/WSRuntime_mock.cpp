#include "gtest/gtest.h"

static bool expectPanic = false;

void testExpectPanic() {
    expectPanic = true;
}

// FIXME: the tested code assumes this does not return;
// Maybe this should be a macro which returns(?)
// The panic is probably nested, so we would rather unwind?
// Maybe we should use EXPECT_ANY_THROW (tested code has no exceptions)?
void system_panic_no_return() {
    EXPECT_TRUE(expectPanic);
    expectPanic = false;
}

void assert_or_panic(bool condition)
{
    // TODO: is this enough?
    EXPECT_TRUE(condition);
}
