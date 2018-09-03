#ifndef ACE_TIME_SYSTEM_SYNC_MANUAL_H
#define ACE_TIME_SYSTEM_SYNC_MANUAL_H

#include <stdint.h>
#include "TimeKeeper.h"

#define ENABLE_SERIAL 0

namespace ace_time {

/**
 * Periodically sync the SystemTimeKeeper from the external sync source. Call
 * the run() method periodically from the global loop() function.
 */
class SystemSyncManual {
  public:
    explicit SystemSyncManual(
            TimeKeeper* systemTimeKeeper,
            TimeKeeper* syncTimeKeeper,
            TimeKeeper* backupTimeKeeper):
        mSystemTimeKeeper(systemTimeKeeper),
        mSyncTimeKeeper(syncTimeKeeper),
        mBackupTimeKeeper(backupTimeKeeper),
        mPrevMillis(0) {}

    /** Call this from the global loop() method. */
    void run() {
      uint16_t nowMillis = millis();
      if (nowMillis - mPrevMillis < kSyncingPeriodMillis) return;
      uint32_t nowSeconds = mSyncTimeKeeper->getNow(); // blocking call
      if (nowSeconds != 0) {
#if ENABLE_SERIAL == 1
        Serial.println("SystemSyncManual: Syncing system time keeper");
#endif
        // TODO: Implement a more graceful SystemTimeKeeper.sync() method
        mSystemTimeKeeper->setNow(nowSeconds);
        if (mBackupTimeKeeper != nullptr
              && mBackupTimeKeeper != mSyncTimeKeeper) {
          mBackupTimeKeeper->setNow(nowSeconds);
        }
      }
      mPrevMillis = nowMillis;
    }

  private:
    static const uint16_t kSyncingPeriodMillis = 30000;

    virtual unsigned long millis() const { return ::millis(); }
 
    TimeKeeper* mSystemTimeKeeper;
    TimeKeeper* mSyncTimeKeeper;
    TimeKeeper* mBackupTimeKeeper;
    uint16_t mPrevMillis;
};

}

#endif
