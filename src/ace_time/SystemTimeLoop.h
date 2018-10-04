#ifndef ACE_TIME_SYSTEM_TIME_LOOP_H
#define ACE_TIME_SYSTEM_TIME_LOOP_H

#include <stdint.h>
#include "SystemTimeKeeper.h"

namespace ace_time {

/**
 * A class that peridically freshens the SystemTimeKeeper using the heartbeat
 * call to getNow(), and peridically syncs with the syncTimeProvider (if it was
 * provided to the SystemTimeKeeper).
 */
class SystemTimeLoop {
  public:
    /**
     * Constructor.
     *
     * @param systemTimeKeeper the underlying SystemTimeKeeper to sync
     * @param syncPeriodSeconds seconds between sync attempts (default 3600)
     * @param heartbeatPeriodMillis millis between calls to getNow()
     *    (default 5000)
     */
    SystemTimeLoop(SystemTimeKeeper& systemTimeKeeper,
          uint16_t syncPeriodSeconds = 3600,
          uint16_t heartbeatPeriodMillis = 5000):
      mSystemTimeKeeper(systemTimeKeeper),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      unsigned long nowMillis = millis();
      uint32_t timeSinceLastSync = nowMillis - mLastSyncMillis;

      // Make sure that mSecondsSinceEpoch does not fall too far behind.
      if (timeSinceLastSync >= mHeartbeatPeriodMillis) {
        mSystemTimeKeeper.getNow();
      }

      // Synchronize if a TimeProvider is available, and mSyncPeriodSeconds has
      // passed.
      if (mSystemTimeKeeper.mSyncTimeProvider != nullptr) {
        if (timeSinceLastSync >= mSyncPeriodSeconds
            * (uint32_t) syncPeriodSeconds) {
          // blocking call
          uint32_t nowSeconds = mSystemTimeKeeper.mSyncTimeProvider->getNow();
          if (nowSeconds == 0) return;
          mSystemTimeKeeper.sync(nowSeconds);
          mLastSyncMillis = nowMillis;
        }
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mSyncPeriodSeconds;
    uint16_t const mHeartbeatPeriodMillis;
    unsigned long mLastSyncMillis; // should be the same type as millis()
};

}

#endif
