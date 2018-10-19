#ifndef ACE_TIME_SYSTEM_TIME_HEARTBEAT_LOOP_H
#define ACE_TIME_SYSTEM_TIME_HEARTBEAT_LOOP_H

#include <stdint.h>
#include "SystemTimeKeeper.h"

namespace ace_time {

/**
 * A class that peridically freshens the SystemTimeKeeper using the heartbeat
 * call to getNow(). Call loop() from the global loop() method.
 */
class SystemTimeHeartbeatLoop {
  public:
    /**
     * Constructor.
     *
     * @param systemTimeKeeper the underlying SystemTimeKeeper
     * @param heartbeatPeriodMillis millis between calls to getNow()
     *    (default 5000)
     */
    SystemTimeHeartbeatLoop(SystemTimeKeeper& systemTimeKeeper,
          uint16_t heartbeatPeriodMillis = 5000):
      mSystemTimeKeeper(systemTimeKeeper),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    /**
     * Call this from the global loop() method to make SystemTimeKeeper
     * keep in sync with the system millis().
     */
    void loop() {
      unsigned long nowMillis = millis();
      uint32_t timeSinceLastSync = nowMillis - mLastSyncMillis;

      // Make sure that mSecondsSinceEpoch does not fall too far behind.
      if (timeSinceLastSync >= mHeartbeatPeriodMillis) {
        mSystemTimeKeeper.getNow();
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mHeartbeatPeriodMillis;

    unsigned long mLastSyncMillis = 0; // should be the same type as millis()
};

}

#endif
