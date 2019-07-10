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
 * A coroutine that calls SystemClock::getNow() every heartbeatPeriodMillis
 * interval. This is required because the SystemClock uses an internal uint16_t
 * counter which can overflow every 65535 milliseconds. Calling
 * SystemClock::getNow() resets this internal counter.
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
    explicit SystemClockHeartbeatCoroutine(SystemClock& systemClock,
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
    // disable copy constructor and assignment operator
    SystemClockHeartbeatCoroutine(const SystemClockHeartbeatCoroutine&) =
        delete;
    SystemClockHeartbeatCoroutine& operator=(
        const SystemClockHeartbeatCoroutine&) = delete;

    SystemClock& mSystemClock;
    uint16_t const mHeartbeatPeriodMillis;
};

}
}

#endif
