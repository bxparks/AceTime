/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_LOOP_H
#define ACE_TIME_SYSTEM_CLOCK_LOOP_H

#include <stdint.h>
#include "SystemClock.h"

namespace ace_time {
namespace clock {

/**
 * A subclass of SystemClock that sync with its mReferenceClock using a
 * blocking mReferenceClock->getNow() call. The blocking call can be a problem
 * for clocks like NtpClock which makes a network request. If this is a
 * problem, use SystemClockCoroutine instead.
 *
 * Initial syncing occurs at initialSyncPeriodSeconds interval, until the
 * first successful sync, then subsequent syncing occurs at syncPeriodSeconds
 * interval. Initial syncing implements an exponential backoff when the sync
 * request fails, increasing from initialSyncPeriodSeconds to until a maximum
 * of syncPeriodSeconds.
 */
class SystemClockLoop: public SystemClock {
  public:
    /**
     * Constructor.
     *
     * @param referenceClock The authoritative source of the time. If this is
     *    null, the object relies just on clockMillis() and the user to set the
     *    proper time using setNow().
     * @param backupClock An RTC chip which continues to keep time
     *    even when power is lost. Can be null.
     * @param syncPeriodSeconds seconds between normal sync attempts
     *    (default 3600)
     * @param initialSyncPeriodSeconds seconds between sync attempts when
     *    the systemClock is not initialized (default 5), exponentially
     *    increasing (2X) at each attempt until syncPeriodSeconds is reached
     */
    explicit SystemClockLoop(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5):
      SystemClock(referenceClock, backupClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * Call this from the global loop() method. This uses a blocking call to
     * the mReferenceClock.
     */
    void loop() {
      if (mReferenceClock == nullptr) return;

      unsigned long nowMillis = clockMillis();
      unsigned long timeSinceLastSync = nowMillis - mLastSyncMillis;

      if (timeSinceLastSync >= mCurrentSyncPeriodSeconds * 1000UL
          || getNow() == kInvalidSeconds) {
        acetime_t nowSeconds = mReferenceClock->getNow();

        if (nowSeconds == kInvalidSeconds) {
          // subsequent loop() retries with an exponential backoff, until a
          // maximum of mSyncPeriodSeconds is reached.
          if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
          } else {
            mCurrentSyncPeriodSeconds *= 2;
          }
        } else {
          syncNow(nowSeconds);
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
      unsigned long elapsedMillis = clockMillis() - mLastSyncMillis;
      return elapsedMillis / 1000;
    }

  private:
    // disable copy constructor and assignment operator
    SystemClockLoop(const SystemClockLoop&) = delete;
    SystemClockLoop& operator=(const SystemClockLoop&) = delete;

    uint16_t const mSyncPeriodSeconds;
    uint16_t mCurrentSyncPeriodSeconds;
    unsigned long mLastSyncMillis = 0;
};

}
}

#endif
