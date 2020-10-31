/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_PERIOD_MUTATION_H
#define ACE_TIME_TIME_PERIOD_MUTATION_H

#include <stdint.h>
#include <AceCommon.h>
#include "TimePeriod.h"

namespace ace_time {
namespace time_period_mutation {

/**
 * @file time_period_mutation.h
 *
 * Methods that mutate a TimePeriod object.
 *
 * @code{.cpp}
 * TimePeriod period = TimePeriod(1, 2, 3); // 1h2m3s
 * time_offset_mutation::incrementHour(period);
 * @endcode
 */

/** Change the sign of the object. */
inline void negate(TimePeriod& period) {
  period.sign(-period.sign());
}

/** Increment the hour by one, modulo 'limit'. */
inline void incrementHour(TimePeriod& period, uint8_t limit) {
  uint8_t hour = period.hour();
  ace_common::incrementMod(hour, limit);
  period.hour(hour);
}

/** Increment the hour component by one, modulo 24. */
inline void incrementHour(TimePeriod& period) {
  incrementHour(period, (uint8_t) 24);
}

/** Increment the minute by one, modulo 60. */
inline void incrementMinute(TimePeriod& period) {
  uint8_t minute = period.minute();
  ace_common::incrementMod(minute, (uint8_t) 60);
  period.minute(minute);
}


}
}

#endif
