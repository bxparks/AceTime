#ifndef ACE_TIME_SYSTEM_TIME_HEARTBEAT_COROUTINE_H
#define ACE_TIME_SYSTEM_TIME_HEARTBEAT_COROUTINE_H

#include <stdint.h>
#include "SystemTimeKeeper.h"

namespace ace_time {

/**
 * A coroutine that calls SystemTimeKeeper.getNow() peridically. This must be
 * performed before the uint16_t timer in SystemTimeKeeper overflows, i.e.
 * every 65535 milliseconds at a minimum. I recommend every 5000 millis, which
 * is the default.
 */
class SystemTimeHeartbeatCoroutine: public ace_routine::Coroutine {
  public:
    /**
     * Constructor.
     *
     * @param systemTimeKeeper reference to the SystemTimeKeeper
     * @param heartbeatPeriodMillis milliseconds between calls to getNow()
     */
    SystemTimeHeartbeatCoroutine(SystemTimeKeeper& systemTimeKeeper,
        uint16_t heartbeatPeriodMillis = 5000):
      mSystemTimeKeeper(systemTimeKeeper),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    int runCoroutine() override {
      COROUTINE_LOOP() {
        mSystemTimeKeeper.getNow();
        COROUTINE_DELAY(mHeartbeatPeriodMillis);
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mHeartbeatPeriodMillis;
};

}

#endif
