/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <AceCommon.h>
#include "common/DateStrings.h"
#include "LocalDate.h"

using ace_common::printPad2To;

namespace ace_time {

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
  printPad2To(printer, mMonth, '0');
  printer.print('-');
  printPad2To(printer, mDay, '0');
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.dayOfWeekLongString(dayOfWeek()));
}

LocalDate LocalDate::forDateString(const char* dateString) {
  if (strlen(dateString) < kDateStringLength) {
    return forError();
  }
  return forDateStringChainable(dateString);
}

LocalDate LocalDate::forDateStringChainable(const char*& dateString) {
  const char* s = dateString;

  // year (assumes 4 digit year)
  int16_t year = (*s++ - '0');
  year = 10 * year + (*s++ - '0');
  year = 10 * year + (*s++ - '0');
  year = 10 * year + (*s++ - '0');

  // '-'
  s++;

  // month
  uint8_t month = (*s++ - '0');
  month = 10 * month + (*s++ - '0');

  // '-'
  s++;

  // day
  uint8_t day = (*s++ - '0');
  day = 10 * day + (*s++ - '0');

  dateString = s;
  return forComponents(year, month, day);
}

}
