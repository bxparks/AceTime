/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <string.h> // strchr(), strncpy(), memcpy()
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

/**
  * Create the time zone abbreviation in dest from the format string
  * (e.g. "P%T", "E%T"), the time zone deltaMinutes (!= 0 means DST), and the
  * replacement letter (e.g. 'S', 'D', '\0' (represented as '-' in the
  * Rule.LETTER entry). If the Zone.RULES column is '-' or 'hh:mm', then
  * 'letter' will be set to '\0' also, although AceTimetools/transformer.py
  * should have detected this condition and filtered that zone out.
  *
  * 1) If the FORMAT contains a '%', then:
  *    1a) If the letter is '\0', then the '%' is removed. This indicates the
  *    Zone.Rule was ('-', 'hh:mm'), or Rule.LETTER was a '-'.
  *    1b) Else the 'letter' is a single letter (e.g. 'S', 'D', etc) from the
  *    Rule.LETTER column, so replace '%' with with the given 'letter'.
  *
  * 2) If the FORMAT contains a '/', then, ignore the 'letter' and just
  * use deltaMinutes in the following way:
  * 2a) If deltaMinutes is 0, pick the first component, i.e. before the '/'.
  * 2b) Else deltaMinutes != 0, pick the second component, i.e. after the '/'.
  *
  * The above algorithm supports the following edge cases from the TZ
  * Database:
  *
  * A) Asia/Dushanbe in 1991 has a ZoneEra with a fixed hh:mm in the RULES
  * and a '/' in the FORMAT, the fixed hh:mm selects the DST abbreviation
  * in FORMAT. (This seems have been fixed in TZDB sometime before 2022g).
  *
  * B) Africa/Johannesburg 1942-1944 where the RULES which contains a
  * reference to named RULEs with DST transitions but there is no '/' or '%'
  * to distinguish between the 2.
  *
  * @param dest destination string buffer
  * @param destSize size of buffer
  * @param format encoded abbreviation, '%' is a character substitution
  * @param deltaMinutes the additional delta minutes std offset
  *    (0 for standard, != 0 for DST)
  * @param letterString the string corrresonding to the LETTER field in the
  * ZoneRule record. It is `nullptr` if ZoneEra.RULES is a '- or an 'hh:mm';
  * an empty string if the ZoneRule.LETTER was a '-'; or a pointer to a
  * non-empty string if ZoneRule.LETTER was a 'S', 'D', 'WAT' and so on. It
  * is possible for `letterString` to be the same buffer as the `dest`
  * string. Therefore we must copy the `letterString` before overwriting
  * `dest`.
  */
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
