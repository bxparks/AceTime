/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_LOOP_H
#define ACE_TIME_SYSTEM_CLOCK_LOOP_H

#include <stdint.h>
#include <AceCommon.h> // TimingStats
#include "SystemClock.h"

class SystemClockLoopTest_loop;

namespace ace_time {
namespace clock {

/**
 * A subclass of SystemClock that sync with its mReferenceClock using the
 * non-blocking Clock API of the referenceClock. This is helpful when the
 * referenceClock issues a network request to an NTP server. The
 * SystemClockLoop::loop() function should be called from the global loop()
 * function.
 *
 * Syncing occurs at initialSyncPeriodSeconds interval, until the first
 * successful sync, then subsequent syncing occurs at syncPeriodSeconds
 * interval. Initial syncing implements an exponential backoff when the sync
 * request fails, increasing from initialSyncPeriodSeconds to until a maximum of
 * syncPeriodSeconds.
 *
 * Initially, `SystemClockLoop` used the blocking API of Clock, and
 * `SystemClockCoroutine` used the non-blocking API. That meant that
 * `SystemClockCoroutine` was better suited for referenceClocks that could block
 * for a long time (e.g. NtpClock). at some point however, `SystemClockLoop`
 * was converted to use the non-blocking API as well, so the two classes are now
 * functionally equivalent. I keep around the SystemClockCoroutine class because
 * I find the code easier to understand. But for the end-users of the library,
 * they are equivalent.
 */
class SystemClockLoop : public SystemClock {
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
     * @param requestTimeoutMillis number of milliseconds before the request to
     *    referenceClock times out
     * @param timingStats internal statistics (nullable)
     */
    explicit SystemClockLoop(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        ace_common::TimingStats* timingStats = nullptr):
      SystemClock(referenceClock, backupClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mTimingStats(timingStats),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * Make a request to the referenceClock, wait for the request, then set the
     * SystemClock (the parent class) to the time returned by the
     * referneceClock. If the referenceClock returns an error, implement a retry
     * algorithm with an exponential backoff, until a maximum of
     * syncPeriodSeconds interval is reached.
     *
     * This method should be called from the global loop() method.
     */
    void loop() {
      keepAlive();
      if (getReferenceClock() == nullptr) return;

      unsigned long nowMillis = clockMillis();

      // Finite state machine based on mRequestStatus
      switch (mRequestStatus) {
        case kStatusReady:
          getReferenceClock()->sendRequest();
          mRequestStartMillis = nowMillis;
          mRequestStatus = kStatusSent;
          setSecondsSinceSyncAttempt(0);
          setSecondsToSyncAttempt(mCurrentSyncPeriodSeconds);
          break;

        case kStatusSent: {
          unsigned long elapsedMillis = nowMillis - mRequestStartMillis;
          setSecondsSinceSyncAttempt(elapsedMillis / 1000);
          if (mTimingStats) mTimingStats->update(elapsedMillis);

          if (getReferenceClock()->isResponseReady()) {
            acetime_t nowSeconds = getReferenceClock()->readResponse();

            // If response came back but was invalid, reschedule.
            if (nowSeconds == kInvalidSeconds) {
              mRequestStatus = kStatusWaitForRetry;
              setSyncStatusCode(kSyncStatusError);
            } else {
              syncNow(nowSeconds);
              mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
              mLastSyncMillis = nowMillis;
              mRequestStatus = kStatusOk;
              setSyncStatusCode(kSyncStatusOk);
            }
          } else {
            // If timed out, reschedule.
            if (elapsedMillis >= mRequestTimeoutMillis) {
              mRequestStatus = kStatusWaitForRetry;
              setSyncStatusCode(kSyncStatusTimedOut);
            }
          }
          setSecondsToSyncAttempt(mCurrentSyncPeriodSeconds);
          break;
        }

        // The previous request succeeded, so wait until the next sync attempt.
        case kStatusOk: {
          unsigned long elapsedMillis = nowMillis - mLastSyncMillis;
          setSecondsSinceSyncAttempt(elapsedMillis / 1000);
          if (elapsedMillis >= mCurrentSyncPeriodSeconds * 1000UL) {
            mRequestStatus = kStatusReady;
          }
          setSecondsToSyncAttempt((int32_t) mCurrentSyncPeriodSeconds
              - getSecondsSinceSyncAttempt());
          break;
        }

        // Previous request failed, so update timing parameters so that
        // subsequent loop() retries with an exponential backoff, until a
        // maximum of mSyncPeriodSeconds is reached.
        case kStatusWaitForRetry: {
          unsigned long elapsedMillis = nowMillis - mRequestStartMillis;
          setSecondsSinceSyncAttempt(elapsedMillis / 1000);

          // Adjust mCurrentSyncPeriodSeconds using exponential backoff.
          if (elapsedMillis >= mCurrentSyncPeriodSeconds * 1000UL) {
            if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
              mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
            } else {
              mCurrentSyncPeriodSeconds *= 2;
            }
            mRequestStatus = kStatusReady;
            setSecondsToSyncAttempt(0);
          } else {
            setSecondsToSyncAttempt((int32_t) mCurrentSyncPeriodSeconds
                - getSecondsSinceSyncAttempt());
          }
          break;
        }
      }
    }

  protected:
    /** Empty constructor used for testing. */
    SystemClockLoop() {}

  private:
    friend class ::SystemClockLoopTest_loop;

    /** Ready to send request. */
    static const uint8_t kStatusReady = 0;

    /** Request sent, waiting for response. */
    static const uint8_t kStatusSent = 1;

    /** Request received and is valid. */
    static const uint8_t kStatusOk = 2;

    /** Request received but is invalid, so retry with exponential backoff. */
    static const uint8_t kStatusWaitForRetry = 3;

    // disable copy constructor and assignment operator
    SystemClockLoop(const SystemClockLoop&) = delete;
    SystemClockLoop& operator=(const SystemClockLoop&) = delete;

    uint16_t const mSyncPeriodSeconds = 3600;
    uint16_t const mRequestTimeoutMillis = 1000;
    ace_common::TimingStats* const mTimingStats = nullptr;

    unsigned long mLastSyncMillis;
    unsigned long mRequestStartMillis;
    uint16_t mCurrentSyncPeriodSeconds = 5;
    uint8_t mRequestStatus = kStatusReady;
};

}
}

#endif
