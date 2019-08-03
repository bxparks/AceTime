/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_SYSTEM_CLOCK_H
#define ACE_TIME_SYSTEM_CLOCK_H

#include <stdint.h>
#include "../common/TimingStats.h"
#include "Clock.h"

extern "C" unsigned long millis();
class SystemClockLoopTest;
class SystemClockLoopTest_syncNow;
class SystemClockCoroutineTest;

namespace ace_time {
namespace clock {

/**
 * A Clock that uses the Arduino millis() function to advance the time
 * returned to the user. It has 2 major features:
 *
 *    1) The built-in millis() is not accurate, so this class allows a periodic
 *    sync using the (presumably) more accurate referenceClock.
 *    2) The current time can be periodically backed up into the backupClock
 *    which is expected to be an RTC chip that continues to keep time during
 *    power loss. Upon (re)start, SystemClock::setup() reads back the time from
 *    the backupClock if it exists.
 *
 * There are 2 maintenance tasks which this class must perform peridicallly:
 *
 *    1) The value of the previous system time millis() is stored internally as
 *    a uint16_t. That has 2 advantages: 1) it saves memory, 2) the upper bound
 *    of the execution time of getNow() limited to 65 iterations. The
 *    disadvantage is the that internal counter will rollover within 65.535
 *    milliseconds. To prevent that, keepAlive() must be called more frequently
 *    than every 65.536 seconds.
 *    2) The current time can be synchronized to the referenceClock peridically.
 *    Some reference clocks can take hundreds or thousands of milliseconds to
 *    return, so it's important that the non-block methods of Clock are
 *    used to synchronize to the reference clock.
 *
 * Two subclasses of SystemClock expose 2 different ways of performing these
 * maintenance tasks.
 *
 *    1) Call SystemClockCoroutine::runCoroutine using the framework of
 *    the AceRoutine library in the global loop() function.
 *    2) Call the SystemClockLoop::loop() method from the global loop()
 *    function.
 */
class SystemClock: public Clock {
  public:

    /** Attempt to retrieve the time from the backupClock if it exists. */
    void setup() {
      if (mBackupClock != nullptr) {
        setNow(mBackupClock->getNow());
      }
    }

    acetime_t getNow() const override {
      if (!mIsInit) return kInvalidSeconds;

      while ((uint16_t) ((uint16_t) clockMillis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mEpochSeconds += 1;
      }
      return mEpochSeconds;
    }

    void setNow(acetime_t epochSeconds) override {
      syncNow(epochSeconds);

      // Also set the reference clock if possible.
      if (mReferenceClock != nullptr) {
        mReferenceClock->setNow(epochSeconds);
      }
    }

    /** Force a sync with the mReferenceClock. */
    void forceSync() {
      acetime_t nowSeconds = mReferenceClock->getNow();
      setNow(nowSeconds);
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
    friend class ::SystemClockLoopTest;
    friend class ::SystemClockCoroutineTest;
    friend class ::SystemClockLoopTest_syncNow;

    // disable copy constructor and assignment operator
    SystemClock(const SystemClock&) = delete;
    SystemClock& operator=(const SystemClock&) = delete;

    /**
     * Constructor.
     * @param referenceClock The authoritative source of the time. If this is
     *    null, object relies just on clockMillis() and the user to set the
     *    proper time using setNow().
     * @param backupClock An RTC chip which continues to keep time
     *    even when power is lost. Can be null.
     */
    explicit SystemClock(
          Clock* referenceClock /* nullable */,
          Clock* backupClock /* nullable */):
        mReferenceClock(referenceClock),
        mBackupClock(backupClock) {}

    /**
     * Return the Arduino millis(). Override for unit testing. Named
     * 'clockMillis()' to avoid conflict with Coroutine::millis().
     */
    virtual unsigned long clockMillis() const { return ::millis(); }

    /**
     * Call this (or getNow() every 65.535 seconds or faster to keep the
     * internal counter in sync with millis().
     */
    void keepAlive() {
      getNow();
    }

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
     * backupClock is different from the referenceClock. This prevents us from
     * retrieving the time from the RTC, then saving it right back again, with
     * a drift each time it is saved back.
     *
     * TODO: Implement a more graceful syncNow() algorithm which shifts only a
     * few milliseconds per iteration, and which guarantees that the clock
     * never goes backwards in time.
     */
    void syncNow(acetime_t epochSeconds) {
      if (epochSeconds == kInvalidSeconds) return;
      mLastSyncTime = epochSeconds;
      if (mEpochSeconds == epochSeconds) return;

      mEpochSeconds = epochSeconds;
      mPrevMillis = clockMillis();
      mIsInit = true;

      if (mBackupClock != mReferenceClock) {
        backupNow(epochSeconds);
      }
    }

    Clock* const mReferenceClock;
    Clock* const mBackupClock;

    mutable acetime_t mEpochSeconds = kInvalidSeconds;
    acetime_t mLastSyncTime = kInvalidSeconds; // time when last synced
    mutable uint16_t mPrevMillis = 0;  // lower 16-bits of clockMillis()
    bool mIsInit = false; // true if setNow() or syncNow() was successful
};

}
}

#endif
