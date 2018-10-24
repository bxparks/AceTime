#include "ZoneRule.h"
#include "ZoneInfo.h"

namespace ace_time {

static const ZoneRule kUSRule03 = {
  7 /*fromYear*/,
  255 /*toYear*/,
  3 /*inMonth*/,
  1 /*onDayOfWeek=Sunday*/,
  8 /*onDayOfMonth*/,
  2 /*atHour=2:00*/,
  0 /*atHourModifier=wall*/,
  1 /*offsetHour*/,
  1 /*letter=D*/
};

static const ZoneRule kUSRule11 = {
  7 /*fromYear*/,
  255 /*toYear*/,
  11 /*inMonth*/,
  1 /*onDayOfWeek=Sunday*/,
  1 /*onDayOfMonth*/,
  2 /*atHour=2:00*/,
  0 /*atHourModifier=wall*/,
  0 /*offsetHour*/,
  0 /*letter=S*/
};

const ZoneRule* const ZoneRule::kUsRules[] = {
  &kUSRule03,
  &kUSRule11
};

}
