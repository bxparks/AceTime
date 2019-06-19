#include "common/util.h"
#include "common/DateStrings.h"
#include "LocalDateTime.h"
#include "OffsetDateTime.h"

namespace ace_time {

using common::printPad2;
using common::DateStrings;

void OffsetDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid OffsetDateTime>"));
    return;
  }

  // Date
  printer.print(mLocalDateTime.year());
  printer.print('-');
  printPad2(printer, mLocalDateTime.month());
  printer.print('-');
  printPad2(printer, mLocalDateTime.day());

  // 'T' separator
  printer.print('T');

  // Time
  printPad2(printer, mLocalDateTime.hour());
  printer.print(':');
  printPad2(printer, mLocalDateTime.minute());
  printer.print(':');
  printPad2(printer, mLocalDateTime.second());

  // TimeOffset
  mTimeOffset.printTo(printer);
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.dayOfWeekLongString(dayOfWeek()));
}

OffsetDateTime OffsetDateTime::forDateString(const char* dateString) {
  if (strlen(dateString) < kDateStringLength) {
    return forError();
  }
  return forDateStringChainable(dateString);
}

OffsetDateTime OffsetDateTime::forDateStringChainable(const char*& dateString) {
  const char* s = dateString;

  LocalDateTime ldt = LocalDateTime::forDateStringChainable(s);
  TimeOffset offset = TimeOffset::forOffsetStringChainable(s);

  dateString = s;
  return OffsetDateTime(ldt, offset);
}

}
