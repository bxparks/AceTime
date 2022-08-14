/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_TIME_UTILS_H
#define ACE_TIME_TIME_UTILS_H

namespace ace_time {

/**
 * Calculate number of days from today to the next target (month, day). For
 * example, setting (month, day) of (12, 25) returns number of days until
 * Christmas. This function should always return an integer in the interval [0,
 * 365]. In a normal year, the maximum is 364. During a leap year, the maximum
 * is 365.
 */
inline int16_t daysUntil(const LocalDate& today, uint8_t month, uint8_t day) {
  int16_t year = today.year();
  LocalDate target = LocalDate::forComponents(year, month, day);
  if (today.compareTo(target) > 0) {
    target.year(year + 1);
  }
  return target.toEpochDays() - today.toEpochDays();
}

}

#endif
