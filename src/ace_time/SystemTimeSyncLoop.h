#ifndef ACE_TIME_SYSTEM_TIME_SYNC_LOOP_H
#define ACE_TIME_SYSTEM_TIME_SYNC_LOOP_H

#include <stdint.h>
#include "SystemTimeKeeper.h"

namespace ace_time {

/**
 * A class that periodically that syncs the SystemTimeKeeper with its
 * syncTimeProvider.
 */
class SystemTimeSyncLoop {
  public:
    /**
     * Constructor.
     *
     * @param systemTimeKeeper the system time keeper to sync up
     * @param syncPeriodSeconds seconds between normal sync attempts
     *    (default 3600)
     * @param initialSyncPeriodSeconds seconds between sync attempts when
     *    the systemTimeKeeper is not initialized (default 5)
     * @param requestTimeoutMillis number of milliseconds before the request to
     *    syncTimeProvider times out
     */
    SystemTimeSyncLoop(SystemTimeKeeper& systemTimeKeeper,
          uint16_t syncPeriodSeconds = 3600,
          uint16_t initialSyncPeriodSeconds = 5,
          uint16_t requestTimeoutMillis = 1000):
      mSystemTimeKeeper(systemTimeKeeper),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mInitialSyncPeriodSeconds(initialSyncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      if (mSystemTimeKeeper.mSyncTimeProvider == nullptr) return;

      unsigned long nowMillis = millis();
      uint32_t timeSinceLastSync = nowMillis - mLastSyncMillis;

      if (timeSinceLastSync >= mCurrentSyncPeriodSeconds * (uint32_t) 1000
          || mSystemTimeKeeper.getNow() == 0) {
        uint32_t nowSeconds = mSystemTimeKeeper.mSyncTimeProvider->getNow();

        if (nowSeconds == 0) {
          // retry with exponential backoff
          if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
          } else {
            mCurrentSyncPeriodSeconds *= 2;
          }
        } else {
          mSystemTimeKeeper.sync(nowSeconds);
          mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
        }

        mLastSyncMillis = nowMillis;
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mSyncPeriodSeconds;
    uint16_t const mInitialSyncPeriodSeconds;
    uint16_t const mRequestTimeoutMillis;

    unsigned long mLastSyncMillis = 0; // should be the same type as millis()
    uint16_t mCurrentSyncPeriodSeconds;
};

}

#endif
