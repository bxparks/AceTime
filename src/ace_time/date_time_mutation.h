#ifndef ACE_TIME_DATE_TIME_MUTATION_H
#define ACE_TIME_DATE_TIME_MUTATION_H

#include <stdint.h>
#include "common/util.h"
#include "ZonedDateTime.h"

namespace ace_time {
namespace date_time_mutation {

/**
 * @file date_time_mutation.h
 *
 * Methods that mutate a DateTime object.
 *
 * The number of mutation methods of a ZoneDateTime object is basically
 * unlimited, so including them in the ZonedDateTime class would make its API
 * too complex and always incomplete. By extracting them into a separate
 * namespace, we limit the complexity of the ZoneDateTime class and allow
 * additional mutation methods to be added to this namespace by downstream
 * applications.
 *
 * Example:
 *
 * @code{.cpp}
 * ZonedDateTime dt(...);
 * date_time_mutation::incrementDay(dt);
 * @code
 */

/** Increment the year by one within the interval [0, 99]. */
inline void incrementYear(ZonedDateTime& dateTime) {
  int8_t yearTiny = dateTime.yearTiny();
  common::incrementMod(yearTiny, (int8_t) 100);
  dateTime.yearTiny(yearTiny);
}

/** Increment the year by one within the interval [1, 12]. */
inline void incrementMonth(ZonedDateTime& dateTime) {
  uint8_t month = dateTime.month();
  common::incrementMod(month, (uint8_t) 12, (uint8_t) 1);
  dateTime.month(month);
}

/** Increment the day by one within the interval [1, 31]. */
inline void incrementDay(ZonedDateTime& dateTime) {
  uint8_t day = dateTime.day();
  common::incrementMod(day, (uint8_t) 31, (uint8_t) 1);
  dateTime.day(day);
}

/** Increment the hour by one within the interval [0, 23]. */
inline void incrementHour(ZonedDateTime& dateTime) {
  uint8_t hour = dateTime.hour();
  common::incrementMod(hour, (uint8_t) 24);
  dateTime.hour(hour);
}

/** Increment the minute by one within the interval [0, 59]. */
inline void incrementMinute(ZonedDateTime& dateTime) {
  uint8_t minute = dateTime.minute();
  common::incrementMod(minute, (uint8_t) 60);
  dateTime.minute(minute);
}

}
}

#endif
