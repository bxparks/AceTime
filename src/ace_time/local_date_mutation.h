/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_LOCAL_DATE_MUTATION_H
#define ACE_TIME_LOCAL_DATE_MUTATION_H

#include <stdint.h>
#include "LocalDate.h"

namespace ace_time {
namespace local_date_mutation {

/**
 * Increment LocalDate by one day. Incrementing past 2127-12-31 produces
 * an error result whose isError() returns true.
 */
inline void incrementOneDay(LocalDate& ld) {
  uint8_t day = ld.day() + 1;
  uint8_t month = ld.month();
  int16_t year = ld.year();

  if (day > LocalDate::daysInMonth(ld.year(), month)) {
    day = 1;
    month++;
    if (month > 12) {
      month = 1;
      year++;
    }
  }
  ld.day(day);
  ld.month(month);
  ld.year(year);
}

/**
 * Decrement LocalDate by one day. Decrementing past 1873-01-01 produces
 * an error result whose isError() returns true.
 */
inline void decrementOneDay(LocalDate& ld) {
  uint8_t day = ld.day() - 1;
  uint8_t month = ld.month();
  int16_t year = ld.year();

  if (day == 0) {
    if (month == 1) {
      day = 31;
      month = 12;
      year--;
    } else {
      month--;
      day = LocalDate::daysInMonth(ld.year(), month);
    }
  }
  ld.day(day);
  ld.month(month);
  ld.year(year);
}

}
}

#endif
