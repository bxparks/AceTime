/**
 * @mainpage AceTime Library
 *
 * This is the Doxygen documentation for the
 * <a href="https://github.com/bxparks/AceTime">AceTime Library</a>.
 */

#ifndef ACE_TIME_ACE_TIME_H
#define ACE_TIME_ACE_TIME_H

#include "ace_time/TimeZone.h"
#include "ace_time/DateTime.h"
#include "ace_time/TimePeriod.h"
#include "ace_time/TimeProvider.h"
#include "ace_time/TimeKeeper.h"
#include "ace_time/NtpTimeProvider.h"
#include "ace_time/DS3231TimeKeeper.h"
#include "ace_time/SystemTimeKeeper.h"
#include "ace_time/SystemTimeLoop.h"

// activate only if <AceRoutine.h> is included before this header
#ifdef ACE_ROUTINE_VERSION
  #include "ace_time/SystemTimeSyncCoroutine.h"
  #include "ace_time/SystemTimeHeartbeatCoroutine.h"
#endif

// Version format: xxyyzz == "xx.yy.zz", 100 == 0.1.0
#define ACE_TIME_VERSION 100

#endif
