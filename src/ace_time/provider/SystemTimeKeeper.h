#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#include <Arduino.h> // millis()
#include <stdint.h>
#include "../common/TimingStats.h"
#include "../common/logger.h"
#include "TimeKeeper.h"

namespace ace_time {
namespace provider {

/**
 * A TimeKeeper that uses the Arduino millis() function to advance the time
 * returned to the user. The real time is returned as the number of seconds
 * since the AceTime epoch of 2000-01-01T00:00:00Z.
 *
 * The built-in millis() is not accurate, so this class allows a periodic
 * sync using the (presumably) more accurate syncTimeProvider. The current
 * time can be periodically backed up into the backupTimeKeeper which is
 * expected to be an RTC chip that continues to keep time during power loss.
 *
 * The value of the previous system time millis() is stored internally as
 * a uint16_t. That has 2 advantages: 1) it saves memory, 2) the upper bound of
 * the execution time of getNow() limited to 65 iterations. The disadvantage is
 * the that internal counter will rollover within 65.535 milliseconds. To
 * prevent that, getNow() or setNow() must be called more frequently than every
 * 65.536 seconds. This can be satisfied by using the
 * SystemTimeHeartbeatCoroutine or SystemTimeHeartbeatLoop helper classes.
 *
 * There are 2 ways to perform syncing from the syncTimeProvider:
 *
 * 1) Create an instance of SystemTimeSyncCoroutine and register it with the
 * CoroutineSchedule so that it runs periodically. The
 * SystemTimeSyncCoroutine::runCoroutine() method uses the non-blocking
 * sendRequest(), isResponseReady() and readResponse() methods of TimeProvider
 * to retrieve the current time. Some time providers (e.g. NtpTimeProvider) can
 * take 100s of milliseconds to return, so using the coroutine infrastructure
 * allows other coroutines to continue executing.
 *
 * 2) Call the SystemTimeSyncLoop::loop() method from the global loop()
 * function. This method uses the blocking TimeProvider::getNow() method which
 * can take 100s of milliseconds for something like NtpTimeProvider.
 */
class SystemTimeKeeper: public TimeKeeper {
  public:

    /**
     * @param syncTimeProvider The authoritative source of the time. Can be
     * null in which case the objec relies just on millis() and the user
     * to set the proper time using setNow().
     * @param backupTimeKeeper An RTC chip which continues to keep time
     * even when power is lost. Can be null.
     */
    explicit SystemTimeKeeper(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */):
        mSyncTimeProvider(syncTimeProvider),
        mBackupTimeKeeper(backupTimeKeeper) {}

    void setup() {
      if (mBackupTimeKeeper != nullptr) {
        setNow(mBackupTimeKeeper->getNow());
      }
    }

    uint32_t getNow() const override {
      if (!mIsInit) return 0;

      while ((uint16_t) ((uint16_t) millis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mEpochSeconds += 1;
      }
      return mEpochSeconds;
    }

    void setNow(uint32_t epochSeconds) override {
      if (epochSeconds == 0) return;

      mEpochSeconds = epochSeconds;
      mPrevMillis = millis();
      mIsInit = true;
      backupNow(epochSeconds);
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
    void sync(uint32_t epochSeconds) {
      if (epochSeconds == 0) return;
      if (mEpochSeconds == epochSeconds) return;

      mEpochSeconds = epochSeconds;
      mPrevMillis = millis();
      mIsInit = true;
      mLastSyncTime = epochSeconds;

      if (mBackupTimeKeeper != mSyncTimeProvider) {
        backupNow(epochSeconds);
      }
    }

    /**
     * Return the time (seconds since Epoch) of the last valid sync() call.
     * Returns 0 if never synced.
     */
    uint32_t getLastSyncTime() const {
      return mLastSyncTime;
    }

    /** Return true if initialized by setNow() or sync(). */
    bool isInit() const { return mIsInit; }

  protected:
    /** Return the Arduino millis(). Override for unit testing. */
    virtual unsigned long millis() const { return ::millis(); }

  private:
    friend class SystemTimeSyncCoroutine;
    friend class SystemTimeSyncLoop;

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

    const TimeProvider* const mSyncTimeProvider;
    TimeKeeper* const mBackupTimeKeeper;

    mutable uint32_t mEpochSeconds = 0; // time presented to the user
    mutable uint16_t mPrevMillis = 0;  // lower 16-bits of millis()
    bool mIsInit = false; // true if setNow() or sync() was successful
    uint32_t mLastSyncTime = 0; // time when last synced
};

}
}

#endif
