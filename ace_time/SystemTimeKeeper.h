#ifndef ACE_TIME_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_SYSTEM_TIME_KEEPER_H

#include <stdint.h>

namespace ace_time {

class SystemTimeKeeper: public TimeKeeper {
  public:
    explicit SystemTimeKeeper():
        mPrevMillis(0) {}

    virtual void setup() override {}

    /**
     * @copydoc TimeKeeper::now()
     *
     * The value of the previous system time millis() is stored internally as a
     * uint16_t. This method synchronizes the secondsSinceEpoch with the
     * system time on each call. Therefore, this method must be called more
     * frequently than the rollover time of a uin16_t, i.e. 65.536 seconds.
     * Using a uint16_t internally has two advantages: 1) it saves memory, 2)
     * the upper bound of the run time of this method is automatically limited
     * to 65 iterations.
     */
    virtual uint32_t now() const override {
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

  protected:
    virtual uint32_t millis() const { return ::millis(); }
  
  private:
    mutable uint32_t mSecondsSinceEpoch;
    mutable uint16_t mPrevMillis;
};

}

#endif
