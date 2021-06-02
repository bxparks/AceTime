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

class SystemClockCoroutineTest;
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
 *
 * @tparam T_SCCI SystemClock ClockInterface
 * @tparam T_CRCI Coroutine ClockInterface
 */
template <typename T_SCCI, typename T_CRCI>
class SystemClockCoroutineTemplate :
    public SystemClockTemplate<T_SCCI>,
    public ace_routine::CoroutineTemplate<T_CRCI> {

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
    explicit SystemClockCoroutineTemplate(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        ace_common::TimingStats* timingStats = nullptr):
      SystemClockTemplate<T_SCCI>(referenceClock, backupClock),
      ace_routine::CoroutineTemplate<T_CRCI>(),
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
      this->keepAlive();
      if (this->getReferenceClock() == nullptr) return 0;

      uint32_t nowMillis = this->clockMillis();

      COROUTINE_LOOP() {
        // Send request
        this->getReferenceClock()->sendRequest();
        mRequestStartMillis = this->coroutineMillis();
        mRequestStatus = kStatusSent;
        this->setPrevSyncAttemptMillis(nowMillis);
        this->setNextSyncAttemptMillis(
            nowMillis + mCurrentSyncPeriodSeconds * (uint32_t) 1000);

        // Wait for request until mRequestTimeoutMillis.
        while (true) {
          if (this->getReferenceClock()->isResponseReady()) {
            mRequestStatus = kStatusOk;
            break;
          }

          {
            // Local variable waitMillis must be scoped with {} so that the
            // goto in COROUTINE_LOOP() skips past it. This seems to be
            // a problem only in clang++; g++ seems to be fine without it.
            uint16_t waitMillis =
                (uint16_t) this->coroutineMillis() - mRequestStartMillis;
            if (waitMillis >= mRequestTimeoutMillis) {
              mRequestStatus = kStatusTimedOut;
              this->setSyncStatusCode(this->kSyncStatusTimedOut);
              break;
            }
          }

          COROUTINE_YIELD();
        }

        // Process the response
        if (mRequestStatus == kStatusOk) {
          acetime_t nowSeconds = this->getReferenceClock()->readResponse();
          if (mTimingStats != nullptr) {
            uint16_t elapsedMillis =
                (uint16_t) this->coroutineMillis() - mRequestStartMillis;
            mTimingStats->update(elapsedMillis);
          }

          if (nowSeconds == this->kInvalidSeconds) {
            this->setSyncStatusCode(this->kSyncStatusError);
            // Clobber the mRequestStatus to trigger the exponential backoff
            mRequestStatus = kStatusUnknown;
          } else {
            this->syncNow(nowSeconds);
            mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
            this->setSyncStatusCode(this->kSyncStatusOk);
          }
        }

        // Wait for mCurrentSyncPeriodSeconds
        this->setNextSyncAttemptMillis(
            nowMillis + mCurrentSyncPeriodSeconds * (uint32_t) 1000);
        for (mWaitCount = 0;
            mWaitCount < mCurrentSyncPeriodSeconds;
            mWaitCount++
        ) {
          COROUTINE_DELAY(1000);
        }

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
    SystemClockCoroutineTemplate() {}

  private:
    friend class ::SystemClockCoroutineTest;
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
    SystemClockCoroutineTemplate(const SystemClockCoroutineTemplate&) = delete;
    SystemClockCoroutineTemplate& operator=(
        const SystemClockCoroutineTemplate&) = delete;

    uint16_t const mSyncPeriodSeconds = 3600;
    uint16_t const mRequestTimeoutMillis = 1000;
    ace_common::TimingStats* const mTimingStats = nullptr;

    uint16_t mRequestStartMillis; // lower 16-bit of millis()
    uint16_t mCurrentSyncPeriodSeconds = 5;
    uint16_t mWaitCount;
    uint8_t mRequestStatus = kStatusUnknown;
};

/**
 * Concrete template instance of SystemClockCoroutineTemplate that uses the
 * real millis().
 */
using SystemClockCoroutine = SystemClockCoroutineTemplate<
    hw::ClockInterface, hw::ClockInterface
>;

}
}

#endif

#endif
