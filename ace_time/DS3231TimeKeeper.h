#ifndef ACE_TIME_DS3231_TIME_KEEPER_H
#define ACE_TIME_DS3231_TIME_KEEPER_H

#include <stdint.h>
#include <AceHardware.h>
#include "TimeKeeper.h"
#include "DateTime.h"

using namespace ace_hardware;
using namespace ace_hardware::ds3231;

namespace ace_time {

class DS3231TimeKeeper: public TimeKeeper {
  public:
    explicit DS3231TimeKeeper(const DS3231& ds3231):
        mDS3231(ds3231) {}

    virtual void setup() override {}

    virtual uint32_t getNow() const override {
      HardwareDateTime hardwareDateTime;
      mDS3231.readDateTime(&hardwareDateTime);
      return toDateTime(hardwareDateTime).toSecondsSinceEpoch();
    }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      DateTime now(secondsSinceEpoch);
      mDS3231.setDateTime(toHardwareDateTime(now));
    }

  private:
    static DateTime toDateTime(const HardwareDateTime& dt) {
      return DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    }

    static HardwareDateTime toHardwareDateTime(const DateTime& dt) {
      return HardwareDateTime{dt.year(), dt.month(), dt.day(), dt.hour(),
          dt.minute(), dt.second(), dt.dayOfWeek()};
    }

    const DS3231& mDS3231;
};

}

#endif
