#ifndef ACE_TIME_SYSTEM_SYNC_COROUTINE_H
#define ACE_TIME_SYSTEM_SYNC_COROUTINE_H

#include <stdint.h>
#include <AceRoutine.h>
#include "TimeKeeper.h"

#define ENABLE_SERIAL 0

using namespace ace_routine;

namespace ace_time {

/**
 * Periodically sync the SystemTimeKeeper from the external sync source using
 * AceRoutine coroutine. Call the Coroutine::init() method just before calling
 * CoroutineScheduler::setup() in the global setup() function.
 */
class SystemSyncCoroutine: public Coroutine {
  public:
    explicit SystemSyncCoroutine(
            TimeKeeper* systemTimeKeeper,
            TimeProvider* syncTimeProvider,
            TimeKeeper* backupTimeKeeper /* nullable */):
        mSystemTimeKeeper(systemTimeKeeper),
        mSyncTimeProvider(syncTimeProvider),
        mBackupTimeKeeper(backupTimeKeeper) {}

    virtual int run() override {
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
          // TODO: Implement a more graceful SystemTimeKeeper.sync() method
          mSystemTimeKeeper->setNow(nowSeconds);
          if (mBackupTimeKeeper != nullptr
              && mBackupTimeKeeper != mSyncTimeProvider) {
            mBackupTimeKeeper->setNow(nowSeconds);
          }
        }
        COROUTINE_DELAY(kSyncingPeriodMillis);
      }
    }

  private:
    static const uint16_t kSyncingPeriodMillis = 30000;

    TimeKeeper* mSystemTimeKeeper;
    TimeProvider* mSyncTimeProvider;
    TimeKeeper* mBackupTimeKeeper;
};

}

#endif
