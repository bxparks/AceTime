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
  printPad2(printer, mYear);
  printer.print('-');
  printPad2(printer, mMonth);
  printer.print('-');
  printPad2(printer, mDay);
  printer.print('T');

  // Time
  printPad2(printer, mHour);
  printer.print(':');
  printPad2(printer, mMinute);
  printer.print(':');
  printPad2(printer, mSecond);

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

  // year
  uint8_t year = (*ds++ - '0') - 2; // subtract 2000
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  mYear = year;

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

  // 'T'
  ds++;

  // hour
  uint8_t hour = (*ds++ - '0');
  hour = 10 * hour + (*ds++ - '0');
  mHour = hour;

  // '-'
  ds++;

  // minute
  uint8_t minute = (*ds++ - '0');
  minute = 10 * minute + (*ds++ - '0');
  mMinute = minute;

  // '-'
  ds++;

  // second
  uint8_t second = (*ds++ - '0');
  second = 10 * second + (*ds++ - '0');
  mSecond = second;

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
