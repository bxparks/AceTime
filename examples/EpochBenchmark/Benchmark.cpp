/*
 * Compare the speed of various low-level epoch conversion routines in:
 *
 * * EpochConverterJulian
 * * EpochConverterHinnant
 */

#include <Arduino.h>
#include <stdint.h>
#include <AceCommon.h> // printUint32AsFloat3To()
#include <AceTime.h>
#include "Benchmark.h"

using ace_common::printUint32AsFloat3To;
using ace_time::LocalDate;
using ace_time::EpochConverterJulian;
using ace_time::EpochConverterHinnant;

// Sometimes, depending on the size of the AceTime library, the SparkFun
// ProMicro does not have enough flash, so this allows us to disable
// ExtendedZoneProcessor when needed.
#if defined(ARDUINO_AVR_PROMICRO)
  #define ENABLE_EXTENDED_ZONE_PROCESSOR 0
#else
  #define ENABLE_EXTENDED_ZONE_PROCESSOR 1
#endif

#if defined(ARDUINO_ARCH_AVR)
const uint32_t YEAR_STEP = 10;
#elif defined(ARDUINO_ARCH_SAMD)
const uint32_t YEAR_STEP = 5;
#elif defined(ARDUINO_ARCH_STM32)
const uint32_t YEAR_STEP = 1;
#elif defined(ESP8266)
const uint32_t YEAR_STEP = 1;
#elif defined(ESP32)
const uint32_t YEAR_STEP = 1;
#elif defined(TEENSYDUINO)
const uint32_t YEAR_STEP = 1;
#elif defined(EPOXY_DUINO)
// Linux or MacOS
const uint32_t YEAR_STEP = 1;
#else
// A generic Arduino board that we have not looked at.
const uint32_t YEAR_STEP = 1;
#endif

// The compiler is extremelly good about removing code that does nothing. This
// volatile variable is used to create side-effects that prevent the compiler
// from optimizing out the code that's being tested. Each disableOptimization()
// method should perform 6 XOR operations to cancel each other out when
// subtracted.
volatile uint32_t guard;

// Given total elapsed time in millis, print micros per iteration as a floating
// point number (without using floating point operations).
static void printMicrosPerIteration(
    const __FlashStringHelper* label, 
    uint32_t elapsedMillis,
    uint32_t iterations) {

  SERIAL_PORT_MONITOR.print(label);
  SERIAL_PORT_MONITOR.print(' ');

  uint32_t nanos = elapsedMillis * 1000 * 1000 / iterations;
  printUint32AsFloat3To(SERIAL_PORT_MONITOR, nanos);

  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(iterations);
}

//-----------------------------------------------------------------------------

// Return how long the empty lookup takes.
void runEmptyLoop(const __FlashStringHelper* label) {
  uint32_t iterations = 0;
  uint32_t startMillis = millis();
  for (int16_t year = 2000 - 127; year <= 2000 + 127; year += YEAR_STEP) {
    for (uint8_t month = 1; month <= 12; month++) {
      uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
      for (uint8_t day = 1; day <= daysInMonth; day++) {
        guard ^= year;
        guard ^= month;
        guard ^= day;

        iterations++;
      }
    }
  }
  uint32_t elapsedMillis = millis() - startMillis;
  printMicrosPerIteration(label, elapsedMillis, iterations);
}

// Benchmark the EpochConverterJulian
void runConverterJulian(const __FlashStringHelper* label) {
  uint32_t startMillis = millis();

  uint32_t iterations = 0;
  for (int16_t year = 2000 - 127; year <= 2000 + 127; year += YEAR_STEP) {
    for (uint8_t month = 1; month <= 12; month++) {
      uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
      for (uint8_t day = 1; day <= daysInMonth; day++) {
        // Test toEpochDays()
        int32_t epochDays = EpochConverterJulian::toEpochDays(
            year, month, day);

        // Test fromEopchDays()
        int16_t obsYear;
        uint8_t obsMonth;
        uint8_t obsDay;
        EpochConverterJulian::fromEpochDays(epochDays,
            obsYear, obsMonth, obsDay);

        guard ^= obsYear;
        guard ^= obsMonth;
        guard ^= obsDay;

        iterations++;
      }
    }
  }
  uint32_t elapsedMillis = millis() - startMillis;
  printMicrosPerIteration(label, elapsedMillis, iterations);
}

// Benchmark the EpochConverterHinnant
void runConverterHinnant(const __FlashStringHelper* label) {
  uint32_t startMillis = millis();

  uint32_t iterations = 0;
  for (int16_t year = 2000 - 127; year <= 2000 + 127; year += YEAR_STEP) {
    for (uint8_t month = 1; month <= 12; month++) {
      uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
      for (uint8_t day = 1; day <= daysInMonth; day++) {
        // Test toEpochDays()
        int32_t epochDays = EpochConverterHinnant::toEpochDays(
            year, month, day);

        // Test fromEopchDays()
        int16_t obsYear;
        uint8_t obsMonth;
        uint8_t obsDay;
        EpochConverterHinnant::fromEpochDays(epochDays,
            obsYear, obsMonth, obsDay);

        guard ^= obsYear;
        guard ^= obsMonth;
        guard ^= obsDay;

        iterations++;
      }
    }
  }
  uint32_t elapsedMillis = millis() - startMillis;
  printMicrosPerIteration(label, elapsedMillis, iterations);
}

//-----------------------------------------------------------------------------

void runBenchmarks() {
  runEmptyLoop(F("EmptyLoop"));
  runConverterJulian(F("EpochConverterJulian"));
  runConverterHinnant(F("EpochConverterHinnant"));
}
