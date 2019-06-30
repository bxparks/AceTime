/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_HEARTBEAT_COROUTINE_H
#define ACE_TIME_SYSTEM_CLOCK_HEARTBEAT_COROUTINE_H

#include <stdint.h>
#include "SystemClock.h"

namespace ace_time {
namespace clock {

/**
 * A coroutine that calls SystemClock.getNow() peridically. This must be
 * performed before the uint16_t timer in SystemClock overflows, i.e.
 * every 65535 milliseconds at a minimum. I recommend every 5000 millis, which
 * is the default.
 */
class SystemClockHeartbeatCoroutine: public ace_routine::Coroutine {
  public:
    /**
     * Constructor.
     *
     * @param systemClock the underlying SystemClock
     * @param heartbeatPeriodMillis milliseconds between calls to getNow()
     *    (default 5000)
     */
    SystemClockHeartbeatCoroutine(SystemClock& systemClock,
        uint16_t heartbeatPeriodMillis = 5000):
      mSystemClock(systemClock),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    int runCoroutine() override {
      COROUTINE_LOOP() {
        mSystemClock.getNow();
        COROUTINE_DELAY(mHeartbeatPeriodMillis);
      }
    }

  private:
    SystemClock& mSystemClock;
    uint16_t const mHeartbeatPeriodMillis;
};

}
}

#endif
