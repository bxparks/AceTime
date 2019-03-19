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

  // UtcOffset
  mUtcOffset.printTo(printer);
  printer.print(' ');

  // Week day
  DateStrings ds;
  printer.print(ds.weekDayLongString(dayOfWeek()));
}

OffsetDateTime OffsetDateTime::forDateString(const char* ds) {
  if (strlen(ds) < kDateStringLength) {
    return forError();
  }

  // date
  LocalDateTime ldt = LocalDateTime::forDateString(ds);
  ds += LocalDateTime::kDateTimeStringLength;

  // '+' or '-'
  char utcSign = *ds++;
  int8_t sign;
  if (utcSign == '-') {
    sign = -1;
  } else if (utcSign == '+') {
    sign = 1;
  } else {
    return forError();
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
  UtcOffset offset = UtcOffset::forHourMinute(sign, utcHour, utcMinute);

  return OffsetDateTime(ldt, offset);
}

}
