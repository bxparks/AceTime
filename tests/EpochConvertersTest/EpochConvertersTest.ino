#line 2 "EpochConvertersTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>

using aunit::TestOnce;
using aunit::TestRunner;
using ace_time::LocalDate;
using ace_time::internal::EpochConverterJulian;
using ace_time::internal::EpochConverterHinnant;

//---------------------------------------------------------------------------
// Test the EpochConverters.
//
// Do a round-trip toEpochDays()/fromEpochDays() conversion for every day over
// the year interval [0001,10000) on Linux machines, and [1900,2100) on
// microcontrollers.
//
// * 2000-01-01: epoch = 0 (2000 is the internal epoch year)
// * 2100-01-01: epoch = 25*(366+365+365+365) = 36525
// * 1900-01-01: epoch = -(25*(365+365+365+366)-1) = -36524
// * 0000-01-01: epoch = -5*146097 = -730119
// * 0001-01-01: epoch = -730485 + 366 (year 0 is leap) = -730119
//---------------------------------------------------------------------------

#if defined(EPOXY_DUINO)
  const int16_t startYear = 1;
  const int16_t untilYear = 10000;
  const int32_t startEpochDays = -730119; // 0001-01-01
#else
  const int16_t startYear = 1900;
  const int16_t untilYear = 2100;
  const int32_t startEpochDays = -36524; // 1900-01-01
#endif

test(EpochConverterJulianTest, epoch2000) {
  int32_t days = EpochConverterJulian::toEpochDays(2000, 1, 1);
  assertEqual((int32_t) 0, days);
}

test(EpochConverterJulianTest, allDays) {
#if ! defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(
      F("EpochConverterJulianTest (one dot per year): "));
#endif

  int32_t epochDays = startEpochDays;
  for (int16_t year = startYear; year < untilYear; year++) {
  #if ! defined(EPOXY_DUINO)
    SERIAL_PORT_MONITOR.print('.'); // Prevent serial port timeout
    yield(); // Prevent watch dog timer on ESP8266.
  #endif
    for (uint8_t month = 1; month <= 12; month++) {
      uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
      for (uint8_t day = 1; day <= daysInMonth; day++) {
        // Test toEpochDays()
        int32_t obsEpochDays = EpochConverterJulian::toEpochDays(
            year, month, day);
        assertEqual(epochDays, obsEpochDays);

        // Test fromEopchDays()
        int16_t obsYear;
        uint8_t obsMonth;
        uint8_t obsDay;
        EpochConverterJulian::fromEpochDays(epochDays,
            obsYear, obsMonth, obsDay);
        assertEqual(year, obsYear);
        assertEqual(month, obsMonth);
        assertEqual(day, obsDay);

        // next epoch day
        epochDays++;
      }
    }
  }
#if ! defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.println();
#endif
}

test(EpochConverterHinnantTest, epoch2000) {
  int32_t days = EpochConverterHinnant::toEpochDays(2000, 1, 1);
  assertEqual((int32_t) 0, days);
}

test(EpochConverterHinnantTest, allDays) {
#if ! defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(
      F("EpochConverterHinnantTest (one dot per year): "));
#endif

  int32_t epochDays = startEpochDays;
  for (int16_t year = startYear; year < untilYear; year++) {
#if ! defined(EPOXY_DUINO)
    SERIAL_PORT_MONITOR.print('.'); // Prevent serial port timeout
    yield(); // Prevent watch dog timer on ESP8266.
#endif
    for (uint8_t month = 1; month <= 12; month++) {
      uint8_t daysInMonth = LocalDate::daysInMonth(year, month);
      for (uint8_t day = 1; day <= daysInMonth; day++) {
        // Test toEpochDays()
        int32_t obsEpochDays = EpochConverterHinnant::toEpochDays(
            year, month, day);
        assertEqual(epochDays, obsEpochDays);

        // Test fromEpochDays()
        int16_t obsYear;
        uint8_t obsMonth;
        uint8_t obsDay;
        EpochConverterHinnant::fromEpochDays(epochDays,
            obsYear, obsMonth, obsDay);
        assertEqual(year, obsYear);
        assertEqual(month, obsMonth);
        assertEqual(day, obsDay);

        // next epoch day
        epochDays++;
      }
    }
  }
#if ! defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.println();
#endif
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(2000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  // On slow 8-bit processors, the test suite can take over 50 seconds.
  TestRunner::setTimeout(100);
}

void loop() {
  TestRunner::run();
}
