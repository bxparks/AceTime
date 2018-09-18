#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#ifndef ACE_TIME_ENABLE_SERIAL
#define ACE_TIME_ENABLE_SERIAL 1
#endif

#include <Arduino.h> // millis()
#include <AceRoutine.h>
#include <stdint.h>
#include "TimeKeeper.h"
#include "common/logger.h"

using namespace ace_routine;

namespace ace_time {

using namespace common;

/**
 * A TimeKeeper that uses the Arduino millis() function to advance the time.
 * The built-in millis() is not accurate, so this class performs a periodic
 * sync using the (presumably) more accurate syncTimeProvider. The current
 * is periodically backed up into the backupTimeKeeper which is expected
 * to be an RTC chip that continues to keep time during power loss.
 *
 * The value of the previous system time millis() is stored internally as
 * a uint16_t. That has 2 advantages: 1) it saves memory, 2) the upper bound of
 * the execution time of getNow() limited to 65 iterations.
 *
 * The disadvantage is the that internal counter will rollover within 65.535
 * milliseconds. To prevent that, getNow(), setNow(), or sync() must be called
 * more frequently than every 65.536 seconds. This can be satisfied by enabling
 * the syncing from syncTimeProvider.
 *
 * There are 2 ways to perform syncing from the syncTimeProvider:
 *
 * 1) Use either the AceRoutine infrastructure by calling setupCoroutine()
 * (inherited from Coroutine) of this object in the global setup() function,
 * which allows the CoroutineScheduler to call SystemTimeKeeper::runCoroutine()
 * periodically. The runCoroutine() method uses the non-blocking
 * TimeProvicer::pollNow() method to retrieve the current time. Some time
 * providers (e.g. NtpTimeProvider) can take 100s of milliseconds to return, so
 * using the coroutine infrastructure allows other coroutines to continue
 * executing.
 *
 * 2) Call the SystemTimeKeeper::loop() method from the global loop() function.
 * This method uses the blocking TimeProvider::getNow() method which can take
 * 100s milliseconds for something like NtpTimeProvider.
 */
class SystemTimeKeeper: public TimeKeeper, public Coroutine {
  public:

    /**
     * @param syncTimeProvider The authoritative source of the time. Can be
     * null in which case the objec relies just on millis() and the user
     * to set the proper time using setNow().
     * @param backupTimeKeeper An RTC chip which continues to keep time
     * even when power is lost. Can be null.
     * @param TimingStats internal statistics
     */
    explicit SystemTimeKeeper(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */,
            uint16_t syncPeriodSeconds = 3600,
            common::TimingStats* timingStats = nullptr):
        mSyncTimeProvider(syncTimeProvider),
        mBackupTimeKeeper(backupTimeKeeper),
        mSyncPeriodSeconds(syncPeriodSeconds),
        mTimingStats(timingStats),
        mPrevMillis(0),
        mIsSynced(false) {}

    virtual void setup() override {
      if (mBackupTimeKeeper != nullptr) {
        setNow(mBackupTimeKeeper->getNow());
      }
    }

    virtual uint32_t getNow() const override {
      if (!mIsSynced) return 0;

      while ((uint16_t) ((uint16_t) millis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mSecondsSinceEpoch += 1;
      }
      return mSecondsSinceEpoch;
    }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      if (secondsSinceEpoch == 0) return;

      mSecondsSinceEpoch = secondsSinceEpoch;
      mPrevMillis = millis();
      mIsSynced = true;
      backupNow(secondsSinceEpoch);
    }

    common::TimingStats* getTimingStats() const {
      return mTimingStats;
    }

    /**
     * @copydoc Coroutine::runCoroutine()
     *
     * The CoroutineScheduler will use this method if enabled. Don't forget to
     * call setupCoroutine() (inherited from Coroutine) in the global setup() to
     * register this coroutine into the CoroutineScheduler.
     */
    virtual int runCoroutine() override {
      static uint16_t startTime;

      if (mSyncTimeProvider == nullptr) return 0;

      uint8_t status;
      uint32_t nowSeconds;
      COROUTINE_LOOP() {
        startTime = millis();
#if ACE_TIME_ENABLE_SERIAL == 1
          logger("=== SystemTimeKeeper::runCoroutine(): sending request");
#endif
        COROUTINE_AWAIT(mSyncTimeProvider->pollNow(status, nowSeconds));
        uint16_t elapsedTime = millis() - startTime;
        if (mTimingStats != nullptr) {
          mTimingStats->update(elapsedTime);
        }

        if (status != TimeKeeper::kStatusOk) {
#if ACE_TIME_ENABLE_SERIAL == 1
          logger("SystemTimeKeeper::runCoroutine(): Invalid status: %u",
              status);
#endif
        } else if (nowSeconds == 0) {
#if ACE_TIME_ENABLE_SERIAL == 1
          logger("SystemTimeKeeper::runCoroutine(): Invalid nowSeconds == 0");
#endif
        } else {
#if ACE_TIME_ENABLE_SERIAL == 1
          logger("SystemTimeKeeper::runCoroutine(): status ok");
          sync(nowSeconds);
#endif
        }
#if ACE_TIME_ENABLE_SERIAL == 1
        logger("SystemTimeKeeper::runCoroutine; %u ms", elapsedTime);
#endif


#if ACE_TIME_ENABLE_SERIAL == 1
        if (mTimingStats != nullptr) {
          logger("SystemTimeKeeper::runCoroutine(): "
                 "min/avg/max: %u/%u/%u; count: %u",
                 mTimingStats->getMin(),
                 mTimingStats->getAvg(),
                 mTimingStats->getMax(),
                 mTimingStats->getCount());

          if (mTimingStats->getCount() >= 10) {
            mTimingStats->reset();
          }
        }
#endif

        // Wait for mSyncPeriodSeconds.
        static uint16_t i;
        for (i = 0; i < mSyncPeriodSeconds; i++) {
          COROUTINE_DELAY(1000);
        }
      }
    }

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      if (mSyncTimeProvider == nullptr) return;

      unsigned long nowMillis = millis();
      uint32_t timeSinceLastSync = nowMillis - mLastSyncMillis;
      if (timeSinceLastSync < mSyncPeriodSeconds * (uint32_t) 1000) return;

      uint32_t nowSeconds = mSyncTimeProvider->getNow(); // blocking call
      if (nowSeconds == 0) return;

#if ACE_TIME_ENABLE_SERIAL == 1
      Serial.println("SystemTimeKeeper::loop(): Syncing system time keeper");
#endif
      sync(nowSeconds);
      mLastSyncMillis = nowMillis;
    }

  protected:
    // Override for unit testing.
    virtual unsigned long millis() const { return ::millis(); }
  
  private:
    /**
     * Write the nowSeconds to the backup TimeKeeper (which can be an RTC that
     * has non-volatile memory, or simply flash memory which emulates a backup
     * TimeKeeper.
     */
    void backupNow(uint32_t nowSeconds) {
      if (mBackupTimeKeeper != nullptr) {
        mBackupTimeKeeper->setNow(nowSeconds);
      }
    }

    /**
     * Similar to setNow() except that backupNow() is called only if the
     * backupTimeKeeper is different from the syncTimeKeeper. This prevents us
     * from retrieving the time from the RTC, then saving it right back again,
     * with a drift each time it is saved back.
     *
     * TODO: Implement a more graceful sync() algorithm which shifts only a few
     * milliseconds per iteration, and which guarantees that the clock never
     * goes backwards in time.
     */
    void sync(uint32_t secondsSinceEpoch) {
      if (secondsSinceEpoch == 0) return;
      if (mSecondsSinceEpoch == secondsSinceEpoch) return;

      mSecondsSinceEpoch = secondsSinceEpoch;
      mPrevMillis = millis();
      mIsSynced = true;

      if (mBackupTimeKeeper != mSyncTimeProvider) {
        backupNow(secondsSinceEpoch);
      }
    }

    const TimeProvider* const mSyncTimeProvider;
    TimeKeeper* const mBackupTimeKeeper;
    uint16_t mSyncPeriodSeconds;
    TimingStats* const mTimingStats;

    mutable uint32_t mSecondsSinceEpoch; // time presented to the user
    mutable uint16_t mPrevMillis;  // lower 16-bits of millis()
    mutable uint16_t mLastSyncMillis;
    bool mIsSynced;
};

}

#endif
