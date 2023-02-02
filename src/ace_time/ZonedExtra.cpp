#include "ZonedExtra.h"
#include "TimeZone.h"

namespace ace_time {

ZonedExtra ZonedExtra::forComponents(
    int16_t year, uint8_t month, uint8_t day,
    uint8_t hour, uint8_t minute, uint8_t second,
    const TimeZone& tz, uint8_t fold) {
  auto ldt = LocalDateTime::forComponents(
      year, month, day, hour, minute, second, fold);
  return forLocalDateTime(ldt, tz);
}

// These are defined in the .cpp file to break cyclic dependency.
ZonedExtra ZonedExtra::forEpochSeconds(
    acetime_t epochSeconds,
    const TimeZone& tz) {
  return tz.getZonedExtra(epochSeconds);
}

// These are defined in the .cpp file to break cyclic dependency.
ZonedExtra ZonedExtra::forLocalDateTime(
    const LocalDateTime& ldt,
    const TimeZone& tz) {
  return tz.getZonedExtra(ldt);
}

}
