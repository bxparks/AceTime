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

  // Week day
  DateStrings ds;
  printer.print(ds.weekDayLongString(dayOfWeek()));
}

LocalDateTime LocalDateTime::forDateString(const char* ds) {
  if (strlen(ds) < kDateTimeStringLength) {
    return LocalDateTime::forError();
  }

  // date
  LocalDate ld = LocalDate::forDateString(ds);
  ds += LocalDate::kDateStringLength;

  // 'T'
  ds++;

  // time
  LocalTime lt = LocalTime::forTimeString(ds);

  return LocalDateTime(ld, lt);
}

}

