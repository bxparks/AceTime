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
// Do a round-trip toEpochDays()/fromEpochDays() conversion for every day from
// 0001-01-01 to 9999-12-31, inclusive.
// * 2000-01-01: epoch = 0
// * 0000-01-01: epoch = -5*146097 = -730119
// * 0001-01-01: epoch = -730485 + 366 (year=0 is leap) = -730119
//---------------------------------------------------------------------------

test(EpochConverterJulianTest, epoch2000) {
  int32_t days = EpochConverterJulian::toEpochDays(2000, 1, 1);
  assertEqual(0, days);
}

test(EpochConverterJulianTest, allDays) {
  int32_t epochDays = -730119; // 0001-01-01
  for (int16_t year = 1; year <= 9999; year++) {
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
}

test(EpochConverterHinnantTest, epoch2000) {
  int32_t days = EpochConverterHinnant::toEpochDays(2000, 1, 1);
  assertEqual(0, days);
}

test(EpochConverterHinnantTest, allDays) {
  int32_t epochDays = -730119; // 0001-01-01
  for (int16_t year = 1; year <= 9999; year++) {
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
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

  // On slow 8-bit processors, the test suite can take over 50 seconds.
  TestRunner::setTimeout(100);
}

void loop() {
  TestRunner::run();
}
