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

// Blacklist boards using new Arduino API due to incompatibilities. This
// currently includes all megaAVR boards and SAMD21 boards using arduino::samd
// >= 1.8.10. Boards using arduino:samd <= 1.8.9 or SparkFun:samd are fine.
#if defined(ARDUINO_API_VERSION)
#error Platforms using ArduinoCore-API not supported
#endif

#include "zoneinfo/compat.h"
#include "zoneinfo/infos.h"
//
#include "ace_time/common/common.h"
#include "ace_time/common/DateStrings.h"
#include "ace_time/common/DateConv.h"
#include "ace_time/EpochConverterJulian.h"
#include "ace_time/EpochConverterHinnant.h"
#include "ace_time/Epoch.h"
#include "ace_time/LocalDate.h"
#include "ace_time/local_date_mutation.h"
#include "ace_time/LocalTime.h"
#include "ace_time/LocalDateTime.h"
#include "ace_time/TimeOffset.h"
#include "ace_time/time_offset_mutation.h"
#include "ace_time/OffsetDateTime.h"
#include "ace_time/offset_date_time_mutation.h"
//
#include "ace_time/ZoneProcessor.h"
#include "ace_time/BasicZoneProcessor.h"
#include "ace_time/ExtendedZoneProcessor.h"
#include "ace_time/CompleteZoneProcessor.h"
#include "ace_time/ZoneProcessorCache.h"
#include "ace_time/ZoneRegistrar.h"
#include "ace_time/Zone.h"
#include "ace_time/ZoneManager.h"
#include "ace_time/ZoneSorterByName.h"
#include "ace_time/ZoneSorterByOffsetAndName.h"
#include "ace_time/TimeZoneData.h"
#include "ace_time/TimeZone.h"
#include "ace_time/ZonedDateTime.h"
#include "ace_time/zoned_date_time_mutation.h"
#include "ace_time/TimePeriod.h"
#include "ace_time/time_period_mutation.h"
//
#include "zonedb/zone_policies.h"
#include "zonedb/zone_infos.h"
#include "zonedb/zone_registry.h"
#include "zonedbx/zone_policies.h"
#include "zonedbx/zone_infos.h"
#include "zonedbx/zone_registry.h"
#include "zonedbc/zone_policies.h"
#include "zonedbc/zone_infos.h"
#include "zonedbc/zone_registry.h"

// Version format: xxyyzz == "xx.yy.zz"
#define ACE_TIME_VERSION 30000
#define ACE_TIME_VERSION_STRING "3.0.0"

#endif
