#ifndef ACE_TIME_SYSTEM_TIME_SYNC_COROUTINE_H
#define ACE_TIME_SYSTEM_TIME_SYNC_COROUTINE_H

#include <stdint.h>
#include "SystemTimeKeeper.h"
#include "common/TimingStats.h"

namespace ace_time {

/**
 * A coroutine that syncs the SystemTimeKeeper with its syncTimeProvider.
 */
class SystemTimeSyncCoroutine: public ace_routine::Coroutine {
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
     * @param timingStats internal statistics
     */
    SystemTimeSyncCoroutine(SystemTimeKeeper& systemTimeKeeper,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        common::TimingStats* timingStats = nullptr):
      mSystemTimeKeeper(systemTimeKeeper),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mInitialSyncPeriodSeconds(initialSyncPeriodSeconds),
      mRequestTimeoutMillis(requestTimeoutMillis),
      mTimingStats(timingStats) {}

    /**
     * @copydoc Coroutine::runCoroutine()
     *
     * The CoroutineScheduler will use this method if enabled. Don't forget to
     * call setupCoroutine() (inherited from Coroutine) in the global setup() to
     * register this coroutine into the CoroutineScheduler.
     */
    int runCoroutine() override {
      if (mSystemTimeKeeper.mSyncTimeProvider == nullptr) return 0;

      COROUTINE_LOOP() {
        // Send request
        mSystemTimeKeeper.mSyncTimeProvider->sendRequest();
        mRequestStartTime = millis();

        // Wait for request
        while (true) {
          if (mSystemTimeKeeper.mSyncTimeProvider->isResponseReady()) {
            mRequestStatus = kStatusOk;
            break;
          }

          uint16_t waitTime = millis() - mRequestStartTime;
          if (waitTime >= mRequestTimeoutMillis) {
            mRequestStatus = kStatusTimedOut;
            break;
          }

          COROUTINE_YIELD();
        }

        // Process the response
        if (mRequestStatus == kStatusOk) {
          uint32_t nowSeconds =
              mSystemTimeKeeper.mSyncTimeProvider->readResponse();
          uint16_t elapsedTime = millis() - mRequestStartTime;
          if (mTimingStats != nullptr) {
            mTimingStats->update(elapsedTime);
          }
          mSystemTimeKeeper.sync(nowSeconds);
        }

        if (mSystemTimeKeeper.mIsSynced) {
          COROUTINE_DELAY_SECONDS(mDelayLoopCounter, mSyncPeriodSeconds);
        } else {
          COROUTINE_DELAY_SECONDS(mDelayLoopCounter, mInitialSyncPeriodSeconds);
        }
      }
    }

  private:
    static const uint8_t kStatusOk = 0;
    static const uint8_t kStatusTimedOut = 1;

    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mSyncPeriodSeconds;
    uint16_t const mInitialSyncPeriodSeconds;
    uint16_t const mRequestTimeoutMillis;
    common::TimingStats* const mTimingStats;

    uint16_t mRequestStartTime;
    uint8_t mRequestStatus;
    uint16_t mDelayLoopCounter;
};

}

#endif
