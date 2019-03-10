#ifndef ACE_TIME_LOCAL_DATE_MUTATION_H
#define ACE_TIME_LOCAL_DATE_MUTATION_H

#include <stdint.h>
#include "LocalDate.h"

namespace ace_time {
namespace local_date_mutation {

/** Increment LocalDate by one day. */
inline void incrementOneDay(LocalDate& ld) {
  uint8_t day = ld.day() + 1;
  if (day <= LocalDate::daysInMonth(ld.year(), ld.month())) {
    ld.day(day);
    return;
  }

  ld.day(1);
  uint8_t month = ld.month() + 1;
  if (month <= 12) {
    ld.month(month);
    return;
  }

  ld.month(1);
  ld.yearTiny(ld.yearTiny() + 1);
}

/** Decrement LocalDate by one day. */
inline void decrementOneDay(LocalDate& ld) {
  uint8_t day = ld.day() - 1;
  if (day > 0) {
    ld.day(day);
    return;
  }

  if (ld.month() == 1) {
    ld.day(31);
    ld.month(12);
    ld.yearTiny(ld.yearTiny() - 1);
    return;
  }

  uint8_t newMonth = ld.month() - 1;
  ld.day(LocalDate::daysInMonth(ld.year(), newMonth));
  ld.month(newMonth);
}

}
}

#endif
