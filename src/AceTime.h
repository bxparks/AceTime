/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

/**
 * @mainpage AceTime Library
 *
 * This is the Doxygen documentation for the
 * <a href="https://github.com/bxparks/AceTime">AceTime Library</a>.
 *
 * Click on the "Classes" menu above to see the list of classes.
 *
 * Click on the "Files" menu above to see the list of header files.
 */

#ifndef ACE_TIME_ACE_TIME_H
#define ACE_TIME_ACE_TIME_H

#include "ace_time/common/compat.h"
#include "ace_time/common/common.h"
#include "ace_time/common/DateStrings.h"
#include "ace_time/internal/ZoneContext.h"
#include "ace_time/internal/ZoneInfo.h"
#include "ace_time/internal/ZonePolicy.h"
#include "ace_time/zonedb/zone_policies.h"
#include "ace_time/zonedb/zone_infos.h"
#include "ace_time/zonedb/zone_registry.h"
#include "ace_time/zonedbx/zone_policies.h"
#include "ace_time/zonedbx/zone_infos.h"
#include "ace_time/zonedbx/zone_registry.h"
#include "ace_time/ZoneRegistrar.h"
#include "ace_time/LocalDate.h"
#include "ace_time/local_date_mutation.h"
#include "ace_time/LocalTime.h"
#include "ace_time/LocalDateTime.h"
#include "ace_time/TimeOffset.h"
#include "ace_time/time_offset_mutation.h"
#include "ace_time/OffsetDateTime.h"
#include "ace_time/ZoneProcessor.h"
#include "ace_time/BasicZoneProcessor.h"
#include "ace_time/ExtendedZoneProcessor.h"
#include "ace_time/ZoneProcessorCache.h"
#include "ace_time/ZoneManager.h"
#include "ace_time/TimeZoneData.h"
#include "ace_time/TimeZone.h"
#include "ace_time/BasicZone.h"
#include "ace_time/ExtendedZone.h"
#include "ace_time/ZonedDateTime.h"
#include "ace_time/zoned_date_time_mutation.h"
#include "ace_time/TimePeriod.h"
#include "ace_time/time_period_mutation.h"
#include "ace_time/clock/Clock.h"
#include "ace_time/clock/NtpClock.h"
#include "ace_time/clock/DS3231Clock.h"
#include "ace_time/clock/UnixClock.h"
#include "ace_time/clock/SystemClock.h"
#include "ace_time/clock/SystemClockLoop.h"
#include "ace_time/clock/SystemClockCoroutine.h"

// Version format: xxyyzz == "xx.yy.zz"
#define ACE_TIME_VERSION 10200
#define ACE_TIME_VERSION_STRING "1.2"

#endif
