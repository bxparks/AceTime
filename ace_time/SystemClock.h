#ifndef ACE_TIME_SYSTEM_CLOCK_H
#define ACE_TIME_SYSTEM_CLOCK_H

#include <stdint.h>
#include <AceRoutine.h>
#include "TimeKeeper.h"

using namespace ace_routine;

namespace ace_time {

class SystemClock: public TimeKeeper, public Coroutine {
  public:
    explicit SystemClock(TimeKeeper& syncTimeKeeper,
            TimeKeeper& backupTimeKeeper):
        mSyncTimeKeeper(syncTimeKeeper),
        mBackupTimeKeeper(backupTimeKeeper),
        mPrevMillis(0) {}

    virtual void setup() override {
      uint32_t now = mBackupTimeKeeper.getNow();
      setNow(now);
    }

    /**
     * @copydoc TimeKeeper::getNow()
     *
     * The value of the previous system time millis() is stored internally as a
     * uint16_t. This method synchronizes the secondsSinceEpoch with the
     * system time on each call. Therefore, this method (or setNow()) must be
     * called more frequently than the rollover time of a uin16_t, i.e. 65.536
     * seconds.
     *
     * Using a uint16_t internally has two advantages: 1) it saves memory, 2)
     * the upper bound of the run time of this method is automatically limited
     * to 65 iterations.
     */
    virtual uint32_t getNow() const override {
      while ((uint16_t) ((uint16_t) millis() - mPrevMillis) >= 1000) {
        mPrevMillis += 1000;
        mSecondsSinceEpoch += 1;
      }
      return mSecondsSinceEpoch;
    }

    virtual bool isSettable() const override { return true; }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      mSecondsSinceEpoch = secondsSinceEpoch;
      mPrevMillis = millis();
    }

    virtual int run() override {
      uint8_t status;
      uint32_t seconds;
      COROUTINE_LOOP() {
        COROUTINE_AWAIT(mSyncTimeKeeper.pollNow(status, seconds));
        if (status != TimeKeeper::kStatusOk) {
          Serial.println("Invalid status");
        } else if (seconds == 0) {
          Serial.println("Invalid seconds == 0");
        } else {
          Serial.println("Syncing system clock");
          setNow(seconds);
        }
        COROUTINE_DELAY(kSyncingPeriodMillis);
      }
    }

  protected:
    static const uint16_t kSyncingPeriodMillis = 10000;

    virtual unsigned long millis() const { return ::millis(); }
  
  private:
    TimeKeeper& mSyncTimeKeeper;
    TimeKeeper& mBackupTimeKeeper;
    mutable uint32_t mSecondsSinceEpoch;
    mutable uint16_t mPrevMillis;
};

}

#endif
