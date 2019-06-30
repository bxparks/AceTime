/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include "common/util.h"
#include "common/DateStrings.h"
#include "LocalDateTime.h"

namespace ace_time {

using common::printPad2;
using common::DateStrings;

void LocalDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid LocalDateTime>"));
    return;
  }

  // Date
  printer.print(mLocalDate.year());
  printer.print('-');
  printPad2(printer, mLocalDate.month());
  printer.print('-');
  printPad2(printer, mLocalDate.day());

  // 'T' separator
  printer.print('T');

  // Time
  printPad2(printer, mLocalTime.hour());
  printer.print(':');
  printPad2(printer, mLocalTime.minute());
  printer.print(':');
  printPad2(printer, mLocalTime.second());
}

LocalDateTime LocalDateTime::forDateString(const char* dateString) {
  if (strlen(dateString) < kDateTimeStringLength) {
    return LocalDateTime::forError();
  }
  return forDateStringChainable(dateString);
}


LocalDateTime LocalDateTime::forDateStringChainable(const char*& dateString) {
  const char* s = dateString;

  // date
  LocalDate ld = LocalDate::forDateStringChainable(s);

  // 'T'
  s++;

  // time
  LocalTime lt = LocalTime::forTimeStringChainable(s);

  dateString = s;
  return LocalDateTime(ld, lt);
}

}

