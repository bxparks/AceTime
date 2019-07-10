/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_SYNC_LOOP_H
#define ACE_TIME_SYSTEM_CLOCK_SYNC_LOOP_H

#include <stdint.h>
#include "SystemClock.h"

namespace ace_time {
namespace clock {

/**
 * A class that that syncs the SystemClock with its mSyncTimeProvider. The call
 * to mSyncTimeProvider will be a blocking call which can be a problem for time
 * providers like NtpTimeProvider which makes a network request. If this is a
 * problem, use SystemClockSyncCoroutine instead.
 *
 * Initial syncing occurs at initialSyncPeriodSeconds interval, until the
 * first successful sync, then occurs at syncPeriodSeconds interval.
 * Initial syncing implements an exponential backoff when the sync request
 * fails, increasing from initialSyncPeriodSeconds to until a maximum of
 * syncPeriodSeconds.
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
     */
    explicit SystemClockSyncLoop(SystemClock& systemClock,
          uint16_t syncPeriodSeconds = 3600,
          uint16_t initialSyncPeriodSeconds = 5):
      mSystemClock(systemClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * Call this from the global loop() method. This uses a blocking call to
     * the SystemClock.mSyncTimeProvider.
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
    // disable copy constructor and assignment operator
    SystemClockSyncLoop(const SystemClockSyncLoop&) = delete;
    SystemClockSyncLoop& operator=(const SystemClockSyncLoop&) = delete;

    SystemClock& mSystemClock;
    uint16_t const mSyncPeriodSeconds;

    unsigned long mLastSyncMillis = 0; // should be the same type as millis()
    uint16_t mCurrentSyncPeriodSeconds;
};

}
}

#endif
