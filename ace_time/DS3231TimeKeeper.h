#ifndef ACE_TIME_DS3231_TIME_KEEPER_H
#define ACE_TIME_DS3231_TIME_KEEPER_H

#include <stdint.h>
#include "DateTime.h"

namespace ace_time {

class DS3231TimeKeeper: public TimeKeeper {
  public:
    explicit DS3231TimeKeeper(const DS3231& ds3231):
        mDS3231(ds3231) {}

    virtual void setup() override {}

    virtual uint32_t now() const override {
      DateTime now;
      mDS3231.readDateTime(&now);
      return now.toSecondsSinceEpoch();
    }

    virtual bool isSettable() const override { return true; }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      DateTime now(secondsSinceEpoch);
      mDS3231.setDateTime(now);
    }

  private:
    const DS3231& mDS3231;
};

}

#endif
