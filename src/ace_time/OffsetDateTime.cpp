/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Arduino.h> // Print, F(), strncpy_P()
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

OffsetDateTime OffsetDateTime::forDateString(
    const __FlashStringHelper* dateString) {
  // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
  // ESP8266 do not have strlcpy_P(). We need +1 for the '\0' character and
  // another +1 to determine if the dateString is too long to fit.
  char buffer[kDateStringLength + 2];
  strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
  buffer[kDateStringLength + 1] = 0;

  // check if the original F() was too long
  size_t len = strlen(buffer);
  if (len > kDateStringLength) {
    return forError();
  }

  return forDateString(buffer);
}

} // ace_time
