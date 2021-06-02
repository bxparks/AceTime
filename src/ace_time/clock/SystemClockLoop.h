/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_LOOP_H
#define ACE_TIME_SYSTEM_CLOCK_LOOP_H

#include <stdint.h>
#include <AceCommon.h> // TimingStats
#include "SystemClock.h"

class SystemClockLoopTest;
class SystemClockLoopTest_loop;
class SystemClockLoopTest_setup;
class SystemClockLoopTest_backupNow;
class SystemClockLoopTest_syncNow;
class SystemClockLoopTest_getNow;

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
 *
 * @tparam T_SCCI SystemClock ClockInterface
 */
template <typename T_SCCI>
class SystemClockLoopTemplate : public SystemClockTemplate<T_SCCI> {
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
    explicit SystemClockLoopTemplate(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        ace_common::TimingStats* timingStats = nullptr):
      SystemClockTemplate<T_SCCI>(referenceClock, backupClock),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mTimingStats(timingStats),
      mCurrentSyncPeriodSeconds(initialSyncPeriodSeconds) {}

    /**
     * Make a request to the referenceClock every syncPeriodSeconds seconds.
     * Wait for the request, then set the SystemClock (the parent class) to the
     * time returned by the referenceClock. If the referenceClock returns an
     * error, implement a retry algorithm with an exponential backoff, until a
     * maximum of syncPeriodSeconds interval is reached.
     *
     * This method should be called from the global loop() method.
     */
    void loop() {
      this->keepAlive();
      if (this->getReferenceClock() == nullptr) return;

      uint32_t nowMillis = this->clockMillis();

      // Finite state machine based on mRequestStatus
      switch (mRequestStatus) {
        case kStatusReady:
          this->getReferenceClock()->sendRequest();
          mRequestStartMillis = nowMillis;
          mRequestStatus = kStatusSent;
          this->setPrevSyncAttemptMillis(nowMillis);
          this->setNextSyncAttemptMillis(nowMillis
              + mCurrentSyncPeriodSeconds * (uint32_t) 1000);
          break;

        case kStatusSent: {
          uint32_t elapsedMillis = nowMillis - mRequestStartMillis;
          if (mTimingStats) mTimingStats->update((uint16_t) elapsedMillis);

          if (this->getReferenceClock()->isResponseReady()) {
            acetime_t nowSeconds = this->getReferenceClock()->readResponse();

            if (nowSeconds == this->kInvalidSeconds) {
              // If response came back but was invalid, reschedule.
              mRequestStatus = kStatusWaitForRetry;
              this->setSyncStatusCode(this->kSyncStatusError);
            } else {
              // Request succeeded.
              this->syncNow(nowSeconds);
              mCurrentSyncPeriodSeconds = mSyncPeriodSeconds;
              mRequestStatus = this->kStatusOk;
              this->setSyncStatusCode(this->kSyncStatusOk);
            }
          } else {
            // If timed out, reschedule.
            if (elapsedMillis >= mRequestTimeoutMillis) {
              mRequestStatus = this->kStatusWaitForRetry;
              this->setSyncStatusCode(this->kSyncStatusTimedOut);
            }
          }
          break;
        }

        // The previous request succeeded, so wait until the next sync attempt.
        case kStatusOk: {
          uint32_t elapsedMillis = nowMillis - mRequestStartMillis;
          if (elapsedMillis >= mCurrentSyncPeriodSeconds * (uint32_t) 1000) {
            mRequestStatus = kStatusReady;
          }
          break;
        }

        // Previous request failed, so update timing parameters so that
        // subsequent loop() retries with an exponential backoff, until a
        // maximum of mSyncPeriodSeconds is reached.
        case kStatusWaitForRetry: {
          uint32_t elapsedMillis = nowMillis - mRequestStartMillis;
          // Adjust mCurrentSyncPeriodSeconds using exponential backoff.
          if (elapsedMillis >= mCurrentSyncPeriodSeconds * (uint32_t) 1000) {
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

  protected:
    /** Empty constructor used for testing. */
    SystemClockLoopTemplate() {}

    // disable copy constructor and assignment operator
    SystemClockLoopTemplate(const SystemClockLoopTemplate&) = delete;
    SystemClockLoopTemplate& operator=(const SystemClockLoopTemplate&) = delete;

  private:
    friend class ::SystemClockLoopTest;
    friend class ::SystemClockLoopTest_loop;
    friend class ::SystemClockLoopTest_syncNow;
    friend class ::SystemClockLoopTest_setup;
    friend class ::SystemClockLoopTest_backupNow;
    friend class ::SystemClockLoopTest_getNow;

    /** Ready to send request. */
    static const uint8_t kStatusReady = 0;

    /** Request sent, waiting for response. */
    static const uint8_t kStatusSent = 1;

    /** Request received and is valid. */
    static const uint8_t kStatusOk = 2;

    /** Request received but is invalid, so retry with exponential backoff. */
    static const uint8_t kStatusWaitForRetry = 3;

    uint16_t const mSyncPeriodSeconds = 3600;
    uint16_t const mRequestTimeoutMillis = 1000;
    ace_common::TimingStats* const mTimingStats = nullptr;

    uint32_t mRequestStartMillis;
    uint16_t mCurrentSyncPeriodSeconds = 5;
    uint8_t mRequestStatus = kStatusReady;
};

/**
 * Concrete template instance of SystemClockLoopTemplate that uses the real
 * millis().
 */
using SystemClockLoop = SystemClockLoopTemplate<hw::ClockInterface>;


}
}

#endif
