#ifndef ACE_TIME_DS3231_TIME_KEEPER_H
#define ACE_TIME_DS3231_TIME_KEEPER_H

#include <stdint.h>
#include "hw/DS3231.h"
#include "hw/HardwareDateTime.h"
#include "TimeKeeper.h"
#include "DateTime.h"

namespace ace_time {

/**
 * An implementation of TimeKeeper that uses a DS3231 RTC chip.
 */
class DS3231TimeKeeper: public TimeKeeper {
  public:
    explicit DS3231TimeKeeper() {}

    void setup() {}

    virtual uint32_t getNow() const override {
      hw::HardwareDateTime hardwareDateTime;
      mDS3231.readDateTime(&hardwareDateTime);
      return toDateTime(hardwareDateTime).toSecondsSinceEpoch();
    }

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      DateTime now(secondsSinceEpoch);
      mDS3231.setDateTime(toHardwareDateTime(now));
    }

  private:
    /**
     * Convert the HardwareDateTime returned by the DS3231 chip to
     * the DateTime object using UTC time zone.
     */
    static DateTime toDateTime(const hw::HardwareDateTime& dt) {
      return DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    }

    /**
     * Convert a DateTime object to a HardwareDateTime object, ignore time zone.
     * In practice, it will often be most convenient to store the DS3231 as UTC
     * time.
     */
    static hw::HardwareDateTime toHardwareDateTime(const DateTime& dt) {
      return hw::HardwareDateTime{dt.year(), dt.month(), dt.day(), dt.hour(),
          dt.minute(), dt.second(), dt.dayOfWeek()};
    }

    const hw::DS3231 mDS3231;
};

}

#endif
