#ifndef ACE_TIME_SYSTEM_CLOCK_SYNC_LOOP_H
#define ACE_TIME_SYSTEM_CLOCK_SYNC_LOOP_H

#include <stdint.h>
#include "SystemClock.h"

namespace ace_time {
namespace provider {

/**
 * A class that periodically that syncs the SystemClock with its
 * syncTimeProvider.
 */
class SystemClockSyncLoop {
  public:
    /**
     * Constructor.
     *
     * @param systemClock the system time keeper to sync up
     * @param syncPeriodSeconds seconds between normal sync attempts
     *    (default 3600)
     * @param initialSyncPeriodSeconds seconds between sync attempts when
     *    the systemClock is not initialized (default 5)
     * @param requestTimeoutMillis number of milliseconds before the request to
     *    syncTimeProvider times out
     */
    SystemClockSyncLoop(SystemClock& systemClock,
          uint16_t syncPeriodSeconds = 3600,
          uint16_t initialSyncPeriodSeconds = 5,
          uint16_t requestTimeoutMillis = 1000):
      mSystemClock(systemClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mInitialSyncPeriodSeconds(initialSyncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      if (mSystemClock.mSyncTimeProvider == nullptr) return;

      unsigned long nowMillis = millis();
      unsigned long timeSinceLastSync = nowMillis - mLastSyncMillis;

      if (timeSinceLastSync >= mCurrentSyncPeriodSeconds * 1000UL
          || mSystemClock.getNow() == 0) {
        acetime_t nowSeconds = mSystemClock.mSyncTimeProvider->getNow();

        if (nowSeconds == 0) {
          // retry with exponential backoff
          if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
          } else {
            mCurrentSyncPeriodSeconds *= 2;
          }
        } else {
          mSystemClock.sync(nowSeconds);
          mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
        }

        mLastSyncMillis = nowMillis;
      }
    }

    /**
     * Return the number of seconds since last sync. Mostly for
     * debugging purposes.
     */
    uint16_t getSecondsSinceLastSync() const {
      unsigned long elapsedMillis = millis() - mLastSyncMillis;
      return elapsedMillis / 1000;
    }

  private:
    SystemClock& mSystemClock;
    uint16_t const mSyncPeriodSeconds;
    uint16_t const mInitialSyncPeriodSeconds;
    uint16_t const mRequestTimeoutMillis;

    unsigned long mLastSyncMillis = 0; // should be the same type as millis()
    uint16_t mCurrentSyncPeriodSeconds;
};

}
}

#endif
