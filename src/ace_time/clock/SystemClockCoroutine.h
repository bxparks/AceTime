/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_COROUTINE_H
#define ACE_TIME_SYSTEM_CLOCK_COROUTINE_H

// activate only if <AceRoutine.h> is included before this header
#ifdef ACE_ROUTINE_VERSION

#include <stdint.h>
#include <AceCommon.h> // TimingStats
#include <AceRoutine.h>
#include "SystemClock.h"

class SystemClockCoroutineTest_runCoroutine;

namespace ace_time {
namespace clock {

/**
 * A subclass of SystemClock that mixes in the ace_routine::Coroutine class of
 * the AceRoutine library to turn it into a coroutine. It uses the non-blocking
 * Clock API of the referenceClock. This is helpful when the referenceClock
 * issues a network request to an NTP server. The `CoroutineScheduler` can be
 * used if it is already used for something else, or you can call the
 * `SystemCoroutine::runCoroutine()` directly from the global loop() function.
 *
 * The class attempts to sync with its referenceClock every
 * initialSyncPeriodSeconds. If the request fails, then it retries with an
 * exponential backoff (doubling the delay every iteration), until the sync
 * period becomes greater than syncPeriodSeconds, then the delay is set
 * permanently to syncPeriodSeconds.
 *
 * Initially, `SystemClockLoop` used the blocking API of Clock, and
 * `SystemClockCoroutine` used the non-blocking API. That meant that
 * `SystemClockCoroutine` was better suited for referenceClocks that could block
 * for a long time (e.g. NtpClock). At some point however, `SystemClockLoop`
 * was converted to use the non-blocking API as well, so the two classes are now
 * functionally equivalent. I keep around the SystemClockCoroutine class because
 * I find the code easier to understand. But for the end-users of the library,
 * they are equivalent.
 */
class SystemClockCoroutine :
    public SystemClock,
    public ace_routine::Coroutine {

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
     *    the systemClock is not initialized (default 5)
     * @param requestTimeoutMillis number of milliseconds before the request to
     *    referenceClock times out
     * @param timingStats internal statistics (nullable)
     */
    explicit SystemClockCoroutine(
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
     * @copydoc Coroutine::runCoroutine()
     *
     * Make a request to the referenceClock, wait for the request, then set the
     * SystemClock (the parent class) to the time returned by the
     * referneceClock. If the referenceClock returns an error, implement a retry
     * algorithm with an exponential backoff, until a maximum of
     * syncPeriodSeconds interval is reached.
     *
     * Two ways to run this:
     *
     * 1) Call this method directly from the global loop() function.
     * 2) Register this coroutine with CoroutineScheduler using
     *    Coroutine::setupCoroutine() in the global setup(). Then call
     *    CoroutineScheduler::loop() in the global loop() function.
     */
    int runCoroutine() override {
      keepAlive();
      if (getReferenceClock() == nullptr) return 0;

      uint32_t nowMillis = clockMillis();

      COROUTINE_LOOP() {
        // Send request
        getReferenceClock()->sendRequest();
        mRequestStartMillis = coroutineMillis();
        mRequestStatus = kStatusSent;
        setPrevSyncAttemptMillis(nowMillis);
        setNextSyncAttemptMillis(
            nowMillis + mCurrentSyncPeriodSeconds * (uint32_t) 1000);

        // Wait for request until mRequestTimeoutMillis.
        while (true) {
          if (getReferenceClock()->isResponseReady()) {
            mRequestStatus = kStatusOk;
            break;
          }

          {
            // Local variable waitMillis must be scoped with {} so that the
            // goto in COROUTINE_LOOP() skips past it. This seems to be
            // a problem only in clang++; g++ seems to be fine without it.
            uint16_t waitMillis =
                (uint16_t) coroutineMillis() - mRequestStartMillis;
            if (waitMillis >= mRequestTimeoutMillis) {
              mRequestStatus = kStatusTimedOut;
              setSyncStatusCode(kSyncStatusTimedOut);
              break;
            }
          }

          COROUTINE_YIELD();
        }

        // Process the response
        if (mRequestStatus == kStatusOk) {
          acetime_t nowSeconds = getReferenceClock()->readResponse();
          if (mTimingStats != nullptr) {
            uint16_t elapsedMillis =
                (uint16_t) coroutineMillis() - mRequestStartMillis;
            mTimingStats->update(elapsedMillis);
          }

          if (nowSeconds == kInvalidSeconds) {
            setSyncStatusCode(kSyncStatusError);
            // Clobber the mRequestStatus to trigger the exponential backoff
            mRequestStatus = kStatusUnknown;
          } else {
            syncNow(nowSeconds);
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
            setSyncStatusCode(kSyncStatusOk);
          }
        }

        // Wait for mCurrentSyncPeriodSeconds
        setNextSyncAttemptMillis(
            nowMillis + mCurrentSyncPeriodSeconds * (uint32_t) 1000);
        COROUTINE_DELAY_SECONDS(mCurrentSyncPeriodSeconds);

        // Determine the retry delay time based on success or failure. If
        // failure, retry with exponential backoff, until the delay becomes
        // mSyncPeriodSeconds.
        if (mRequestStatus != kStatusOk) {
          if (mCurrentSyncPeriodSeconds >= mSyncPeriodSeconds / 2) {
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
          } else {
            mCurrentSyncPeriodSeconds *= 2;
          }
        }
      }
    }

    /** Return the current request status. Mostly for debugging. */
    uint8_t getRequestStatus() const { return mRequestStatus; }

  protected:
    /** Empty constructor used for testing. */
    SystemClockCoroutine() {}

  private:
    friend class ::SystemClockCoroutineTest_runCoroutine;

    /** Request state unknown or request error. */
    static const uint8_t kStatusUnknown = 0;

    /** Request has been sent and waiting for response. */
    static const uint8_t kStatusSent = 1;

    /** Request received and valid. */
    static const uint8_t kStatusOk = 2;

    /** Request timed out. */
    static const uint8_t kStatusTimedOut = 3;

    // disable copy constructor and assignment operator
    SystemClockCoroutine(const SystemClockCoroutine&) = delete;
    SystemClockCoroutine& operator=(const SystemClockCoroutine&) = delete;

    uint16_t const mSyncPeriodSeconds = 3600;
    uint16_t const mRequestTimeoutMillis = 1000;
    ace_common::TimingStats* const mTimingStats = nullptr;

    uint16_t mRequestStartMillis; // lower 16-bit of millis()
    uint16_t mCurrentSyncPeriodSeconds = 5;
    uint16_t mWaitCount;
    uint8_t mRequestStatus = kStatusUnknown;
};

}
}

#endif

#endif
