#include "DateTime.h"
#include "Util.h"
#include "DateStrings.h"

namespace ace_time {

// Print DateTime in ISO8601 format
void DateTime::printTo(Print& printer) const {
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

  // TimeZone
  mTimeZone.printTo(printer);
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.weekDayLongString(dayOfWeek()));
}

void DateTime::init(const char* ds) {
  // verify exact ISO8601 string length
  if (strlen(ds) != kDateStringLength) {
    setError();
    return;
  }

  // year
  uint8_t year = (*ds++ - '0') - 2; // subtract 2000
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  year = 10 * year + (*ds++ - '0');
  mYear = year;
  ds++;

  // month
  uint8_t month = (*ds++ - '0');
  month = 10 * month + (*ds++ - '0');
  mMonth = month;
  ds++;

  // day
  uint8_t day = (*ds++ - '0');
  day = 10 * day + (*ds++ - '0');
  mDay = day;
  ds++;

  // hour
  uint8_t hour = (*ds++ - '0');
  hour = 10 * hour + (*ds++ - '0');
  mHour = hour;
  ds++;

  // minute
  uint8_t minute = (*ds++ - '0');
  minute = 10 * minute + (*ds++ - '0');
  mMinute = minute;
  ds++;

  // second
  uint8_t second = (*ds++ - '0');
  second = 10 * second + (*ds++ - '0');
  mSecond = second;

  // '+' or '-'
  char utcSign = *ds++;
  if (utcSign != '-' && utcSign != '+') {
    setError();
    return;
  }

  // utc hour
  uint8_t utcHour = (*ds++ - '0');
  utcHour = 10 * utcHour + (*ds++ - '0');
  ds++;

  // utc minute
  uint8_t utcMinute = (*ds++ - '0');
  utcMinute = 10 * utcMinute + (*ds++ - '0');
  ds++;

  // Calculate the tzCode (offset from UTC in 15 minute increments)
  uint8_t code = (utcHour * 4) + (utcMinute / 15);
  mTimeZone = TimeZone((utcSign == '+') ? code : -code);

  // dayOfWeek
  mDayOfWeek = 0;
}

}
