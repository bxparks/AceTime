/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <string.h> // strchr(), strncpy(), memcpy()
#include <AceCommon.h> // copyReplaceString()
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

void createAbbreviation(
    char* dest,
    uint8_t destSize,
    const char* format,
    uint32_t deltaSeconds,
    const char* letterString) {

  // Check if FORMAT contains a '%'.
  if (strchr(format, '%') != nullptr) {
    // Check if RULES column empty, therefore no 'letter'
    if (letterString == nullptr) {
      strncpy(dest, format, destSize - 1);
      dest[destSize - 1] = '\0';
    } else {
      // Copy `letterString` into a local buffer, in case `letterString` is
      // the same as `dest.
      char letter[internal::kAbbrevSize];
      if (letterString) {
        strncpy(letter, letterString, internal::kAbbrevSize - 1);
        letter[internal::kAbbrevSize - 1] = '\0';
      } else {
        letter[0] = '\0';
      }

      ace_common::copyReplaceString(dest, destSize, format, '%', letter);
    }
  } else {
    // Check if FORMAT contains a '/'.
    const char* slashPos = strchr(format, '/');
    if (slashPos != nullptr) {
      if (deltaSeconds == 0) {
        uint8_t headLength = (slashPos - format);
        if (headLength >= destSize) headLength = destSize - 1;
        memcpy(dest, format, headLength);
        dest[headLength] = '\0';
      } else {
        uint8_t tailLength = strlen(slashPos+1);
        if (tailLength >= destSize) tailLength = destSize - 1;
        memcpy(dest, slashPos+1, tailLength);
        dest[tailLength] = '\0';
      }
    } else {
      // Just copy the FORMAT disregarding deltaSeconds and letterString.
      strncpy(dest, format, destSize - 1);
      dest[destSize - 1] = '\0';
    }
  }
}

} // internal
} // ace_time
