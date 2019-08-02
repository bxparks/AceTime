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
    /** Ready to send request. */
    static const uint8_t kStatusReady = 0;

    /** Request sent, waiting for response. */
    static const uint8_t kStatusSent = 1;

    /** Request received and is valid. */
    static const uint8_t kStatusOk = 2;

    /** Request received but is invalid, so retry with exponential backoff. */
    static const uint8_t kStatusRetry = 3;

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
     * @param timingStats internal statistics
     */
    explicit SystemClockLoop(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        common::TimingStats* timingStats = nullptr):
      SystemClock(referenceClock, backupClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mTimingStats(timingStats),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * Call this from the global loop() method. This uses a blocking call to
     * the mReferenceClock.
     */
    void loop() {
      keepAlive();
      if (mReferenceClock == nullptr) return;

      unsigned long nowMillis = clockMillis();

      // Finite state machine based on mRequestStatus
      switch (mRequestStatus) {
        case kStatusReady:
          mReferenceClock->sendRequest();
          mRequestStartMillis = nowMillis;
          mRequestStatus = kStatusSent;
          break;
        case kStatusSent:
          if (mReferenceClock->isResponseReady()) {
            acetime_t nowSeconds = mReferenceClock->readResponse();
            if (mTimingStats != nullptr) {
              uint16_t elapsedMillis =
                  (uint16_t) nowMillis - mRequestStartMillis;
              mTimingStats->update(elapsedMillis);
            }
            if (nowSeconds == kInvalidSeconds) {
              mRequestStatus = kStatusRetry;
            } else {
              syncNow(nowSeconds);
              mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
              mLastSyncMillis = nowMillis;
              mRequestStatus = kStatusOk;
            }
          } else {
            uint16_t waitMillis = (uint16_t) nowMillis - mRequestStartMillis;
            if (waitMillis >= mRequestTimeoutMillis) {
              mRequestStatus = kStatusRetry;
            }
          }
          break;
        case kStatusOk: {
          unsigned long millisSinceLastSync = nowMillis - mLastSyncMillis;
          if (millisSinceLastSync >= mCurrentSyncPeriodSeconds * 1000) {
            mRequestStatus = kStatusReady;
          }
          break;
        }
        case kStatusRetry: {
          // subsequent loop() retries with an exponential backoff, until a
          // maximum of mSyncPeriodSeconds is reached.
          unsigned long millisSinceLastSync = nowMillis - mLastSyncMillis;
          if (millisSinceLastSync >= mCurrentSyncPeriodSeconds * 1000) {
            if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
              mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
            } else {
              mCurrentSyncPeriodSeconds *= 2;
            }
            mRequestStatus = kStatusReady;
          }
          break;
        }
      }
    }

  private:
    // disable copy constructor and assignment operator
    SystemClockLoop(const SystemClockLoop&) = delete;
    SystemClockLoop& operator=(const SystemClockLoop&) = delete;

    uint16_t const mSyncPeriodSeconds;
    uint16_t const mRequestTimeoutMillis;
    common::TimingStats* const mTimingStats;

    unsigned long mLastSyncMillis;
    uint16_t mRequestStartMillis; // lower 16-bits of millis()
    uint16_t mCurrentSyncPeriodSeconds;
    uint8_t mRequestStatus = kStatusReady;
};

}
}

#endif
