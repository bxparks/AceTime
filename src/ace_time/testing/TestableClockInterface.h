/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
*/

#ifndef ACE_TIME_TESTABLE_CLOCK_INTERFACE_H
#define ACE_TIME_TESTABLE_CLOCK_INTERFACE_H

#include <stdint.h>

namespace ace_time {
namespace testing {

/**
 * A version of ace_time::hw::ClockInterface that provides a layer of
 * indirection to Arduino millis() for testing purposes.
 */
class TestableClockInterface {
  public:
    /** Get the current millis. */
    static unsigned long millis() { return sMillis; }

    /** Set the current millis. */
    static void setMillis(unsigned long ms) { sMillis = ms; }

  public:
    static unsigned long sMillis;
};

}
}

#endif
