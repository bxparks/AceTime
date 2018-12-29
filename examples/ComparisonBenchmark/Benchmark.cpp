/*
 * Compare the run time of AceTime to the Arduino Time Library
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

#if defined(AVR)
const uint32_t COUNT = 2000;
#elif defined(ESP8266)
const uint32_t COUNT = 10000;
#else
const uint32_t COUNT = 100000;
#endif

// Number of seconds to increment on each iteration, enough to scan for 15
// years, from 2018 to 2023.
uint32_t const DELTA_SECONDS = (uint32_t) 15 * 365.25 * 86400 / COUNT;

acetime_t const START_SECONDS = 568080000; // 2018-01-01
acetime_t const START_SECONDS_UNIX = 1514764800; // 2018-01-01
  
const char TOP[] = 
  "----------------------------+---------+";
const char HEADER[] = 
  "Method                      |  micros |";
const char DIVIDER[] = 
  "----------------------------|---------|";
const char BOTTOM[] = 
  "----------------------------+---------+";
const char EMPTY_LOOP_LABEL[] =
  "Empty loop                  | ";
const char ACE_TIME_LABEL[] =
  "AceTime library             | ";
const char ARDUINO_TIME_LABEL[] =
  "Arduino Time library        | ";
const char ENDING[] = " |";

// The compiler is extremelly good about removing code that does nothing. This
// variable is used to ensure user-visible side-effects, preventing the compiler
// optimization.
uint8_t guard;
void disableOptimization(acetime_t seconds) {
  guard ^= (seconds >> 24) & 0xff;
  guard ^= (seconds >> 16) & 0xff;
  guard ^= (seconds >> 8) & 0xff;
  guard ^= seconds & 0xff;
}

// A small helper that runs the given lamba expression in a loop
// and returns how long it took.
template <typename F>
unsigned long runBenchmark(acetime_t startSeconds, F&& lambda) {
  unsigned long startMillis = millis();
  uint32_t count = COUNT;
  yield();
  while (count-- > 0) {
    acetime_t unixSeconds = lambda(startSeconds);
    disableOptimization(unixSeconds);
    startSeconds += DELTA_SECONDS;
  }
  yield();
  digitalWrite(LED_BENCHMARK, (guard & 0x55) ? 1 : 0);
  return millis() - startMillis;
}

void printPad3(uint16_t val, char padChar) {
  if (val < 100) Serial.print(padChar);
  if (val < 10) Serial.print(padChar);
  Serial.print(val);
}

const uint32_t MILLIS_TO_NANO_PER_ITERATION = ( 1000000 / COUNT);

/**
 * Given total elapsed time in millis, print micros per iteration as
 * a floating point number (without using floating point operations).
 */
void printMicrosPerIteration(unsigned long elapsedMillis) {
  unsigned long nanos = elapsedMillis * MILLIS_TO_NANO_PER_ITERATION;
  uint16_t whole = nanos / 1000;
  uint16_t frac = nanos % 1000;
  printPad3(whole, ' ');
  Serial.print('.');
  printPad3(frac, '0');
}

void runBenchmarks() {
  Serial.println(TOP);
  Serial.println(HEADER);
  Serial.println(DIVIDER);

  unsigned long elapsedMillis;

  // empty loop
  unsigned long baseMillis = runBenchmark(START_SECONDS, [](acetime_t seconds) {
    return seconds;
  });
  Serial.print(EMPTY_LOOP_LABEL);
  printMicrosPerIteration(baseMillis);
  Serial.println(ENDING);

  // AceTime library
  elapsedMillis = runBenchmark(START_SECONDS, [](acetime_t seconds) {
    DateTime dt = DateTime::forEpochSeconds(seconds);
    acetime_t roundTripSeconds = dt.toEpochSeconds();
    return roundTripSeconds;
  });
  Serial.print(ACE_TIME_LABEL);
  printMicrosPerIteration(elapsedMillis - baseMillis);
  Serial.println(ENDING);

  // Time library
  elapsedMillis = runBenchmark(START_SECONDS_UNIX, [](acetime_t seconds) {
    tmElements_t tm;
    breakTime((time_t) seconds, tm);
    return makeTime(tm);
  });
  Serial.print(ARDUINO_TIME_LABEL);
  printMicrosPerIteration(elapsedMillis - baseMillis);
  Serial.println(ENDING);
  Serial.println(BOTTOM);

  // Print some stats
  Serial.print("Number of iterations per run: ");
  Serial.println(COUNT);
  Serial.print("Delta seconds: ");
  Serial.println(DELTA_SECONDS);
}
