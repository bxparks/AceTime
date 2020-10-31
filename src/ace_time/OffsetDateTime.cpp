/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include "LocalDateTime.h"
#include "OffsetDateTime.h"

namespace ace_time {

void OffsetDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid OffsetDateTime>"));
    return;
  }

  // LocalDateTime
  mLocalDateTime.printTo(printer);

  // TimeOffset "+/-hh:mm
  mTimeOffset.printTo(printer);
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
