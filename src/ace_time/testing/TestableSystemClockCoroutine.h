/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TESTABLE_SYSTEM_CLOCK_COROUTINE_H
#define ACE_TIME_TESTABLE_SYSTEM_CLOCK_COROUTINE_H

#include <stdint.h>
#include "../clock/SystemClockCoroutine.h"
#include "TestableClockInterface.h"

namespace ace_time {
namespace testing {

/**
 * A version of SystemClockCoroutine that allows the clockMillis() function to
 * be manually set for testing purposes.
 */
using TestableSystemClockCoroutine =
    ace_time::clock::SystemClockCoroutineTemplate<
        ace_time::testing::TestableClockInterface,
        ace_time::testing::TestableClockInterface
    >;

}
}

#endif
