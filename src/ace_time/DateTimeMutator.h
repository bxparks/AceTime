#ifndef ACE_TIME_DATE_TIME_MUTATOR_H
#define ACE_TIME_DATE_TIME_MUTATOR_H

#include <stdint.h>
#include "common/util.h"

namespace ace_time {

/**
 * A wrapper class that provides mutation operations on a DateTime object. The
 * recommended usage is to create a temporary object which wraps the target
 * DateTime object, then call the desired operation. The temporary object will
 * automatically be destroyed. Optimizing compilers should optimize out the
 * temporary object completely, thereby providing a zero-overhead abstraction.
 *
 * Using a separate mutator object provides 2 benefits. 1) It reduces the
 * complexity of the API provided by the various DateTime classes. 2) It
 * collects mutation operations in a single place which can be updated or
 * modified as needed.
 *
 * The incrementXxx() methods are convenience methods to allow the user to
 * change the date and time using just two buttons. The user is expected to
 * select a specific DateTime component using one of the buttons, then press
 * the other button to increment it.
 * 
 * Example:
 *
 * @code{.cpp}
 * ZonedDateTime dt(...);
 * DateTimeMutator(dt).incrementDay();
 * @code
 */
class DateTimeMutator {
  public:
    /** Constructor */
    DateTimeMutator(ZonedDateTime& dt):
      mDateTime(dt) {}

    /** Increment the year by one, wrapping from 99 to 0. */
    void incrementYear() {
      int8_t yearTiny = mDateTime.yearTiny();
      common::incrementMod(yearTiny, (int8_t) 100);
      mDateTime.yearTiny(yearTiny);
    }

    /** Increment the year by one, wrapping from 12 to 1. */
    void incrementMonth() {
      uint8_t month = mDateTime.month();
      common::incrementMod(month, (uint8_t) 12, (uint8_t) 1);
      mDateTime.month(month);
    }

    /** Increment the day by one, wrapping from 31 to 1. */
    void incrementDay() {
      uint8_t day = mDateTime.day();
      common::incrementMod(day, (uint8_t) 31, (uint8_t) 1);
      mDateTime.day(day);
    }

    /** Increment the hour by one, wrapping from 23 to 0. */
    void incrementHour() {
      uint8_t hour = mDateTime.hour();
      common::incrementMod(hour, (uint8_t) 24);
      mDateTime.hour(hour);
    }

    /** Increment the minute by one, wrapping from 59 to 0. */
    void incrementMinute() {
      uint8_t minute = mDateTime.minute();
      common::incrementMod(minute, (uint8_t) 60);
      mDateTime.minute(minute);
    }

  private:
    ZonedDateTime& mDateTime;
};

}

#endif
