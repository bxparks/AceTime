/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_DATE_TUPLE_H
#define ACE_TIME_DATE_TUPLE_H

#include <stdint.h> // uint8_t
#include "common/logging.h"
#include "local_date_mutation.h"

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG 0
#endif

namespace ace_time {
namespace extended {

/**
 * The result of comparing 2 DateTuples, or compare the transition time of a
 * Transition to the time interval of its corresponding MatchingEra.
 */
enum class MatchStatus : uint8_t {
  kFarPast, // 0
  kPrior, // 1
  kExactMatch, // 2
  kWithinMatch, // 3
  kFarFuture, // 4
};

/**
 * A tuple that represents a date and time. Packed to 4-byte boundaries to
 * save space on 32-bit processors.
 */
struct DateTuple {
  DateTuple() = default;

  DateTuple(int16_t y, uint8_t mon, uint8_t d, int16_t min, uint8_t mod):
      year(y), month(mon), day(d), suffix(mod), minutes(min) {}

  int16_t year; // [-1,10000]
  uint8_t month; // [1,12]
  uint8_t day; // [1,31]
  uint8_t suffix; // kSuffixS, kSuffixW, kSuffixU
  int16_t minutes; // negative values allowed

  /** Used only for debugging. */
  void log() const {
    if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
      int hour = minutes / 60;
      int minute = minutes - hour * 60;
      char c = "wsu"[(suffix>>4)];
      logging::printf("%04d-%02u-%02uT%02d:%02d%c",
          year, month, day, hour, minute, c);
    }
  }
};

/** Determine if DateTuple a is less than DateTuple b, ignoring the suffix. */
inline bool operator<(const DateTuple& a, const DateTuple& b) {
  if (a.year < b.year) return true;
  if (a.year > b.year) return false;
  if (a.month < b.month) return true;
  if (a.month > b.month) return false;
  if (a.day < b.day) return true;
  if (a.day > b.day) return false;
  if (a.minutes < b.minutes) return true;
  if (a.minutes > b.minutes) return false;
  return false;
}

inline bool operator>=(const DateTuple& a, const DateTuple& b) {
  return ! (a < b);
}

inline bool operator<=(const DateTuple& a, const DateTuple& b) {
  return ! (b < a);
}

inline bool operator>(const DateTuple& a, const DateTuple& b) {
  return (b < a);
}

/** Determine if DateTuple a is equal to DateTuple b, including the suffix. */
inline bool operator==(const DateTuple& a, const DateTuple& b) {
  return a.year == b.year
      && a.month == b.month
      && a.day == b.day
      && a.minutes == b.minutes
      && a.suffix == b.suffix;
}

/** Normalize DateTuple::minutes if its magnitude is more than 24 hours. */
inline void normalizeDateTuple(DateTuple* dt) {
  const int16_t kOneDayAsMinutes = 60 * 24;
  if (dt->minutes <= -kOneDayAsMinutes) {
    LocalDate ld = LocalDate::forComponents(dt->year, dt->month, dt->day);
    local_date_mutation::decrementOneDay(ld);
    dt->year = ld.year();
    dt->month = ld.month();
    dt->day = ld.day();
    dt->minutes += kOneDayAsMinutes;
  } else if (kOneDayAsMinutes <= dt->minutes) {
    LocalDate ld = LocalDate::forComponents(dt->year, dt->month, dt->day);
    local_date_mutation::incrementOneDay(ld);
    dt->year = ld.year();
    dt->month = ld.month();
    dt->day = ld.day();
    dt->minutes -= kOneDayAsMinutes;
  } else {
    // do nothing
  }
}

/**
 * Return the number of seconds in (a - b), ignoring suffix. This function is
 * valid for all years [1, 10000), regardless of the Epoch::currentEpochYear(),
 * as long as the difference between the two DateTuples fits inside an
 * `acetime_t`, which is a signed 32-bit integer.
 */
inline acetime_t subtractDateTuple(const DateTuple& a, const DateTuple& b) {
  int32_t epochDaysA = LocalDate::forComponents(
      a.year, a.month, a.day).toEpochDays();

  int32_t epochDaysB = LocalDate::forComponents(
      b.year, b.month, b.day).toEpochDays();

  // Perform the subtraction of the days first, before converting to seconds, to
  // prevent overflow if a.year or b.year is more than 68 years from the current
  // epoch year.
  return (epochDaysA - epochDaysB) * 86400 + (a.minutes - b.minutes) * 60;
}

/**
  * Determine the relationship of t to the time interval defined by `[start,
  * until)`. The comparison is fuzzy, with a slop of about one month so that
  * we can ignore the day and minutes fields.
  *
  * The following values are returned:
  *
  *  * kPrior if 't' is less than 'start' by at least one month,
  *  * kFarFuture if 't' is greater than 'until' by at least one month,
  *  * kWithinMatch if 't' is within [start, until) with a one month slop,
  *  * kExactMatch is never returned.
  */
inline MatchStatus compareDateTupleFuzzy(
    const DateTuple& t,
    const DateTuple& start,
    const DateTuple& until) {
  // Use int32_t because a delta year of 2730 or greater will exceed
  // the range of an int16_t.
  int32_t tMonths = t.year * (int32_t) 12 + t.month;
  int32_t startMonths = start.year * (int32_t) 12 + start.month;
  if (tMonths < startMonths - 1) return MatchStatus::kPrior;
  int32_t untilMonths = until.year * 12 + until.month;
  if (untilMonths + 1 < tMonths) return MatchStatus::kFarFuture;
  return MatchStatus::kWithinMatch;
}

} // namespace extended
} // namespace ace_time

#endif
