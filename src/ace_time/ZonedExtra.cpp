#include "ZonedExtra.h"
#include "TimeZone.h"

namespace ace_time {

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
