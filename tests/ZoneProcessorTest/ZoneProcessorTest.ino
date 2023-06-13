#line 2 "ZoneProcessorTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using ace_time::LocalDate;
using ace_time::internal::calcStartDayOfMonth;
using ace_time::internal::MonthDay;
using ace_time::internal::createAbbreviation;

test(ZoneProcessorTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  MonthDay monthDay = calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1);
  assertEqual(11, monthDay.month);
  assertEqual(4, monthDay.day);

  // 2018-11, lastSun
  monthDay = calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0);
  assertEqual(11, monthDay.month);
  assertEqual(25, monthDay.day);

  // 2018-11, Sun>=30, should shift to 2018-12-2
  monthDay = calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 30);
  assertEqual(12, monthDay.month);
  assertEqual(2, monthDay.day);

  // 2018-11, Mon<=7
  monthDay = calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -7);
  assertEqual(11, monthDay.month);
  assertEqual(5, monthDay.day);

  // 2018-11, Mon<=1, shifts back into October
  monthDay = calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -1);
  assertEqual(10, monthDay.month);
  assertEqual(29, monthDay.day);

  // 2018-03, Thu>=9
  monthDay = calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9);
  assertEqual(3, monthDay.month);
  assertEqual(15, monthDay.day);

  // 2018-03-30 exactly
  monthDay = calcStartDayOfMonth(2018, 3, 0, 30);
  assertEqual(3, monthDay.month);
  assertEqual(30, monthDay.day);
}

test(ZoneProcessorTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  // If no '%', deltaSeconds and letter should not matter
  createAbbreviation(dst, kDstSize, "SAST", 0, nullptr);
  assertEqual("SAST", dst);

  createAbbreviation(dst, kDstSize, "SAST", 60, "A");
  assertEqual("SAST", dst);

  // If '%', and letter is (incorrectly) set to '\0', just copy the thing
  createAbbreviation(dst, kDstSize, "SA%ST", 0, nullptr);
  assertEqual("SA%ST", dst);

  // If '%', then replaced with (non-null) letterString.
  createAbbreviation(dst, kDstSize, "P%T", 60, "D");
  assertEqual("PDT", dst);

  createAbbreviation(dst, kDstSize, "P%T", 0, "S");
  assertEqual("PST", dst);

  createAbbreviation(dst, kDstSize, "P%T", 0, "");
  assertEqual("PT", dst);

  createAbbreviation(dst, kDstSize, "%", 60, "CAT");
  assertEqual("CAT", dst);

  createAbbreviation(dst, kDstSize, "%", 0, "WAT");
  assertEqual("WAT", dst);

  // If '/', then deltaSeconds selects the first or second component.
  createAbbreviation(dst, kDstSize, "GMT/BST", 0, "");
  assertEqual("GMT", dst);

  createAbbreviation(dst, kDstSize, "GMT/BST", 60, "");
  assertEqual("BST", dst);

  // test truncation to kDstSize
  createAbbreviation(dst, kDstSize, "P%T3456", 60, "DD");
  assertEqual("PDDT3", dst);
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
