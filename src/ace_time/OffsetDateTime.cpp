#include "common/Util.h"
#include "common/DateStrings.h"
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
  printer.print(F("20"));
  printPad2(printer, mLocalDate.year());
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

  // ZoneOffset
  mZoneOffset.printTo(printer);
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.weekDayLongString(dayOfWeek()));
}

OffsetDateTime& OffsetDateTime::initFromDateString(const char* ds) {
  if (strlen(ds) < kDateStringLength) {
    return setError();
  }

  // date
  mLocalDate.initFromDateString(ds);
  ds += LocalDate::kDateStringLength;

  // 'T'
  ds++;

  // time
  mLocalTime.initFromTimeString(ds);
  ds += LocalTime::kTimeStringLength;

  // '+' or '-'
  char utcSign = *ds++;
  int8_t sign;
  if (utcSign == '-') {
    sign = -1;
  } else if (utcSign == '+') {
    sign = 1;
  } else {
    return setError();
  }

  // utc hour
  uint8_t utcHour = (*ds++ - '0');
  utcHour = 10 * utcHour + (*ds++ - '0');
  ds++;

  // utc minute
  uint8_t utcMinute = (*ds++ - '0');
  utcMinute = 10 * utcMinute + (*ds++ - '0');
  ds++;

  // create timeZone from (hour, minute)
  mZoneOffset = ZoneOffset::forHourMinute(sign, utcHour, utcMinute);

  // dayOfWeek
  mDayOfWeek = 0;

  return *this;
}

}
