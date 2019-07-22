/*
 * Compare the run time of LocalDateTime::toEpochSeconds() and
 * forEpochSeconds() with the equivalent makeTime() and breakTime() functions
 * of the Arduino Time Library
 * (https://github.com/PaulStoffregen/Time). Each iteration performs:
 *    1) a conversion from seconds (from epoch) to the date/time components (y,
 *       m, d, h, m, s), then,
 *    2) a round trip conversion back to seconds (from epoch).
 */

#include <stdint.h>
#include <Arduino.h>
#include <AceTime.h>
#include <Time.h> // https://github.com/PaulStoffregen/Time
#include "Benchmark.h"

using namespace ace_time;

#if defined(ARDUINO_ARCH_AVR)
const uint32_t COUNT = 2000;
#elif defined(ARDUINO_ARCH_SAMD)
const uint32_t COUNT = 10000;
#elif defined(ESP8266)
const uint32_t COUNT = 10000;
#elif defined(ESP32) || defined(TEENSYDUINO)
const uint32_t COUNT = 100000;
#else
const uint32_t COUNT = 200000;
#endif

// Number of seconds to increment on each iteration, enough to scan for 15
// years, from 2018 to 2023.
uint32_t const DELTA_SECONDS = (uint32_t) 15 * 365.25 * 86400 / COUNT;

acetime_t const START_SECONDS = 568080000; // 2018-01-01
acetime_t const START_SECONDS_UNIX = 1514764800; // 2018-01-01

// The following strings are placed into PROGMEM flash memory to prevent them
// from consuming static RAM on the AVR platform. The FPSTR() macro converts
// these (const char*) into (const __FlashHelperString*) so that the correct
// version of println() or print() is called.
#ifndef FPSTR
#define FPSTR(pstr_pointer) \
      (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif

const char TOP[] PROGMEM =
  "+----------------------------------+---------+";
const char HEADER[] PROGMEM =
  "| Method                           |  micros |";
const char DIVIDER[] PROGMEM =
  "|----------------------------------|---------|";
const char* const BOTTOM = TOP;
const char EMPTY_LOOP_LABEL[] PROGMEM =
  "| Empty loop                       | ";
const char ACE_TIME_FOR_EPOCH_SECONDS[] PROGMEM =
  "| LocalDateTime::forEpochSeconds() | ";
const char ACE_TIME_TO_EPOCH_SECONDS[] PROGMEM =
  "| LocalDateTime::toEpochSeconds()  | ";
const char ARDUINO_TIME_BREAK_TIME[] PROGMEM =
  "| breakTime()                      | ";
const char ARDUINO_TIME_MAKE_TIME[] PROGMEM =
  "| makeTime()                       | ";
const char ENDING[] PROGMEM = " |";

// The compiler is extremelly good about removing code that does nothing. This
// volatile variable is used to carete side-effects that prevent the compiler
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

void disableOptimization(const tmElements_t& tm) {
  guard ^= tm.Second;
  guard ^= tm.Minute;
  guard ^= tm.Hour;
  guard ^= tm.Day;
  guard ^= tm.Month;
  guard ^= tm.Year;
}

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

void printPad3(uint16_t val, char padChar) {
  if (val < 100) SERIAL_PORT_MONITOR.print(padChar);
  if (val < 10) SERIAL_PORT_MONITOR.print(padChar);
  SERIAL_PORT_MONITOR.print(val);
}

const uint32_t MILLIS_TO_NANO_PER_ITERATION = ( 1000000 / COUNT);

// Given total elapsed time in millis, print micros per iteration as
// a floating point number (without using floating point operations).
//
// Sometimes, the elapsedMillis is negative. This happens on some benchmarks on
// higher powered CPUs where the thing being measured is so quickly executed
// that the empty loop overhead can take a longer. Print "-0.000" if that
// occurs.
void printMicrosPerIteration(long elapsedMillis) {
  if (elapsedMillis < 0) {
    SERIAL_PORT_MONITOR.print(F("  -0.000"));
    return;
  }
  unsigned long nanos = elapsedMillis * MILLIS_TO_NANO_PER_ITERATION;
  uint16_t whole = nanos / 1000;
  uint16_t frac = nanos % 1000;
  printPad3(whole, ' ');
  SERIAL_PORT_MONITOR.print('.');
  printPad3(frac, '0');
}

// empty loop
void runEmptyLoop() {
  unsigned long baseMillis = runLambda(START_SECONDS, [](acetime_t seconds) {
    disableOptimization(seconds);
  });

  SERIAL_PORT_MONITOR.print(FPSTR(EMPTY_LOOP_LABEL));
  printMicrosPerIteration(baseMillis);
  SERIAL_PORT_MONITOR.println(FPSTR(ENDING));
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

  SERIAL_PORT_MONITOR.print(FPSTR(ACE_TIME_FOR_EPOCH_SECONDS));
  printMicrosPerIteration(elapsedMillis - baseMillis);
  SERIAL_PORT_MONITOR.println(FPSTR(ENDING));
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

  SERIAL_PORT_MONITOR.print(FPSTR(ACE_TIME_TO_EPOCH_SECONDS));
  printMicrosPerIteration(elapsedMillis - baseMillis);
  SERIAL_PORT_MONITOR.println(FPSTR(ENDING));
}

// Time library: breakTime()
void runTimeLibBreakTime() {
  unsigned long elapsedMillis = runLambda(START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      disableOptimization(tm);
    });
  unsigned long baseMillis = runLambda(START_SECONDS_UNIX,
    [](acetime_t seconds) {
      disableOptimization(seconds);
    });

  SERIAL_PORT_MONITOR.print(FPSTR(ARDUINO_TIME_BREAK_TIME));
  printMicrosPerIteration(elapsedMillis - baseMillis);
  SERIAL_PORT_MONITOR.println(FPSTR(ENDING));
}

// Time library: makeTime()
void runTimeLibMakeTime() {
  unsigned long elapsedMillis = runLambda(START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      seconds = makeTime(tm);
      disableOptimization(seconds);
    });
  unsigned long baseMillis = runLambda(START_SECONDS_UNIX,
    [](acetime_t seconds) {
      tmElements_t tm;
      breakTime((time_t) seconds, tm);
      disableOptimization(tm);
    });

  SERIAL_PORT_MONITOR.print(FPSTR(ARDUINO_TIME_MAKE_TIME));
  printMicrosPerIteration(elapsedMillis - baseMillis);
  SERIAL_PORT_MONITOR.println(FPSTR(ENDING));
}

void runBenchmarks() {
  SERIAL_PORT_MONITOR.println(FPSTR(TOP));
  SERIAL_PORT_MONITOR.println(FPSTR(HEADER));
  SERIAL_PORT_MONITOR.println(FPSTR(DIVIDER));

  runEmptyLoop();
  SERIAL_PORT_MONITOR.println(FPSTR(DIVIDER));

  runAceTimeForEpochSeconds();
  runTimeLibBreakTime();
  SERIAL_PORT_MONITOR.println(FPSTR(DIVIDER));

  runAceTimeToEpochSeconds();
  runTimeLibMakeTime();
  SERIAL_PORT_MONITOR.println(FPSTR(BOTTOM));

  // Print some stats
  SERIAL_PORT_MONITOR.print("Number of iterations per run: ");
  SERIAL_PORT_MONITOR.println(COUNT);
  SERIAL_PORT_MONITOR.print("Delta seconds: ");
  SERIAL_PORT_MONITOR.println(DELTA_SECONDS);
}
