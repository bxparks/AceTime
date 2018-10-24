#ifndef ACE_TIME_ZONE_INFO_H
#define ACE_TIME_ZONE_INFO_H

#include <stdint.h>

namespace ace_time {

struct ZoneRule {
  static const ZoneRule* const kUsRules[];

  uint8_t const fromYear; // 0 = 2000
  uint8_t const toYear; // 255 = 2255
  uint8_t const inMonth; // 1 - 12
  uint8_t const onDayOfWeek; // 1 = Sunday, 7 = Saturday
  uint8_t const onDayOfMonth; // 1 - 31
  uint8_t const atHour; // 0-23
  uint8_t const atHourModifier; // 0=w=wall; 1=s=standard; 2=g=u=z=meridian
  uint8_t const offsetHour; // 0 - 3
  uint8_t const letter; // 0=S, 1=D
};

}

#endif
