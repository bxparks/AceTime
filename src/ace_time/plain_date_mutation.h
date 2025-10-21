/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_PLAIN_DATE_MUTATION_H
#define ACE_TIME_PLAIN_DATE_MUTATION_H

#include <stdint.h>
#include "PlainDate.h"

namespace ace_time {
namespace plain_date_mutation {

/**
 * Increment PlainDate by one day. Incrementing 9999-12-31 produces 10000-01-01
 * which is not a normal PlainDate because it represents +Infinity.
 */
inline void incrementOneDay(PlainDate& pd) {
  uint8_t day = pd.day() + 1;
  uint8_t month = pd.month();
  int16_t year = pd.year();

  if (day > PlainDate::daysInMonth(pd.year(), month)) {
    day = 1;
    month++;
    if (month > 12) {
      month = 1;
      year++;
    }
  }
  pd.day(day);
  pd.month(month);
  pd.year(year);
}

/**
 * Decrement PlainDate by one day. Decrementing past 0001-01-01 will produce
 * 0000-12-31, which is not a normal PlainDate because it represents -Infinity.
 */
inline void decrementOneDay(PlainDate& pd) {
  uint8_t day = pd.day() - 1;
  uint8_t month = pd.month();
  int16_t year = pd.year();

  if (day == 0) {
    if (month == 1) {
      day = 31;
      month = 12;
      year--;
    } else {
      month--;
      day = PlainDate::daysInMonth(pd.year(), month);
    }
  }
  pd.day(day);
  pd.month(month);
  pd.year(year);
}

}
}

#endif
