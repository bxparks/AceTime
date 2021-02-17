/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include "ZoneProcessor.h"

namespace ace_time {
namespace internal {

MonthDay calcStartDayOfMonth(int16_t year, uint8_t month,
    uint8_t onDayOfWeek, int8_t onDayOfMonth) {
  if (onDayOfWeek == 0) return {month, (uint8_t) onDayOfMonth};

  if (onDayOfMonth >= 0) {
    // Convert "last{Xxx}" to "last{Xxx}>={daysInMonth-6}".
    uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
    if (onDayOfMonth == 0) {
      onDayOfMonth =  daysInMonth - 6;
    }

    auto limitDate = LocalDate::forComponents(year, month, onDayOfMonth);
    uint8_t dayOfWeekShift = (onDayOfWeek - limitDate.dayOfWeek() + 7) % 7;
    uint8_t day = (uint8_t) (onDayOfMonth + dayOfWeekShift);
    if (day > daysInMonth) {
      // TODO: Support shifting from Dec to Jan of following  year.
      day -= daysInMonth;
      month++;
    }
    return {month, day};
  } else {
    onDayOfMonth = -onDayOfMonth;
    auto limitDate = LocalDate::forComponents(year, month, onDayOfMonth);
    int8_t dayOfWeekShift = (limitDate.dayOfWeek() - onDayOfWeek + 7) % 7;
    int8_t day = onDayOfMonth - dayOfWeekShift;
    if (day < 1) {
      // TODO: Support shifting from Jan to Dec of the previous year.
      month--;
      uint8_t daysInPrevMonth = LocalDate::daysInMonth(year, month);
      day += daysInPrevMonth;
    }
    return {month, (uint8_t) day};
  }
}

} // internal
} // ace_time
