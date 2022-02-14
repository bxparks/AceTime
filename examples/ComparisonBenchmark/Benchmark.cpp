/*
 * Compare the run time of LocalDateTime::toEpochSeconds() and
 * LocalDateTime::forEpochSeconds() with the equivalent makeTime() and
 * breakTime() functions of the Arduino Time Library
 * (https://github.com/PaulStoffregen/Time).
 *
 * Each iteration performs:
 *
 *    1) a conversion from seconds (from epoch) to the date/time components (y,
 *       m, d, h, m, s), then,
 *    2) a round trip conversion back to seconds (from epoch).
 */

#include <stdint.h>
#include <Arduino.h>
#include <AceTime.h>
#include <AceCommon.h> // printUint32AsFloat3To()

// TimeLib (https://github.com/PaulStoffregen/Time) does not support EpoxyDuino.
#if ! defined(EPOXY_DUINO)
  #include <TimeLib.h>
#endif

#include "Benchmark.h"

using namespace ace_time;
using ace_common::printUint32AsFloat3To;

// ESP32 does not define SERIAL_PORT_MONITOR
#if !defined(SERIAL_PORT_MONITOR)
  #define SERIAL_PORT_MONITOR Serial
#endif

#if defined(ARDUINO_ARCH_AVR)
const uint32_t COUNT = 2000;
#elif defined(ARDUINO_ARCH_SAMD)
const uint32_t COUNT = 10000;
#elif defined(ARDUINO_ARCH_STM32)
const uint32_t COUNT = 10000;
#elif defined(ESP8266)
const uint32_t COUNT = 10000;
#elif defined(ESP32)
const uint32_t COUNT = 100000;
#elif defined(TEENSYDUINO)
const uint32_t COUNT = 100000;
#elif defined(EPOXY_DUINO)
const uint32_t COUNT = 200000; // Linux or MacOS
#else
const uint32_t COUNT = 10000;
#endif

// Number of seconds to increment on each iteration, enough to scan for 15
// years, from 2018 to 2023.
uint32_t const DELTA_SECONDS = (uint32_t) 15 * 365.25 * 86400 / COUNT;

acetime_t const START_SECONDS = 568080000; // 2018-01-01
acetime_t const START_SECONDS_UNIX = 1514764800; // 2018-01-01

// The compiler is extremelly good about removing code that does nothing. This
// volatile variable is used to create side-effects that prevent the compiler
// from optimizing out the code that's being tested. Each disableOptimization()
// method should perform 6 XOR operations to cancel each other out when
// subtracted.
volatile uint8_t guard;

void disableOptimization(acetime_t seconds) {
  // Two temp variables allows 2 more XOR operations, for a total of 6.
  uint8_t tmp1, tmp2;

  guard ^= (seconds >> 24) & 0xff;
  guard ^= (seconds >> 16) & 0xff;
  guard ^= (tmp1 = (seconds >> 8) & 0xff);
  guard ^= (tmp2 = seconds & 0xff);
  guard ^= tmp1;
  guard ^= tmp2;
}

void disableOptimization(const LocalDateTime& dt) {
  guard ^= dt.year();
  guard ^= dt.month();
  guard ^= dt.day();
  guard ^= dt.hour();
  guard ^= dt.minute();
  guard ^= dt.second();
}

#if ! defined(EPOXY_DUINO)
void disableOptimization(const tmElements_t& tm) {
  guard ^= tm.Second;
  guard ^= tm.Minute;
  guard ^= tm.Hour;
  guard ^= tm.Day;
  guard ^= tm.Month;
  guard ^= tm.Year;
}
#endif

// A small helper that runs the given lamba expression in a loop
// and returns how long it took.
template <typename F>
unsigned long runLambda(acetime_t startSeconds, F&& lambda) {
  unsigned long startMillis = millis();
  uint32_t count = COUNT;
  yield();
  while (count-- > 0) {
    lambda(startSeconds);
    startSeconds += DELTA_SECONDS;
  }
  yield();
  return millis() - startMillis;
}

const uint32_t MILLIS_TO_NANO_PER_ITERATION = (1000000 / COUNT);

// Given total elapsed time in millis, print micros per iteration as
// a floating point number (without using floating point operations).
//
// Sometimes, the elapsedMillis is negative. This happens on some benchmarks on
// higher powered CPUs where the thing being measured is so quickly executed
// that the empty loop overhead can take a longer. Print "-0.000" if that
// occurs.
void printMicrosPerIteration(
    const __FlashStringHelper* label,
    long elapsedMillis
) {
  SERIAL_PORT_MONITOR.print(label);
  SERIAL_PORT_MONITOR.print(' ');
  if (elapsedMillis < 0) {
    SERIAL_PORT_MONITOR.print(F("-0.000"));
  } else {
    unsigned long nanos = elapsedMillis * MILLIS_TO_NANO_PER_ITERATION;
    printUint32AsFloat3To(SERIAL_PORT_MONITOR, nanos);
  }
  SERIAL_PORT_MONITOR.println();
}

// empty loop
void runEmptyLoop() {
  unsigned long baseMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    disableOptimization(seconds);
  });

  printMicrosPerIteration(F("EmptyLoop"), baseMillis);
}

// AceTime library: LocalDateTime::forEpochSeconds()
void runAceTimeForEpochSeconds() {
  unsigned long elapsedMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    LocalDateTime dt = LocalDateTime::forEpochSeconds(seconds);
    disableOptimization(dt);
  });
  unsigned long baseMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    disableOptimization(seconds);
  });

  printMicrosPerIteration(
      F("LocalDateTime::forEpochSeconds()"),
      elapsedMillis - baseMillis);
}

// AceTime library: LocalDateTime::toEpochSeconds()
void runAceTimeToEpochSeconds() {
  unsigned long elapsedMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    LocalDateTime dt = LocalDateTime::forEpochSeconds(seconds);
    acetime_t roundTripSeconds = dt.toEpochSeconds();
    disableOptimization(roundTripSeconds);
  });
  unsigned long baseMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    LocalDateTime dt = LocalDateTime::forEpochSeconds(seconds);
    disableOptimization(dt);
  });

  printMicrosPerIteration(
      F("LocalDateTime::toEpochSeconds()"),
      elapsedMillis - baseMillis);
}

// Time library: breakTime()
void runTimeLibBreakTime() {
#if ! defined(EPOXY_DUINO)
  unsigned long elapsedMillis = runLambda(
    START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      disableOptimization(tm);
    });
  unsigned long baseMillis = runLambda(
    START_SECONDS_UNIX,
    [](acetime_t seconds) {
      disableOptimization(seconds);
    });

  printMicrosPerIteration(F("breakTime()"), elapsedMillis - baseMillis);
#endif
}

// Time library: makeTime()
void runTimeLibMakeTime() {
#if ! defined(EPOXY_DUINO)
  unsigned long elapsedMillis = runLambda(
    START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      seconds = makeTime(tm);
      disableOptimization(seconds);
    });
  unsigned long baseMillis = runLambda(
    START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      disableOptimization(tm);
    });

  printMicrosPerIteration(F("makeTime()"), elapsedMillis - baseMillis);
#endif
}

void runBenchmarks() {
  runEmptyLoop();
  runAceTimeForEpochSeconds();
  runTimeLibBreakTime();
  runAceTimeToEpochSeconds();
  runTimeLibMakeTime();

  // Print some stats
  SERIAL_PORT_MONITOR.print("Iterations_per_run ");
  SERIAL_PORT_MONITOR.println(COUNT);
  SERIAL_PORT_MONITOR.print("Delta_seconds ");
  SERIAL_PORT_MONITOR.println(DELTA_SECONDS);
}
