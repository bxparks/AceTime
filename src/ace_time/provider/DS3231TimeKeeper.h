#ifndef ACE_TIME_DS3231_TIME_KEEPER_H
#define ACE_TIME_DS3231_TIME_KEEPER_H

#include <stdint.h>
#include "../hw/DS3231.h"
#include "../hw/HardwareDateTime.h"
#include "../OffsetDateTime.h"
#include "TimeKeeper.h"

namespace ace_time {
namespace provider {

/**
 * An implementation of TimeKeeper that uses a DS3231 RTC chip.
 */
class DS3231TimeKeeper: public TimeKeeper {
  public:
    explicit DS3231TimeKeeper() {}

    void setup() {}

    acetime_t getNow() const override {
      hw::HardwareDateTime hardwareDateTime;
      mDS3231.readDateTime(&hardwareDateTime);
      return toDateTime(hardwareDateTime).toEpochSeconds();
    }

    void setNow(acetime_t epochSeconds) override {
      if (epochSeconds == kInvalidSeconds) return;

      // TODO: Replace with LocalDateTime
      OffsetDateTime now = OffsetDateTime::forEpochSeconds(epochSeconds);
      mDS3231.setDateTime(toHardwareDateTime(now));
    }

  private:
    /**
     * Convert the HardwareDateTime returned by the DS3231 chip to
     * the OffsetDateTime object using UTC time zone.
     */
    static OffsetDateTime toDateTime(const hw::HardwareDateTime& dt) {
      return OffsetDateTime::forComponents(
          dt.year + LocalDate::kEpochYear, dt.month, dt.day,
          dt.hour, dt.minute, dt.second);
    }

    /**
     * Convert a OffsetDateTime object to a HardwareDateTime object, ignoring
     * time zone. In practice, it will often be most convenient to store the
     * DS3231 as UTC time. Only 2 digits are supported by the year field in the
     * DS3231 so the year is assumed to be between 2000 and 2099.
     */
    static hw::HardwareDateTime toHardwareDateTime(const OffsetDateTime& dt) {
      return hw::HardwareDateTime{(uint8_t) dt.yearTiny(), dt.month(),
          dt.day(), dt.hour(), dt.minute(), dt.second(), dt.dayOfWeek()};
    }

    const hw::DS3231 mDS3231;
};

}
}

#endif
