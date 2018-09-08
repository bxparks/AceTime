#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#define ENABLE_SERIAL 0

#include <Arduino.h> // millis()
#include <AceRoutine.h>
#include <stdint.h>
#include "TimeKeeper.h"

using namespace ace_routine;

namespace ace_time {

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
     */
    explicit SystemTimeKeeper(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */):
        mSyncTimeProvider(syncTimeProvider),
        mBackupTimeKeeper(backupTimeKeeper),
        mPrevMillis(0) {}

    virtual void setup() override {
      if (mBackupTimeKeeper != nullptr) {
        setNow(mBackupTimeKeeper->getNow());
      }
    }

    virtual uint32_t getNow() const override {
      while ((uint16_t) ((uint16_t) millis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mSecondsSinceEpoch += 1;
      }
      return mSecondsSinceEpoch;
    }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      mSecondsSinceEpoch = secondsSinceEpoch;
      mPrevMillis = millis();
      backupNow(secondsSinceEpoch);
    }

    /**
     * @copydoc Coroutine::runCoroutine()
     *
     * The CoroutineScheduler will use this method if enabled. Don't forget to
     * call setupCoroutine() (inherited from Coroutine) in the global setup() to
     * register this coroutine into the CoroutineScheduler.
     */
    virtual int runCoroutine() override {
      if (mSyncTimeProvider == nullptr) return 0;

#if ENABLE_SERIAL == 1
      static uint16_t startTime;
#endif

      uint8_t status;
      uint32_t nowSeconds;
      COROUTINE_LOOP() {
#if ENABLE_SERIAL == 1
        startTime = millis();
#endif

        COROUTINE_AWAIT(mSyncTimeProvider->pollNow(status, nowSeconds));
        if (status != TimeKeeper::kStatusOk) {
#if ENABLE_SERIAL == 1
          Serial.print("SystemSyncCoroutine: Invalid status: ");
          Serial.println(status);
#endif
        } else if (nowSeconds == 0) {
#if ENABLE_SERIAL == 1
          Serial.println("SystemSyncCoroutine: Invalid nowSeconds == 0");
#endif
        } else {
#if ENABLE_SERIAL == 1
          Serial.print("SystemSyncCoroutine: ok: ");
          Serial.print((uint16_t) (millis() - startTime));
          Serial.println("ms");
#endif
          sync(nowSeconds);
        }
        COROUTINE_DELAY(kSyncingPeriodMillis);
      }
    }

    /**
     * If AceRoutine coroutine infrastructure is not used, then call this from
     * the global loop() method.
     */
    void loop() {
      if (mSyncTimeProvider == nullptr) return;

      uint16_t nowMillis = millis();
      if (nowMillis - mPrevMillis < kSyncingPeriodMillis) return;
      uint32_t nowSeconds = mSyncTimeProvider->getNow(); // blocking call
      if (nowSeconds != 0) {
#if ENABLE_SERIAL == 1
        Serial.println("SystemSyncManual: Syncing system time keeper");
#endif
        sync(nowSeconds);
      }
      mPrevMillis = nowMillis;
    }

  protected:
    // Override for unit testing.
    virtual unsigned long millis() const { return ::millis(); }
  
  private:
    static const uint16_t kSyncingPeriodMillis = 30000;

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
      mSecondsSinceEpoch = secondsSinceEpoch;
      mPrevMillis = millis();
      if (mBackupTimeKeeper != mSyncTimeProvider) {
        backupNow(secondsSinceEpoch);
      }
    }

    const TimeProvider* const mSyncTimeProvider;
    TimeKeeper* const mBackupTimeKeeper;
    mutable uint32_t mSecondsSinceEpoch;
    mutable uint16_t mPrevMillis;
};

}

#endif
