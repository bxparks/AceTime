/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
*/

#ifndef ACE_TIME_CLOCK_INTERFACE_H
#define ACE_TIME_CLOCK_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>

namespace ace_time {
namespace hw {

/**
 * A utility class that provides a layer of indirection to the Arduino clock
 * functions (millis() and potentially micros() in the future). Not to be
 * confused with the `ace_clock::clock::Clock` base class.
 *
 * This indirection allows injection of a different ClockInterface for testing
 * purposes. Since this class uses non-virtual, static functions, the compiler
 * will optimize away the function call.
 */
class ClockInterface {
  public:
    /** Get the current millis. */
    static unsigned long millis() { return ::millis(); }
};

}
}

#endif
