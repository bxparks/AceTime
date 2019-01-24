#include <stdint.h>
#include <Arduino.h>
#include <AceTime.h>
#include "Benchmark.h"
#include "ace_time/common/util.h"

using namespace ace_time;
using ace_time::common::printPad3;

#if defined(AVR)
const uint32_t COUNT = 5000;
#elif defined(ESP8266)
const uint32_t COUNT = 25000;
#else
const uint32_t COUNT = 200000;
#endif

const uint32_t MILLIS_TO_NANO_PER_ITERATION = (1000000 / COUNT);

const char TOP[] =
  "---------------------------------------------+----------+";
const char HEADER[] =
  "Method                                       |   micros |";
const char DIVIDER[] =
  "---------------------------------------------|----------|";
const char BOTTOM[] =
  "---------------------------------------------+----------+";
const char ENDING[] = " |";
const char EMPTY_LOOP_LABEL[] =
  "Empty loop                                   | ";
const char LOCAL_DATE_FOR_EPOCH_DAYS_LABEL[] =
  "LocalDate::forEpochDays()                    | ";
const char LOCAL_DATE_TO_EPOCH_DAYS_LABEL[] =
  "LocalDate::toEpochDays()                     | ";
const char LOCAL_DATE_DAY_OF_WEEK_LABEL[] =
  "LocalDate::dayOfWeek()                       | ";

const char DATE_TIME_FOR_EPOCH_SECONDS_LABEL[] =
  "ZonedDateTime::forEpochSeconds(UTC)          | ";
const char DATE_TIME_FOR_EPOCH_SECONDS_LOS_ANGELES_LABEL[] =
  "ZonedDateTime::forEpochSeconds(Los_Angeles)  | ";
const char DATE_TIME_FOR_EPOCH_SECONDS_CACHED_LABEL[] =
  "ZonedDateTime::forEpochSeconds(Cached)       | ";
const char DATE_TIME_TO_EPOCH_DAYS_LABEL[] =
  "ZonedDateTime::toEpochDays()                 | ";
const char DATE_TIME_TO_EPOCH_SECONDS_LABEL[] =
  "ZonedDateTime::toEpochSeconds()              | ";

// The compiler is extremelly good about removing code that does nothing. This
// variable is used to ensure user-visible side-effects, preventing the compiler
// optimization.
uint8_t guard;

void disableOptimization(const ZonedDateTime& dt) {
  guard ^= dt.year();
  guard ^= dt.month();
  guard ^= dt.day();
  guard ^= dt.hour();
  guard ^= dt.minute();
  guard ^= dt.second();
}

void disableOptimization(const LocalDate& ld) {
  guard ^= ld.year();
  guard ^= ld.month();
  guard ^= ld.day();
}

void disableOptimization(uint32_t value) {
  guard ^= value & 0xff;
  guard ^= (value >> 8) & 0xff;
  guard ^= (value >> 16) & 0xff;
  guard ^= (value >> 24) & 0xff;
}

/**
 * A small helper that runs the given lamba expression in a loop
 * and returns how long it took.
 */
template <typename F>
unsigned long runLambda(uint32_t count, F&& lambda) {
  yield();
  unsigned long startMillis = millis();
  while (count--) {
    lambda();
  }
  unsigned long elapsedMillis = millis() - startMillis;
  yield();
  digitalWrite(LED_BENCHMARK, (guard & 0x55) ? 1 : 0);
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

static unsigned long runEmptyLoop() {
  unsigned long emptyLoopMillis = runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    disableOptimization(tickMillis);
  });
  Serial.print(EMPTY_LOOP_LABEL);
  printMicrosPerIteration(emptyLoopMillis);
  Serial.println(ENDING);
  Serial.println(DIVIDER);
  return emptyLoopMillis;
}

// LocalDate::forEpochDays()
static unsigned long runLocalDateForEpochDays(unsigned long emptyLoopMillis) {
  unsigned long localDateForDaysMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // LocalDate::forEpochDays() takes days, but use millis for testing
    // purposes.
    LocalDate localDate = LocalDate::forEpochDays(tickMillis);
    disableOptimization(localDate);
    disableOptimization(tickMillis);
  });
  Serial.print(LOCAL_DATE_FOR_EPOCH_DAYS_LABEL);
  unsigned long elapsedMillis = localDateForDaysMillis - emptyLoopMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// LocalDate::toEpochDays()
static unsigned long runLocalDateToEpochDaysMillis(
    unsigned long forEpochDaysMillis) {
  unsigned long localDateToEpochDaysMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // LocalDate::forEpochDays(seconds) takes seconds, but use millis for
    // testing purposes.
    LocalDate localDate = LocalDate::forEpochDays(tickMillis);
    acetime_t epochDays = localDate.toEpochDays();
    disableOptimization(localDate);
    disableOptimization(epochDays);
  });
  Serial.print(LOCAL_DATE_TO_EPOCH_DAYS_LABEL);
  unsigned long elapsedMillis = localDateToEpochDaysMillis - forEpochDaysMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// LocalDate::dayOfWeek()
static unsigned long runLocalDateDaysOfWeekMillis(
    unsigned long forEpochDaysMillis) {
  unsigned long localDateDayOfWeekMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // LocalDate::forEpochDays(seconds) takes seconds, but use millis for
    // testing purposes.
    LocalDate localDate = LocalDate::forEpochDays(tickMillis);
    uint8_t dayOfWeek = localDate.dayOfWeek();
    disableOptimization(localDate);
    disableOptimization(dayOfWeek);
  });
  Serial.print(LOCAL_DATE_DAY_OF_WEEK_LABEL);
  unsigned long elapsedMillis = localDateDayOfWeekMillis - forEpochDaysMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// ZonedDateTime::forEpochSeconds(seconds)
static unsigned long runDateTimeForEpochSeconds(unsigned long emptyLoopMillis) {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // ZonedDateTime::forEpochSeconds(seconds) takes seconds, but use millis for
    // testing purposes.
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis);
    disableOptimization(dateTime);
    disableOptimization(tickMillis);
  });
  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_LABEL);
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// ZonedDateTime::toEpochDays()
static unsigned long runDateTimeToEpochDays(
    unsigned long forEpochSecondsMillis) {
  unsigned long toEpochDaysMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // ZonedDateTime::forEpochSeconds(seconds) takes seconds, but use millis for
    // testing purposes.
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis);
    acetime_t epochDays = dateTime.toEpochDays();
    disableOptimization(dateTime);
    disableOptimization(epochDays);
  });
  Serial.print(DATE_TIME_TO_EPOCH_DAYS_LABEL);
  unsigned long elapsedMillis = toEpochDaysMillis - forEpochSecondsMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// ZonedDateTime::toEpochSeconds()
static unsigned long runDateTimeToEpochSeconds(
    unsigned long forEpochSecondsMillis) {
  unsigned long toEpochSecondsMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // ZonedDateTime::forEpochSeconds(seconds) takes seconds, but use millis for
    // testing purposes.
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis);
    acetime_t epochSeconds = dateTime.toEpochSeconds();
    disableOptimization(dateTime);
    disableOptimization(epochSeconds);
  });
  Serial.print(DATE_TIME_TO_EPOCH_SECONDS_LABEL);
  unsigned long elapsedMillis = toEpochSecondsMillis - forEpochSecondsMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

// ZonedDateTime::forEpochSeconds(seconds, kZoneLos_Angeles)
static unsigned long runDateTimeForEpochSecondsLosAngeles(
    unsigned long emptyLoopMillis) {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // ZonedDateTime::forEpochSeconds(seconds) takes seconds, but use millis for
    // testing purposes.
    AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);
    TimeZone tz(&zoneSpec);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis, tz);
    disableOptimization(dateTime);
    disableOptimization(tickMillis);
  });
  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_LOS_ANGELES_LABEL);
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

static AutoZoneSpec spec(&zonedb::kZoneLos_Angeles);

// ZonedDateTime::forEpochSeconds(seconds, kZoneLos_Angeles) w/ cached TimeZone
static unsigned long runDateTimeForEpochSecondsLosAngelesCached(
    unsigned long emptyLoopMillis) {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() mutable {
    unsigned long tickMillis = millis();
    // ZonedDateTime::forEpochSeconds(seconds) takes seconds, but use millis for
    // testing purposes.
    TimeZone tzLosAngeles(&spec);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        tickMillis, tzLosAngeles);
    disableOptimization(dateTime);
    disableOptimization(tickMillis);
  });
  Serial.print(DATE_TIME_FOR_EPOCH_SECONDS_CACHED_LABEL);
  unsigned long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;
  printMicrosPerIteration(elapsedMillis);
  Serial.println(ENDING);
  return elapsedMillis;
}

void runBenchmarks() {
  Serial.println(TOP);
  Serial.println(HEADER);
  Serial.println(DIVIDER);

  unsigned long emptyLoopMillis = runEmptyLoop();

  unsigned long localDateForEpochDaysMillis =
      runLocalDateForEpochDays(emptyLoopMillis);
  runLocalDateToEpochDaysMillis(localDateForEpochDaysMillis);
  runLocalDateDaysOfWeekMillis(localDateForEpochDaysMillis);

  unsigned long dateTimeForEpochSecondsMillis =
      runDateTimeForEpochSeconds(emptyLoopMillis);
  runDateTimeForEpochSecondsLosAngeles(emptyLoopMillis);
  runDateTimeForEpochSecondsLosAngelesCached(emptyLoopMillis);
  runDateTimeToEpochDays(dateTimeForEpochSecondsMillis);
  runDateTimeToEpochSeconds(dateTimeForEpochSecondsMillis);

  // End footer
  Serial.println(BOTTOM);

  // Print some stats
  Serial.print("Number of iterations per run: ");
  Serial.println(COUNT);
}
