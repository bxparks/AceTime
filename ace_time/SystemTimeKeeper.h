#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#include <Arduino.h> // millis()
#include <stdint.h>
#include "TimeKeeper.h"
#include "common/TimingStats.h"
#include "common/logger.h"

#ifndef ACE_TIME_ENABLE_SERIAL
#define ACE_TIME_ENABLE_SERIAL 0
#endif

namespace ace_time {

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
 * SystemTimeHeartbeatCoroutine or SystemTimeLoop helper classes.
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
 * 2) Call the SystemTimeLoop::loop() method from the global loop() function.
 * This method uses the blocking TimeProvider::getNow() method which can take
 * 100s milliseconds for something like NtpTimeProvider.
 *
 * The SystemTimeLoop::loop() performs both syncing (against the
 * syncTimeProvider) and heartbeat freshening (against the builtin millis()). If
 * you are using Coroutines, you must use both the SystemTimeHeartbeatCoroutine
 * and the SystemTimeSyncCoroutine.
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
      mLastSyncTime = secondsSinceEpoch;

      if (mBackupTimeKeeper != mSyncTimeProvider) {
        backupNow(secondsSinceEpoch);
      }
    }

    /** Return the time (seconds since Epoch) of the last valid sync() call. */
    uint32_t getLastSyncTime() const {
      return mLastSyncTime;
    }

  protected:
    // Override for unit testing.
    virtual unsigned long millis() const { return ::millis(); }

  private:
    friend class SystemTimeSyncCoroutine;
    friend class SystemTimeHeartbeatCoroutine;
    friend class SystemTimeLoop;

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

    mutable uint32_t mSecondsSinceEpoch = 0; // time presented to the user
    mutable uint16_t mPrevMillis = 0;  // lower 16-bits of millis()
    bool mIsSynced = false;
    uint32_t mLastSyncTime = 0; // time when last synced
};

// Enable Coroutines if <AceRoutine.h> is included before this header.

#ifdef ACE_ROUTINE_VERSION

/**
 * A coroutine that syncs the SystemTimeKeeper with its syncTimeProvider.
 */
class SystemTimeSyncCoroutine: public ace_routine::Coroutine {
  public:
    /**
     * Constructor.
     * @param systemTimeKeeper the system time keeper to sync up
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
    virtual int runCoroutine() override {
      if (mSystemTimeKeeper.mSyncTimeProvider == nullptr) return 0;

      COROUTINE_LOOP() {
        // Send request
        mSystemTimeKeeper.mSyncTimeProvider->sendRequest();
        mRequestStartTime = millis();

        // Wait for request
        while (true) {
          if (mSystemTimeKeeper.mSyncTimeProvider->isResponseReady()) {
            mStatus = kStatusOk;
            break;
          }

          uint16_t waitTime = millis() - mRequestStartTime;
          if (waitTime >= mRequestTimeoutMillis) {
            mStatus = kStatusTimedOut;
            break;
          }

          COROUTINE_YIELD();
        }

        // Process the response
        if (mStatus == kStatusOk) {
          uint32_t nowSeconds =
              mSystemTimeKeeper.mSyncTimeProvider->readResponse();
          uint16_t elapsedTime = millis() - mRequestStartTime;
          if (mTimingStats != nullptr) {
            mTimingStats->update(elapsedTime);
          }
          mSystemTimeKeeper.sync(nowSeconds);
        }

        if (mSystemTimeKeeper.mIsSynced) {
          COROUTINE_DELAY_SECONDS(mSyncPeriodSeconds);
        } else {
          COROUTINE_DELAY_SECONDS(mInitialSyncPeriodSeconds);
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
    uint8_t mStatus;
};

/**
 * A coroutine that calls SystemTimeKeeper.getNow() peridically.
 */
class SystemTimeHeartbeatCoroutine: public ace_routine::Coroutine {
  public:
    /**
     * Constructor.
     * @param systemTimeKeeper reference to the SystemTimeKeeper
     * @param heartbeatPeriodMillis milliseconds between calls to getNow()
     */
    SystemTimeHeartbeatCoroutine(SystemTimeKeeper& systemTimeKeeper,
        uint16_t heartbeatPeriodMillis = 5000):
      mSystemTimeKeeper(systemTimeKeeper),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    virtual int runCoroutine() override {
      COROUTINE_LOOP() {
#if ACE_TIME_ENABLE_SERIAL == 1
        common::logger("SystemTimeHeartbeatCoroutine: calling getNow()");
#endif
        mSystemTimeKeeper.getNow();
        COROUTINE_DELAY(mHeartbeatPeriodMillis);
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mHeartbeatPeriodMillis;
};

#endif

/**
 * A class that peridically freshens the SystemTimeKeeper using the heartbeat
 * call to getNow(), and peridically syncs with the syncTimeProvider (if it was
 * provided to the SystemTimeKeeper).
 */
class SystemTimeLoop {
  public:
    SystemTimeLoop(SystemTimeKeeper& systemTimeKeeper,
          uint16_t syncPeriodSeconds = 3600,
          uint16_t heartbeatPeriodMillis = 5000):
      mSystemTimeKeeper(systemTimeKeeper),
      mSyncPeriodSeconds(syncPeriodSeconds),
      mHeartbeatPeriodMillis(heartbeatPeriodMillis) {}

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      unsigned long nowMillis = millis();
      uint32_t timeSinceLastSync = nowMillis - mLastSyncMillis;

      // Make sure that mSecondsSinceEpoch does not fall too far behind.
      if (timeSinceLastSync >= 5000) {
#if ACE_TIME_ENABLE_SERIAL == 1
        common::logger(
            "SystemTimeLoop::loop(): calling SystemTimeKeeper::getNow()");
#endif
        mSystemTimeKeeper.getNow();
      }

      // Synchronize if a TimeProvider is available, and mSyncPeriodSeconds has
      // passed.
      if (mSystemTimeKeeper.mSyncTimeProvider != nullptr) {
        if (timeSinceLastSync >= mSyncPeriodSeconds * (uint32_t) 1000) {
          // blocking call
          uint32_t nowSeconds = mSystemTimeKeeper.mSyncTimeProvider->getNow();
          if (nowSeconds == 0) return;
#if ACE_TIME_ENABLE_SERIAL == 1
            common::logger(
                "SystemTimeLoop::loop(): calling SystemTimeKeeper::sync()");
#endif
          mSystemTimeKeeper.sync(nowSeconds);
          mLastSyncMillis = nowMillis;
        }
      }
    }

  private:
    SystemTimeKeeper& mSystemTimeKeeper;
    uint16_t const mSyncPeriodSeconds;
    uint16_t const mHeartbeatPeriodMillis;
    uint16_t mLastSyncMillis;
};

}

#endif
