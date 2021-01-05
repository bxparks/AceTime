#include <Arduino.h>
#include <stdint.h>
#include <Arduino.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;
using ace_common::PrintStr;

#if defined(ARDUINO_ARCH_AVR)
const uint32_t COUNT = 2500;
#elif defined(ARDUINO_ARCH_SAMD)
const uint32_t COUNT = 10000;
#elif defined(ESP8266)
const uint32_t COUNT = 10000;
#elif defined(ESP32)
const uint32_t COUNT = 100000;
#elif defined(TEENSYDUINO)
const uint32_t COUNT = 100000;
#elif defined(UNIX_HOST_DUINO)
// Linux or MacOS
const uint32_t COUNT = 200000;
#else
// A generic Arduino board that we have not looked at.
const uint32_t COUNT = 10000;
#endif

const uint32_t MILLIS_TO_NANO_PER_ITERATION = ((uint32_t) 1000000 / COUNT);

// The FPSTR() macro converts these (const char*) into (const
// __FlashHelperString*) so that the correct version of println() or print() is
// called. ESP8266 and ESP32 already define this. AVR and Teensy do not.
#ifndef FPSTR
#define FPSTR(pstr_pointer) \
      (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif

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

// Given total elapsed time in millis, print micros per iteration as a floating
// point number (without using floating point operations).
//
// Sometimes, the elapsedMillis is negative. This happens on some benchmarks on
// higher powered CPUs where the thing being measured is so quickly executed
// that the empty loop overhead can take a longer. Print "-0.000" if that
// occurs.
static void printMicrosPerIteration(long elapsedMillis) {
  if (elapsedMillis < 0) {
    SERIAL_PORT_MONITOR.print(F("  -0.000"));
    return;
  }
  unsigned long nanos = elapsedMillis * MILLIS_TO_NANO_PER_ITERATION;
  uint16_t whole = nanos / 1000;
  uint16_t frac = nanos % 1000;
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(whole);
  SERIAL_PORT_MONITOR.print('.');
  ace_common::printPad3To(SERIAL_PORT_MONITOR, frac, '0');
}

// Return how long the empty lookup takes.
unsigned long runEmptyLoopMillis() {
  return runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    disableOptimization(tickMillis);
  });
}

static void runEmptyLoop() {
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  SERIAL_PORT_MONITOR.print(F("EmptyLoop"));
  printMicrosPerIteration(emptyLoopMillis);
  SERIAL_PORT_MONITOR.println();
}

// LocalDate::forEpochDays()
static void runLocalDateForEpochDays() {
  unsigned long localDateForDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = localDateForDaysMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("LocalDate::forEpochDays()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
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
  long elapsedMillis = localDateToEpochDaysMillis - forEpochDaysMillis;

  SERIAL_PORT_MONITOR.print(F("LocalDate::toEpochDays()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
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
  long elapsedMillis = localDateDayOfWeekMillis - forEpochDaysMillis;

  SERIAL_PORT_MONITOR.print(F("LocalDate::dayOfWeek()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// OffsetDateTime::forEpochSeconds()
static void runOffsetDateTimeForEpochSeconds() {
  unsigned long localDateForDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    disableOptimization(odt);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = localDateForDaysMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("OffsetDateTime::forEpochSeconds()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// OffsetDateTime::toEpochSeconds()
static void runOffsetDateTimeToEpochSeconds() {
  unsigned long localDateToEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    acetime_t epochDays = odt.toEpochSeconds();
    disableOptimization(epochDays);
  });
  unsigned long forEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    disableOptimization(odt);
  });
  long elapsedMillis = localDateToEpochDaysMillis - forEpochDaysMillis;

  SERIAL_PORT_MONITOR.print(F("OffsetDateTime::toEpochSeconds()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// ZonedDateTime::forEpochSeconds(seconds)
static void runZonedDateTimeForEpochSeconds() {
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("ZonedDateTime::forEpochSeconds(UTC)"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// ZonedDateTime::toEpochDays()
static void runZonedDateTimeToEpochDays() {
  unsigned long toEpochDaysMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    acetime_t epochDays = dateTime.toEpochDays();
    disableOptimization(epochDays);
  });
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });
  long elapsedMillis = toEpochDaysMillis - forEpochSecondsMillis;

  SERIAL_PORT_MONITOR.print(F("ZonedDateTime::toEpochDays()"));
  printMicrosPerIteration(elapsedMillis < 0 ? 0 : elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// ZonedDateTime::toEpochSeconds()
static void runZonedDateTimeToEpochSeconds() {
  unsigned long toEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long tickMillis = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis,
        TimeZone());
    acetime_t epochSeconds = dateTime.toEpochSeconds();
    disableOptimization(epochSeconds);
  });
  unsigned long forEpochSecondsMillis = runLambda(COUNT, []() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });
  long elapsedMillis = toEpochSecondsMillis - forEpochSecondsMillis;

  SERIAL_PORT_MONITOR.print(F("ZonedDateTime::toEpochSeconds()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

static const acetime_t kTwoYears = 2 * 365 * 24 * 3600L;

static const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

static const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

// ZonedDateTime::forEpochSeconds(seconds, tz), uncached
static void runZonedDateTimeForEpochSecondsBasicZoneManager() {
	BasicZoneManager<2> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);
  acetime_t offset = 0;

  unsigned long forEpochSecondsMillis = runLambda(COUNT, [&offset, &manager]() {
    offset = (offset) ? 0 : kTwoYears;
    unsigned long fakeEpochSeconds = millis() + offset;
    TimeZone tzLosAngeles = manager.createForZoneInfo(
        &zonedb::kZoneAmerica_Los_Angeles);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("ZonedDateTime::forEpochSeconds(Basic_nocache)"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// ZonedDateTime::forEpochSeconds(seconds, tz) cached
static void runZonedDateTimeForEpochSecondsBasicZoneManagerCached() {
	BasicZoneManager<2> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);

  unsigned long forEpochSecondsMillis = runLambda(COUNT, [&manager]() {
    unsigned long fakeEpochSeconds = millis();
    TimeZone tzLosAngeles = manager.createForZoneInfo(
        &zonedb::kZoneAmerica_Los_Angeles);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("ZonedDateTime::forEpochSeconds(Basic_cached)"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

static const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

static const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

// ZonedDateTime::forEpochSeconds(seconds, tz), uncached
static void runZonedDateTimeForEpochSecondsExtendedZoneManager() {
	ExtendedZoneManager<2> manager(
      kExtendedZoneRegistrySize, kExtendedZoneRegistry);
  acetime_t offset = 0;

  unsigned long forEpochSecondsMillis = runLambda(COUNT, [&offset, &manager]() {
    offset = (offset) ? 0 : kTwoYears;
    unsigned long fakeEpochSeconds = millis() + offset;
    TimeZone tzLosAngeles = manager.createForZoneInfo(
        &zonedbx::kZoneAmerica_Los_Angeles);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(
    F("ZonedDateTime::forEpochSeconds(Extended_nocache)"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// ZonedDateTime::forEpochSeconds(seconds, tz) cached ExtendedZoneManager
static void runZonedDateTimeForEpochSecondsExtendedZoneManagerCached() {
	ExtendedZoneManager<2> manager(
      kExtendedZoneRegistrySize, kExtendedZoneRegistry);

  unsigned long forEpochSecondsMillis = runLambda(COUNT, [&manager]() {
    unsigned long fakeEpochSeconds = millis();
    TimeZone tzLosAngeles = manager.createForZoneInfo(
        &zonedbx::kZoneAmerica_Los_Angeles);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  long elapsedMillis = forEpochSecondsMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(
    F("ZonedDateTime::forEpochSeconds(Extended_cached)"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// These are too big for small AVR chips
#if ! defined(ARDUINO_ARCH_AVR)

static void runIndexForZoneName() {
	ExtendedZoneManager<2> manager(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);

  unsigned long runMillis = runLambda(COUNT, [&manager]() {
    PrintStr<20> printStr; // deliberately short to truncate some zones
    uint16_t randomIndex = random(zonedbx::kZoneRegistrySize);
    const extended::ZoneInfo* info = zonedbx::kZoneRegistry[randomIndex];
    const __FlashStringHelper* name = ExtendedZone(info).name();
    printStr.print(name);

    uint16_t index = manager.indexForZoneName(printStr.getCstr());
    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda(COUNT, [&manager]() {
    PrintStr<20> printStr; // deliberately short to truncate some zones
    uint16_t randomIndex = random(zonedbx::kZoneRegistrySize);
    const extended::ZoneInfo* info = zonedbx::kZoneRegistry[randomIndex];
    const __FlashStringHelper* name = ExtendedZone(info).name();
    printStr.print(name);

    uint16_t len = printStr.length();
    const char* s = printStr.getCstr();
    uint32_t tmp = s[0]
      + ((len > 1) ? ((uint32_t) s[1] << 8) : 0)
      + ((len > 2) ? ((uint32_t) s[2] << 16) : 0)
      + ((len > 3) ? ((uint32_t) s[3] << 24) : 0);
    disableOptimization((uint32_t) tmp);
  });

  long elapsedMillis = runMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("ExtendedZoneManager::indexForZoneName()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

static void runIndexForZoneId() {
	ExtendedZoneManager<2> manager(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  unsigned long runMillis = runLambda(COUNT, [&manager]() {
    uint16_t randomIndex = random(zonedbx::kZoneRegistrySize);
    const extended::ZoneInfo* info = zonedbx::kZoneRegistry[randomIndex];
    uint32_t zoneId = ExtendedZone(info).zoneId();

    uint16_t index = manager.indexForZoneId(zoneId);
    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda(COUNT, [&manager]() {
    uint16_t randomIndex = random(zonedbx::kZoneRegistrySize);
    const extended::ZoneInfo* info = zonedbx::kZoneRegistry[randomIndex];
    uint32_t zoneId = ExtendedZone(info).zoneId();

    disableOptimization(zoneId);
  });

  long elapsedMillis = runMillis - emptyLoopMillis;

  SERIAL_PORT_MONITOR.print(F("ExtendedZoneManager::indexForZoneId()"));
  printMicrosPerIteration(elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

#endif // defined(ARDUINO_ARCH_AVR)

void runBenchmarks() {
  runEmptyLoop();

  runLocalDateForEpochDays();
  runLocalDateToEpochDays();
  runLocalDateDaysOfWeek();

  runOffsetDateTimeForEpochSeconds();
  runOffsetDateTimeToEpochSeconds();

  runZonedDateTimeToEpochSeconds();
  runZonedDateTimeToEpochDays();

  runZonedDateTimeForEpochSeconds();
  runZonedDateTimeForEpochSecondsBasicZoneManager();
  runZonedDateTimeForEpochSecondsBasicZoneManagerCached();
  runZonedDateTimeForEpochSecondsExtendedZoneManager();
  runZonedDateTimeForEpochSecondsExtendedZoneManagerCached();

#if ! defined(ARDUINO_ARCH_AVR)
  runIndexForZoneName();
  runIndexForZoneId();
#endif

  SERIAL_PORT_MONITOR.print(F("Iterations_per_run "));
  SERIAL_PORT_MONITOR.println(COUNT);
}
