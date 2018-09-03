#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#include <stdint.h>
#include "TimeKeeper.h"

namespace ace_time {

/**
 * A TimeKeeper that uses the Arduino millis() function as the source of its
 * time. The value of the previous system time millis() is stored internally as
 * a uint16_t. That has 2 advantages: 1) it saves memory, 2) the upper bound of
 * the execution time of getNow() limited to 65 iterations.
 *
 * The disadvantage is the that internal counter will rollover within 65.535
 * milliseconds. To prevent that, getNow() or setNow() must be called more
 * frequently than 65.536 seconds. Using either the SystemSyncManual or
 * SystemSyncCoroutine helper classes will automatically satisfy this
 * requirement.
 */
class SystemTimeKeeper: public TimeKeeper {
  public:
    explicit SystemTimeKeeper():
        mPrevMillis(0) {}

    virtual void setup() override {}

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
    }

  protected:
    virtual unsigned long millis() const { return ::millis(); }
  
  private:
    mutable uint32_t mSecondsSinceEpoch;
    mutable uint16_t mPrevMillis;
};

}

#endif
