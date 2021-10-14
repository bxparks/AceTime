/*
 * Here are various attempts to reduce program size to < 32kB so that this can
 * run on an Arduino Nano w/ only 32kB of flash.
 *
 * * Sketch uses 36830 bytes (119%) of program storage space. Maximum is 30720
 *   bytes. (original, with 266 Basic zones)
 *
 * * Sketch uses 36378 bytes (118%) of program storage space. Maximum is 30720
 *   bytes (after converting Lambda to function pointer).
 *
 * * Sketch uses 36074 bytes (117%) of program storage space. Maximum is 30720
 *   bytes. (after creating common printResult()).
 *
 * * Sketch uses 35898 bytes (116%) of program storage space. Maximum is 30720
 *   bytes. (after commenting out runLocalDateForEpochDays() and
 *   runLocalDateToEpochDays(), reverted).
 *
 * * Sketch uses 28136 bytes (91%) of program storage space. Maximum is 30720
 *   bytes. (After creating a custom kBenchmarkZoneRegistry with only 83 zones.)
 *
 * * Sketch uses 28036 bytes (91%) of program storage space. Maximum is 30720
 *   bytes. (After moving elapsedMillis calculation into printResult().
 *
 * * Sketch uses 27990 bytes (91%) of program storage space. Maximum is 30720
 *   bytes. (After replacing 2 zones with America/Denver and
 *   America/Los_Angeles, since they are already used in the kBasicZoneRegistry
 *   anyway.
 *
 * * Sketch uses 26298 bytes (85%) of program storage space. Maximum is 30720
 *   bytes. (After replace BasicZoneManager and ExtendedZoneManager with
 *   BasicZoneProcessor and ExtendedZoneProcessor, since the ZoneManagers were
 *   not needed for the benchmark.)
 *
 * * Sketch uses 26776 bytes (87%) of program storage space. Maximum is 30720
 *   bytes. (After adding support for LinkRegistry.)
 *
 * * (Pro Micro) Sketch uses 22780 bytes (79%) of program storage space.
 *   Maximum is 28672 bytes. On a ProMicro, disabling
 *   ZonedDateTime::forEpochSeconds(*) removes the dependency to
 *   ExtendedZoneProcessor, reducing flash consumption from 28872 bytes.
 *
 * * (Pro Micro) Sketch uses 24862 bytes (86%) of program storage space.
 *   After adding kLinkRegistry with 183 links to BasicZoneManager.
 *
 * * (Nano) Sketch uses 28856 bytes (93%) of program storage space.
 *   After adding kLinkRegistry with 183 links to BasicZoneManager.
 */

#include <Arduino.h>
#include <stdint.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include "Benchmark.h"
#include "zone_registry.h"

using namespace ace_time;
using ace_common::PrintStr;

#if defined(ARDUINO_ARCH_AVR)
const uint32_t COUNT = 1000;
#elif defined(ARDUINO_ARCH_SAMD)
const uint32_t COUNT = 5000;
#elif defined(ARDUINO_ARCH_STM32)
const uint32_t COUNT = 10000;
#elif defined(ESP8266)
const uint32_t COUNT = 10000;
#elif defined(ESP32)
const uint32_t COUNT = 20000;
#elif defined(TEENSYDUINO)
const uint32_t COUNT = 20000;
#elif defined(EPOXY_DUINO)
// Linux or MacOS
const uint32_t COUNT = 50000;
#else
// A generic Arduino board that we have not looked at.
const uint32_t COUNT = 10000;
#endif

const uint32_t MILLIS_TO_NANO_PER_ITERATION = ((uint32_t) 1000000 / COUNT);

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
  guard ^= value & 0xff;
  guard ^= (value >> 8) & 0xff;
  guard ^= (value >> 16) & 0xff;
  guard ^= (value >> 24) & 0xff;
}

// Type declaration of the Lambda.
typedef void (*Lambda)();

// A small helper that runs the given lamba expression in a loop
// and returns how long it took.
unsigned long runLambda(Lambda lambda) {
  yield();
  uint32_t count = COUNT;
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

#if defined(ARDUINO_AVR_PROMICRO)
// Print -1 to indicate that the benchmark was not executed, due to memory.
static void printNullResult(const __FlashStringHelper* label) {
  SERIAL_PORT_MONITOR.print(label);
  SERIAL_PORT_MONITOR.println(" -1");
}
#endif

static void printResult(
    const __FlashStringHelper* label,
    unsigned long benchmarkMillis,
    unsigned long baselineMillis
) {
  long elapsedMillis = benchmarkMillis - baselineMillis;
  SERIAL_PORT_MONITOR.print(label);
  printMicrosPerIteration(elapsedMillis < 0 ? 0 : elapsedMillis);
  SERIAL_PORT_MONITOR.println();
}

// Return how long the empty lookup takes.
unsigned long runEmptyLoopMillis() {
  return runLambda([]() {
    unsigned long tickMillis = millis();
    disableOptimization(tickMillis);
  });
}

static void runEmptyLoop() {
  unsigned long emptyLoopMillis = runEmptyLoopMillis();
  printResult(F("EmptyLoop"), emptyLoopMillis, 0);
}

// LocalDate::forEpochDays()
static void runLocalDateForEpochDays() {
  unsigned long localDateForDaysMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("LocalDate::forEpochDays()"), localDateForDaysMillis,
      emptyLoopMillis);
}

// LocalDate::toEpochDays()
static void runLocalDateToEpochDays() {
  unsigned long localDateToEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    acetime_t epochDays = localDate.toEpochDays();
    disableOptimization(epochDays);
  });
  unsigned long forEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });

  printResult(F("LocalDate::toEpochDays()"), localDateToEpochDaysMillis,
      forEpochDaysMillis);
}

// LocalDate::dayOfWeek()
static void runLocalDateDaysOfWeek() {
  unsigned long localDateDayOfWeekMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    uint8_t dayOfWeek = localDate.dayOfWeek();
    disableOptimization(localDate);
    disableOptimization(dayOfWeek);
  });
  unsigned long forEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    LocalDate localDate = LocalDate::forEpochDays(fakeEpochDays);
    disableOptimization(localDate);
  });

  printResult(F("LocalDate::dayOfWeek()"), localDateDayOfWeekMillis,
      forEpochDaysMillis);
}

// OffsetDateTime::forEpochSeconds()
static void runOffsetDateTimeForEpochSeconds() {
  unsigned long localDateForDaysMillis = runLambda([]() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    disableOptimization(odt);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("OffsetDateTime::forEpochSeconds()"), localDateForDaysMillis,
      emptyLoopMillis);
}

// OffsetDateTime::toEpochSeconds()
static void runOffsetDateTimeToEpochSeconds() {
  unsigned long localDateToEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    acetime_t epochDays = odt.toEpochSeconds();
    disableOptimization(epochDays);
  });
  unsigned long forEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochSeconds = millis();
    OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
        fakeEpochSeconds, TimeOffset());
    disableOptimization(odt);
  });

  printResult(F("OffsetDateTime::toEpochSeconds()"), localDateToEpochDaysMillis,
      forEpochDaysMillis);
}

// ZonedDateTime::forEpochSeconds(seconds)
static void runZonedDateTimeForEpochSecondsUTC() {
  unsigned long forEpochSecondsMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("ZonedDateTime::forEpochSeconds(UTC)"), forEpochSecondsMillis,
    emptyLoopMillis);
}

// ZonedDateTime::toEpochDays()
static void runZonedDateTimeToEpochDays() {
  unsigned long toEpochDaysMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    acetime_t epochDays = dateTime.toEpochDays();
    disableOptimization(epochDays);
  });
  unsigned long forEpochSecondsMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });

  printResult(F("ZonedDateTime::toEpochDays()"), toEpochDaysMillis,
      forEpochSecondsMillis);
}

// ZonedDateTime::toEpochSeconds()
static void runZonedDateTimeToEpochSeconds() {
  unsigned long toEpochSecondsMillis = runLambda([]() {
    unsigned long tickMillis = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(tickMillis,
        TimeZone());
    acetime_t epochSeconds = dateTime.toEpochSeconds();
    disableOptimization(epochSeconds);
  });
  unsigned long forEpochSecondsMillis = runLambda([]() {
    unsigned long fakeEpochDays = millis();
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(fakeEpochDays,
        TimeZone());
    disableOptimization(dateTime);
  });

  printResult(F("ZonedDateTime::toEpochSeconds()"), toEpochSecondsMillis,
      forEpochSecondsMillis);
}

// Epoch seconds offset alternating between 0 and kTwoYears on each iterations,
// used to prevent caching to obtain benchmarking number without caching.
static acetime_t offset = 0;
static const acetime_t kTwoYears = 2 * 365 * 24 * 3600L;

// Pointer to BasicZoneManager and ExtendedZoneManager, whose actual instances
// are created within the specific test on the stack. These global variables
// allows the lamba functions below to avoid captures, which allows passing the
// lambdas into runLambda() as a function pointer, which I hope lowers the
// flash memory consumption of this program enough to run on a 32kB Arduino
// Nano.
static BasicZoneProcessor* basicZoneProcessor;

#if ! defined(ARDUINO_AVR_PROMICRO)
  static ExtendedZoneProcessor* extendedZoneProcessor;
#endif

// ZonedDateTime::forEpochSeconds(seconds, tz), uncached
static void runZonedDateTimeForEpochSecondsBasicNoCache() {
	BasicZoneProcessor processor;
  basicZoneProcessor = &processor;
  offset = 0;

  unsigned long forEpochSecondsMillis = runLambda([]() {
    offset = (offset) ? 0 : kTwoYears;  // break caching
    unsigned long fakeEpochSeconds = millis() + offset;
    TimeZone tzLosAngeles = TimeZone::forZoneInfo(
        &zonedb::kZoneAmerica_Los_Angeles,
        basicZoneProcessor);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("ZonedDateTime::forEpochSeconds(Basic_nocache)"),
      forEpochSecondsMillis, emptyLoopMillis);
}

// ZonedDateTime::forEpochSeconds(seconds, tz) cached
static void runZonedDateTimeForEpochSecondsBasicCached() {
	BasicZoneProcessor processor;
  basicZoneProcessor = &processor;

  unsigned long forEpochSecondsMillis = runLambda([]() {
    unsigned long fakeEpochSeconds = millis();
    TimeZone tzLosAngeles = TimeZone::forZoneInfo(
        &zonedb::kZoneAmerica_Los_Angeles,
        basicZoneProcessor);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("ZonedDateTime::forEpochSeconds(Basic_cached)"),
      forEpochSecondsMillis, emptyLoopMillis);
}

// ZonedDateTime::forEpochSeconds(seconds, tz), uncached
static void runZonedDateTimeForEpochSecondsExtendedNoCache() {
#if defined(ARDUINO_AVR_PROMICRO)
  printNullResult(F("ZonedDateTime::forEpochSeconds(Extended_nocache)"));

#else
	ExtendedZoneProcessor processor;
  extendedZoneProcessor = &processor;
  offset = 0;

  unsigned long forEpochSecondsMillis = runLambda([]() {
    offset = (offset) ? 0 : kTwoYears;
    unsigned long fakeEpochSeconds = millis() + offset;
    TimeZone tzLosAngeles = TimeZone::forZoneInfo(
        &zonedbx::kZoneAmerica_Los_Angeles,
        extendedZoneProcessor);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("ZonedDateTime::forEpochSeconds(Extended_nocache)"),
      forEpochSecondsMillis, emptyLoopMillis);
#endif
}

// ZonedDateTime::forEpochSeconds(seconds, tz) cached
static void runZonedDateTimeForEpochSecondsExtendedCached() {
#if defined(ARDUINO_AVR_PROMICRO)
  printNullResult(F("ZonedDateTime::forEpochSeconds(Extended_cache)"));

#else
	ExtendedZoneProcessor processor;
  extendedZoneProcessor = &processor;

  unsigned long forEpochSecondsMillis = runLambda([]() {
    unsigned long fakeEpochSeconds = millis();
    TimeZone tzLosAngeles = TimeZone::forZoneInfo(
        &zonedbx::kZoneAmerica_Los_Angeles,
        extendedZoneProcessor);
    ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
        fakeEpochSeconds, tzLosAngeles);
    disableOptimization(dateTime);
  });
  unsigned long emptyLoopMillis = runEmptyLoopMillis();

  printResult(F("ZonedDateTime::forEpochSeconds(Extended_cached)"),
      forEpochSecondsMillis, emptyLoopMillis);
#endif

}

// This sketch is small enough to run on an Arduino Nano with about ~32kB. It
// currently squeezes just under the ~30kB limit for a SparkFun Pro Micro.
// "Sketch uses 28394 bytes (99%) of program storage space. Maximum is 28672
// bytes."

basic::ZoneRegistrar* basicZoneRegistrar;

static void runIndexForZoneName() {
  basic::ZoneRegistrar registrar(
      kBenchmarkZoneRegistrySize,
      kBenchmarkZoneRegistry);
  basicZoneRegistrar = &registrar;

  unsigned long runMillis = runLambda([]() {
    PrintStr<40> printStr;
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = basicZoneRegistrar->getZoneInfoForIndex(
        randomIndex);
    BasicZone(info).printNameTo(printStr);

    uint16_t index = basicZoneRegistrar->findIndexForName(printStr.cstr());
    if (index == basic::ZoneRegistrar::kInvalidIndex) {
      SERIAL_PORT_MONITOR.println(F("Not found"));
    }
    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda([]() {
    PrintStr<40> printStr;
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = basicZoneRegistrar->getZoneInfoForIndex(
        randomIndex);
    BasicZone(info).printNameTo(printStr);

    uint16_t len = printStr.length();
    const char* s = printStr.cstr();
    uint32_t tmp = s[0]
      + ((len > 1) ? ((uint32_t) s[1] << 8) : 0)
      + ((len > 2) ? ((uint32_t) s[2] << 16) : 0)
      + ((len > 3) ? ((uint32_t) s[3] << 24) : 0);
    disableOptimization((uint32_t) tmp);
  });

  printResult(F("BasicZoneManager::createForZoneName(binary)"), runMillis,
      emptyLoopMillis);
}

// non-static to allow friend access into basic::ZoneRegistrar
void runIndexForZoneIdBinary() {
	basic::ZoneRegistrar registrar(
      kBenchmarkZoneRegistrySize,
      kBenchmarkZoneRegistry);
  basicZoneRegistrar = &registrar;

  unsigned long runMillis = runLambda([]() {
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = basicZoneRegistrar->getZoneInfoForIndex(
        randomIndex);
    uint32_t zoneId = BasicZone(info).zoneId();

    uint16_t index = basicZoneRegistrar->findIndexForIdBinary(zoneId);
    if (index == basic::ZoneRegistrar::kInvalidIndex) {
      SERIAL_PORT_MONITOR.println(F("Not found"));
    }
    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda([]() {
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = basicZoneRegistrar->getZoneInfoForIndex(
        randomIndex);
    uint32_t zoneId = BasicZone(info).zoneId();

    disableOptimization(zoneId);
  });

  printResult(F("BasicZoneManager::createForZoneId(binary)"), runMillis,
      emptyLoopMillis);
}

// non-static to allow friend access into basic::ZoneRegistrar
void runIndexForZoneIdLinear() {
	basic::ZoneRegistrar registrar(
      kBenchmarkZoneRegistrySize,
      kBenchmarkZoneRegistry);
  basicZoneRegistrar = &registrar;

  unsigned long runMillis = runLambda([]() {
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = kBenchmarkZoneRegistry[randomIndex];
    uint32_t zoneId = BasicZone(info).zoneId();

    uint16_t index = basicZoneRegistrar->findIndexForIdLinear(zoneId);
    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda([]() {
    uint16_t randomIndex = random(kBenchmarkZoneRegistrySize);
    const basic::ZoneInfo* info = kBenchmarkZoneRegistry[randomIndex];
    uint32_t zoneId = BasicZone(info).zoneId();

    disableOptimization(zoneId);
  });

  printResult(F("BasicZoneManager::createForZoneId(linear)"), runMillis,
      emptyLoopMillis);
}

basic::LinkRegistrar* basicLinkRegistrar;

// non-static to allow friend access into basic::ZoneRegistrar
void runIndexForZoneIdWithThinLinks() {
	basic::ZoneRegistrar zoneRegistrar(
      kBenchmarkZoneRegistrySize,
      kBenchmarkZoneRegistry);
  basicZoneRegistrar = &zoneRegistrar;

  basic::LinkRegistrar linkRegistrar(
      zonedb::kLinkRegistrySize,
      zonedb::kLinkRegistry);
  basicLinkRegistrar = &linkRegistrar;

  unsigned long runMillis = runLambda([]() {
    uint16_t randomIndex = random(zonedb::kLinkRegistrySize);
    const basic::LinkEntry* entry = basicLinkRegistrar->getLinkEntryForIndex(
        randomIndex);
    uint32_t linkId = basic::LinkEntryBroker(entry).linkId();

    uint16_t index = basicZoneRegistrar->findIndexForId(linkId);
    if (index != basic::ZoneRegistrar::kInvalidIndex) {
      SERIAL_PORT_MONITOR.println(F("ERROR: Link found in Zone registry"));
    }

    index = basicLinkRegistrar->findIndexForId(linkId);
    if (index == basic::ZoneRegistrar::kInvalidIndex) {
      SERIAL_PORT_MONITOR.println(F("ERROR: Link missing from Link registry"));
    }

    disableOptimization(index);
  });

  unsigned long emptyLoopMillis = runLambda([]() {
    uint16_t randomIndex = random(zonedb::kLinkRegistrySize);
    const basic::LinkEntry* entry = basicLinkRegistrar->getLinkEntryForIndex(
        randomIndex);
    uint32_t linkId = basic::LinkEntryBroker(entry).linkId();

    disableOptimization(linkId);
  });

  printResult(F("BasicZoneManager::createForZoneId(link)"), runMillis,
      emptyLoopMillis);
}

void runBenchmarks() {
  runEmptyLoop();

  runLocalDateForEpochDays();
  runLocalDateToEpochDays();
  runLocalDateDaysOfWeek();

  runOffsetDateTimeForEpochSeconds();
  runOffsetDateTimeToEpochSeconds();

  runZonedDateTimeToEpochSeconds();
  runZonedDateTimeToEpochDays();

  runZonedDateTimeForEpochSecondsUTC();
  runZonedDateTimeForEpochSecondsBasicNoCache();
  runZonedDateTimeForEpochSecondsBasicCached();
  runZonedDateTimeForEpochSecondsExtendedNoCache();
  runZonedDateTimeForEpochSecondsExtendedCached();

  runIndexForZoneName();
  runIndexForZoneIdBinary();
  runIndexForZoneIdLinear();
  //runIndexForZoneIdWithThinLinks();

  SERIAL_PORT_MONITOR.print(F("Iterations_per_run "));
  SERIAL_PORT_MONITOR.println(COUNT);
}
