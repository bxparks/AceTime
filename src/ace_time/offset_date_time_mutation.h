/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_OFFSET_DATE_TIME_MUTATION_H
#define ACE_TIME_OFFSET_DATE_TIME_MUTATION_H

#include <stdint.h>
#include <AceCommon.h>
#include "OffsetDateTime.h"

namespace ace_time {
namespace offset_date_time_mutation {

/**
 * @file offset_date_time_mutation.h
 *
 * Methods that mutate an OffsetDateTime object.
 *
 * The number of mutation methods of a OffsetDateTime object is basically
 * unlimited, so including them in the OffsetDateTime class would make its API
 * too complex and always incomplete. By extracting them into a separate
 * namespace, we limit the complexity of the OffsetDateTime class and allow
 * additional mutation methods to be added to this namespace by downstream
 * applications.
 *
 * No validation is performed during the mutation operation. Client code is
 * normally expected to call the toEpochSeconds() method to convert this into
 * an acetime_t, then later convert it back to human-readable components using
 * the forEpochSeconds() factory method.
 *
 * Example:
 *
 * @code{.cpp}
 * OffsetDateTime dt(...);
 * offset_date_time_mutation::incrementDay(dt);
 * @endcode
 */

/** Increment the year by one within the interval [2000, 2099]. */
inline void incrementYear(OffsetDateTime& dateTime) {
  int16_t year = dateTime.year();
  ace_common::incrementModOffset(year, (int16_t) 100, (int16_t) 2000);
  dateTime.year(year);
}

/** Increment the month by one within the interval [1, 12]. */
inline void incrementMonth(OffsetDateTime& dateTime) {
  uint8_t month = dateTime.month();
  ace_common::incrementModOffset(month, (uint8_t) 12, (uint8_t) 1);
  dateTime.month(month);
}

/** Increment the day by one within the interval [1, 31]. */
inline void incrementDay(OffsetDateTime& dateTime) {
  uint8_t day = dateTime.day();
  ace_common::incrementModOffset(day, (uint8_t) 31, (uint8_t) 1);
  dateTime.day(day);
}

/** Increment the hour by one within the interval [0, 23]. */
inline void incrementHour(OffsetDateTime& dateTime) {
  uint8_t hour = dateTime.hour();
  ace_common::incrementMod(hour, (uint8_t) 24);
  dateTime.hour(hour);
}

/** Increment the minute by one within the interval [0, 59]. */
inline void incrementMinute(OffsetDateTime& dateTime) {
  uint8_t minute = dateTime.minute();
  ace_common::incrementMod(minute, (uint8_t) 60);
  dateTime.minute(minute);
}

}
}

#endif
