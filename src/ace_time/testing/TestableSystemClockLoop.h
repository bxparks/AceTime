/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TESTABLE_SYSTEM_CLOCK_LOOP_H
#define ACE_TIME_TESTABLE_SYSTEM_CLOCK_LOOP_H

#include <stdint.h>
#include "../clock/SystemClockLoop.h"
#include "TestableClockInterface.h"

namespace ace_time {
namespace testing {

/**
 * A version of SystemClockLoop that allows the clockMillis() function to be
 * manually set for testing purposes.
 */
using TestableSystemClockLoop = clock::SystemClockLoopTemplate<
    TestableClockInterface
>;

}
}

#endif

