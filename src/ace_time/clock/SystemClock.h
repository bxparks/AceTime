/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_H
#define ACE_TIME_SYSTEM_CLOCK_H

#include <stdint.h>
#include "../common/TimingStats.h"
#include "TimeKeeper.h"

extern "C" unsigned long millis();

namespace ace_time {
namespace clock {

/**
 * A TimeKeeper that uses the Arduino millis() function to advance the time
 * returned to the user. The real time is returned as the number of seconds
 * since the AceTime epoch of 2000-01-01T00:00:00Z.
 *
 * The built-in millis() is not accurate, so this class allows a periodic
 * sync using the (presumably) more accurate referenceClock. The current
 * time can be periodically backed up into the backupClock which is
 * expected to be an RTC chip that continues to keep time during power loss.
 *
 * The value of the previous system time millis() is stored internally as
 * a uint16_t. That has 2 advantages: 1) it saves memory, 2) the upper bound of
 * the execution time of getNow() limited to 65 iterations. The disadvantage is
 * the that internal counter will rollover within 65.535 milliseconds. To
 * prevent that, keepAlive() must be called more frequently than every 65.536
 * seconds. The easiest way to do this is to call it from the global loop()
 * method.
 *
 * There are 2 ways to perform syncing from the referenceClock:
 *
 * 1) Create an instance of SystemClockSyncCoroutine and register it with the
 * CoroutineSchedule so that it runs periodically. The
 * SystemClockSyncCoroutine::runCoroutine() method uses the non-blocking
 * sendRequest(), isResponseReady() and readResponse() methods of TimeProvider
 * to retrieve the current time. Some time providers (e.g. NtpTimeProvider) can
 * take 100s of milliseconds to return, so using the coroutine infrastructure
 * allows other coroutines to continue executing.
 *
 * 2) Call the SystemClockSyncLoop::loop() method from the global loop()
 * function. This method uses the blocking TimeProvider::getNow() method which
 * can take O(100) milliseconds for something like NtpTimeProvider.
 */
class SystemClock: public TimeKeeper {
  public:

    /** Attempt to retrieve the time from the backupClock if it exists. */
    void setup() {
      if (mBackupClock != nullptr) {
        setNow(mBackupClock->getNow());
      }
    }

    /**
     * Call this (or getNow() every 65.535 seconds or faster to keep the
     * internal counter in sync with millis().
     */
    void keepAlive() {
      getNow();
    }

    acetime_t getNow() const override {
      if (!mIsInit) return kInvalidSeconds;

      while ((uint16_t) ((uint16_t) millis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mEpochSeconds += 1;
      }
      return mEpochSeconds;
    }

    void setNow(acetime_t epochSeconds) override {
      if (epochSeconds == kInvalidSeconds) return;

      mEpochSeconds = epochSeconds;
      mPrevMillis = millis();
      mIsInit = true;
      mLastSyncTime = epochSeconds;
      backupNow(epochSeconds);
      // TODO: If timeProvider is a timeKeeper, then set the external
      // timeKeeper as well.
    }

    /** Force a sync with the mReferenceClock. */
    void forceSync() {
      acetime_t nowSeconds = mReferenceClock->getNow();
      syncNow(nowSeconds);
    }

    /**
     * Return the time (seconds since Epoch) of the last valid sync() call.
     * Returns kInvalidSeconds if never synced.
     */
    acetime_t getLastSyncTime() const {
      return mLastSyncTime;
    }

    /** Return true if initialized by setNow() or syncNow(). */
    bool isInit() const { return mIsInit; }

  protected:
    // disable copy constructor and assignment operator
    SystemClock(const SystemClock&) = delete;
    SystemClock& operator=(const SystemClock&) = delete;

    /** Return the Arduino millis(). Override for unit testing. */
    virtual unsigned long millis() const { return ::millis(); }

    /**
     * @param referenceClock The authoritative source of the time. Can be
     * null in which case the objec relies just on millis() and the user
     * to set the proper time using setNow().
     * @param backupClock An RTC chip which continues to keep time
     * even when power is lost. Can be null.
     */
    explicit SystemClock(
          TimeProvider* referenceClock /* nullable */,
          TimeKeeper* backupClock /* nullable */):
        mReferenceClock(referenceClock),
        mBackupClock(backupClock) {}

    /**
     * Write the nowSeconds to the backupClock (which can be an RTC that has
     * non-volatile memory, or simply flash memory which emulates a backupClock.
     */
    void backupNow(acetime_t nowSeconds) {
      if (mBackupClock != nullptr) {
        mBackupClock->setNow(nowSeconds);
      }
    }

    /**
     * Similar to setNow() except that backupNow() is called only if the
     * backupClock is different from the syncTimeKeeper. This prevents us
     * from retrieving the time from the RTC, then saving it right back again,
     * with a drift each time it is saved back.
     *
     * TODO: Implement a more graceful syncNow() algorithm which shifts only a
     * few milliseconds per iteration, and which guarantees that the clock
     * never goes backwards in time.
     */
    void syncNow(acetime_t epochSeconds) {
      if (epochSeconds == kInvalidSeconds) return;
      if (mEpochSeconds == epochSeconds) return;

      mEpochSeconds = epochSeconds;
      mPrevMillis = millis();
      mIsInit = true;
      mLastSyncTime = epochSeconds;

      if (mBackupClock != mReferenceClock) {
        backupNow(epochSeconds);
      }
    }

    const TimeProvider* const mReferenceClock;
    TimeKeeper* const mBackupClock;

    mutable acetime_t mEpochSeconds = 0; // time presented to the user
    mutable uint16_t mPrevMillis = 0;  // lower 16-bits of millis()
    bool mIsInit = false; // true if setNow() or syncNow() was successful
    acetime_t mLastSyncTime = kInvalidSeconds; // time when last synced
};

}
}

#endif
