#include <stdint.h>
#include <Arduino.h>
#include <AceTime.h>
#include "Benchmark.h"
#include "ace_time/common/util.h"

using namespace ace_time;
using ace_time::common::printPad3;

#if defined(AVR)
const uint32_t COUNT = 2500;
#elif defined(ESP8266)
const uint32_t COUNT = 25000;
#elif defined(ESP32)
const uint32_t COUNT = 250000;
#elif defined(TEENSYDUINO)
const uint32_t COUNT = 250000;
#elif !defined(ARDUINO)
const uint32_t COUNT = 100000;
#else
  #error Unsupported platform
#endif

const uint32_t MILLIS_TO_NANO_PER_ITERATION = ((uint32_t) 1000000 / COUNT);

const char TOP[] =
  "+--------------------------------------------------+----------+";
const char HEADER[] =
  "| Method                                           |   micros |";
const char ROW_DIVIDER[] =
  "|--------------------------------------------------|----------|";
const char* const BOTTOM = TOP;
const char COL_DIVIDER[] = " |";
const char EMPTY_LOOP_LABEL[] =
  "| Empty loop                                       | ";
const char LOCAL_DATE_FOR_EPOCH_DAYS_LABEL[] =
  "| LocalDate::forEpochDays()                        | ";
const char LOCAL_DATE_TO_EPOCH_DAYS_LABEL[] =
  "| LocalDate::toEpochDays()                         | ";
const char LOCAL_DATE_DAY_OF_WEEK_LABEL[] =
  "| LocalDate::dayOfWeek()                           | ";

const char OFFSET_DATE_TIME_FOR_EPOCH_SECONDS_LABEL[] =
  "| OffsetDateTime::forEpochSeconds()                | ";
const char OFFSET_DATE_TIME_TO_EPOCH_SECONDS_LABEL[] =
  "| OffsetDateTime::toEpochSeconds()                 | ";

const char DATE_TIME_FOR_EPOCH_SECONDS_LABEL[] =
  "| ZonedDateTime::forEpochSeconds(UTC)              | ";
const char DATE_TIME_FOR_EPOCH_SECONDS_LOS_ANGELES_LABEL[] =
  "| ZonedDateTime::forEpochSeconds(BasicZoneSpec)    | ";
const char DATE_TIME_FOR_EPOCH_SECONDS_CACHED_LABEL[] =
  "| ZonedDateTime::forEpochSeconds(BasicZone cached) | ";
const char DATE_TIME_TO_EPOCH_DAYS_LABEL[] =
  "| ZonedDateTime::toEpochDays()                     | ";
const char DATE_TIME_TO_EPOCH_SECONDS_LABEL[] =
  "| ZonedDateTime::toEpochSeconds()                  | ";

// The compiler is extremelly good about removing code that does nothing. This
// volatile variable is used to create side-effects that prevent the compiler
// from optimizing out the code that's being tested. Each disableOptimization()
// method should perform 6 XOR operations to cancel each other out when
// subtracted.
volatile uint8_t guard;

void disableOptimization(const LocalDate& ld) {
  guard ^= ld.year();
  guard ^= ld.month();
  guard ^= ld.day();
  guard ^= ld.year();
  guard ^= ld.month();
  guard ^= ld.day();
}

void disableOptimization(const ZonedDateTime& dt) {
  guard ^= dt.year();
  guard ^= dt.month();
  guard ^= dt.day();
  guard ^= dt.hour();
  guard ^= dt.minute();
  guard ^= dt.second();
}

void disableOptimization(const OffsetDateTime& dt) {
  guard ^= dt.year();
  guard ^= dt.month();
  guard ^= dt.day();
  guard ^= dt.hour();
  guard ^= dt.minute();
  guard ^= dt.second();
}

void disableOptimization(uint32_t value) {
  // Two temp variables allows 2 more XOR operations, for a total of 6.
  uint8_t tmp1, tmp2;

  guard ^= value & 0xff;
  guard ^= (value >> 8) & 0xff;
  guard ^= (tmp1 = (value >> 16) & 0xff);
  guard ^= (tmp2 = (value >> 24) & 0xff);
  guard ^= tmp1;
  guard ^= tmp2;
}

// A small helper that runs the given lamba expression in a loop
// and returns how long it took.
template <typename F>
unsigned long runLambda(uint32_t count, F&& lambda) {
  yield();
  unsigned long startMillis = millis();
  while (count--) {
    lambda();
  }
  unsigned long elapsedMillis = millis() - startMillis;
  yield();
  return elapsedMillis;
}

void printPad3(uint16_t val, char padChar) {
  if (val < 100) Serial.print(padChar);
  if (val < 10) Serial.print(padChar);
  Serial.print(val);
}

void printPad4(uint16_t val, char padChar) {
  if (val < 1000) Serial.print(padChar);
  if (val < 100) Serial.print(padChar);
  if (val < 10) Serial.print(padChar);
  Serial.print(val);
}

/**
 * Given total elapsed time in millis, print micros per iteration as
 * a floating point number (without using floating point operations).
 */
static void printMicrosPerIteration(unsigned long elapsedMillis) {
  unsigned long nanos = elapsedMillis * MILLIS_TO_NANO_PER_ITERATION;
  uint16_t whole = nanos / 1000;
  uint16_t frac = nanos % 1000;
  printPad4(whole, ' ');
  Serial.print('.');
  printPad3(frac, '0');
}

static void runEmptyLoop() {
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    disableOptimization(tickMillis);
  });
  Serial.print(EMPTY_LOOP_LABEL);
  printMicrosPerIteration(emptyLoopMillis);
  Serial.println(COL_DIVIDER);
}

// LocalDate::forEpochDays()
static void runLocalDateForEpochDays() {
  unsigned long localDateForDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long emptyMillis = millis();
    disableOptimization(emptyMillis);
  });
  unsigned long elapsedMillis = localDateForDaysMillis - emptyLoopMillis;

  Serial.print(LOCAL_DATE_FOR_EPOCH_DAYS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// LocalDate::toEpochDays()
static void runLocalDateToEpochDays() {
  unsigned long localDateToEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    acetime_t epochDays = localDate.toEpochDays();
    disableOptimization(epochDays);
  });
  unsigned long forEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });
  unsigned long elapsedMillis = localDateToEpochDaysMillis - forEpochDaysMillis;

  Serial.print(LOCAL_DATE_TO_EPOCH_DAYS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// LocalDate::dayOfWeek()
static void runLocalDateDaysOfWeek() {
  unsigned long localDateDayOfWeekMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    uint8_t dayOfWeek = localDate.dayOfWeek();
    disableOptimization(localDate);
    disableOptimization(dayOfWeek);
  });
  unsigned long forEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });
  unsigned long elapsedMillis = localDateDayOfWeekMillis - forEpochDaysMillis;

  Serial.print(LOCAL_DATE_DAY_OF_WEEK_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// OffsetDateTime::forEpochSeconds()
static void runOffsetDateTimeForEpochSeconds() {
  unsigned long localDateForDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(fakeEpochSeconds);
    disableOptimization(odt);
  });
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long emptyMillis = millis();
    disableOptimization(emptyMillis);
  });
  unsigned long elapsedMillis = localDateForDaysMillis - emptyLoopMillis;

  Serial.print(OFFSET_DATE_TIME_FOR_EPOCH_SECONDS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// OffsetDateTime::toEpochSeconds()
static void runOffsetDateTimeToEpochSeconds() {
  unsigned long localDateToEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(fakeEpochSeconds);
    acetime_t epochDays = odt.toEpochSeconds();
    disableOptimization(epochDays);
  });
  unsigned long forEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(fakeEpochSeconds);
    disableOptimization(odt);
  });
  unsigned long elapsedMillis = localDateToEpochDaysMillis - forEpochDaysMillis;

  Serial.print(OFFSET_DATE_TIME_TO_EPOCH_SECONDS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// ZonedDateTime::forEpochSeconds(seconds)
static void runZonedDateTimeForEpochSeconds() {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long emptyMillis = millis();
    disableOptimization(emptyMillis);
  });
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// ZonedDateTime::toEpochDays()
static void runZonedDateTimeToEpochDays() {
  unsigned long toEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays);
    acetime_t epochDays = dateTime.toEpochDays();
    disableOptimization(epochDays);
  });
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays);
    disableOptimization(dateTime);
  });
  unsigned long elapsedMillis = toEpochDaysMillis - forEpochSecondsMillis;

  Serial.print(DATE_TIME_TO_EPOCH_DAYS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// ZonedDateTime::toEpochSeconds()
static void runZonedDateTimeToEpochSeconds() {
  unsigned long toEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis);
    acetime_t epochSeconds = dateTime.toEpochSeconds();
    disableOptimization(epochSeconds);
  });
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays);
    disableOptimization(dateTime);
  });
  unsigned long elapsedMillis = toEpochSecondsMillis - forEpochSecondsMillis;

  Serial.print(DATE_TIME_TO_EPOCH_SECONDS_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

// ZonedDateTime::forEpochSeconds(seconds, tz) without cached ZoneSpecifier
static void runZonedDateTimeForEpochSecondsBasicZoneSpecifier() {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
    TimeZone tzLosAngeles = TimeZone::forZoneSpecifier(&zoneSpecifier);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    disableOptimization(fakeEpochSeconds);
  });
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_LOS_ANGELES_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

static BasicZoneSpecifier spec(&zonedb::kZoneAmerica_Los_Angeles);

// ZonedDateTime::forEpochSeconds(seconds, tz) w/ cached ZoneSpecifier
static void runZonedDateTimeForEpochSecondsBasicZoneSpecifierCached() {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    TimeZone tzLosAngeles = TimeZone::forZoneSpecifier(&spec);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    disableOptimization(fakeEpochSeconds);
  });
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_CACHED_LABEL);
  printMicrosPerIteration(elapsedMillis);
  Serial.println(COL_DIVIDER);
}

void runBenchmarks() {
  Serial.println(TOP);
  Serial.println(HEADER);
  Serial.println(ROW_DIVIDER);

  runEmptyLoop();
  Serial.println(ROW_DIVIDER);

  runLocalDateForEpochDays();
  runLocalDateToEpochDays();
  runLocalDateDaysOfWeek();

  runOffsetDateTimeForEpochSeconds();
  runOffsetDateTimeToEpochSeconds();

  runZonedDateTimeForEpochSeconds();
  runZonedDateTimeForEpochSecondsBasicZoneSpecifier();
  runZonedDateTimeForEpochSecondsBasicZoneSpecifierCached();
  runZonedDateTimeToEpochSeconds();
  runZonedDateTimeToEpochDays();

  // End footer
  Serial.println(BOTTOM);

  // Print some stats
  Serial.print("Number of iterations per run: ");
  Serial.println(COUNT);
}
