#include "common/Util.h"
#include "common/DateStrings.h"
#include "LocalDate.h"

namespace ace_time {

using common::printPad2;
using common::printPad3;
using common::DateStrings;

// Using 0=Jan offset.
const uint8_t LocalDate::sDayOfWeek[12] = {
  5 /*Jan=31*/,
  1 /*Feb=28*/,
  0 /*Mar=31, start of "year"*/,
  3 /*Apr=30*/,
  5 /*May=31*/,
  1 /*Jun=30*/,
  3 /*Jul=31*/,
  6 /*Aug=31*/,
  2 /*Sep=30*/,
  4 /*Oct=31*/,
  0 /*Nov=30*/,
  2 /*Dec=31*/,
};

// Using 0=Jan offset.
const uint8_t LocalDate::sDaysInMonth[12] = {
  31 /*Jan=31*/,
  28 /*Feb=28*/,
  31 /*Mar=31*/,
  30 /*Apr=30*/,
  31 /*May=31*/,
  30 /*Jun=30*/,
  31 /*Jul=31*/,
  31 /*Aug=31*/,
  30 /*Sep=30*/,
  31 /*Oct=31*/,
  30 /*Nov=30*/,
  31 /*Dec=31*/,
};

void LocalDate::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid LocalDate>"));
    return;
  }

  // Date
  printer.print(year());
  printer.print('-');
  printPad2(printer, mMonth);
  printer.print('-');
  printPad2(printer, mDay);
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.weekDayLongString(dayOfWeek()));
}

LocalDate& LocalDate::initFromDateString(const char* ds) {
  if (strlen(ds) < kDateStringLength) {
    return setError();
  }

  // year (assumes 4 digit year)
  int16_t year = (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  mYearShort = year - kEpochYear;

  // '-'
  ds++;

  // month
  uint8_t month = (*ds++ - '0');
  month = 10 * month + (*ds++ - '0');
  mMonth = month;

  // '-'
  ds++;

  // day
  uint8_t day = (*ds++ - '0');
  day = 10 * day + (*ds++ - '0');
  mDay = day;

  return *this;
}

}
