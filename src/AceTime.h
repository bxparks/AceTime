/**
 * @mainpage AceTime Library
 *
 * This is the Doxygen documentation for the
 * <a href="https://github.com/bxparks/AceTime">AceTime Library</a>.
 */

#ifndef ACE_TIME_ACE_TIME_H
#define ACE_TIME_ACE_TIME_H

#include "ace_time/common/DateStrings.h"
#include "ace_time/common/ZoneInfo.h"
#include "ace_time/common/ZonePolicy.h"
#include "ace_time/zonedb/zone_policies.h"
#include "ace_time/zonedb/zone_infos.h"
#include "ace_time/zonedbx/zone_policies.h"
#include "ace_time/zonedbx/zone_infos.h"
#include "ace_time/LocalDate.h"
#include "ace_time/local_date_mutation.h"
#include "ace_time/LocalTime.h"
#include "ace_time/LocalDateTime.h"
#include "ace_time/TimeOffset.h"
#include "ace_time/time_offset_mutation.h"
#include "ace_time/OffsetDateTime.h"
#include "ace_time/ZoneSpecifier.h"
#include "ace_time/ManualZoneSpecifier.h"
#include "ace_time/BasicZoneSpecifier.h"
#include "ace_time/ExtendedZoneSpecifier.h"
#include "ace_time/TimeZone.h"
#include "ace_time/ZonedDateTime.h"
#include "ace_time/date_time_mutation.h"
#include "ace_time/TimePeriod.h"
#include "ace_time/provider/TimeProvider.h"
#include "ace_time/provider/TimeKeeper.h"
#include "ace_time/provider/NtpTimeProvider.h"
#include "ace_time/provider/DS3231TimeKeeper.h"
#include "ace_time/provider/SystemTimeKeeper.h"
#include "ace_time/provider/SystemTimeSyncLoop.h"
#include "ace_time/provider/SystemTimeHeartbeatLoop.h"

// activate only if <AceRoutine.h> is included before this header
#ifdef ACE_ROUTINE_VERSION
  #include "ace_time/provider/SystemTimeSyncCoroutine.h"
  #include "ace_time/provider/SystemTimeHeartbeatCoroutine.h"
#endif

// Version format: xxyyzz == "xx.yy.zz"
#define ACE_TIME_VERSION 100
#define ACE_TIME_VERSION_STRING "0.1"

#endif
