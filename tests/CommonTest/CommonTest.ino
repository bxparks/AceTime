#line 2 "CommonTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// DateStrings
//---------------------------------------------------------------------------

test(DateStringsTest, monthString) {
  DateStrings ds;

  assertEqual(F("Error"), ds.monthLongString(0));
  assertEqual(F("January"), ds.monthLongString(1));
  assertEqual(F("February"), ds.monthLongString(2));
  assertEqual(F("March"), ds.monthLongString(3));
  assertEqual(F("April"), ds.monthLongString(4));
  assertEqual(F("May"), ds.monthLongString(5));
  assertEqual(F("June"), ds.monthLongString(6));
  assertEqual(F("July"), ds.monthLongString(7));
  assertEqual(F("August"), ds.monthLongString(8));
  assertEqual(F("September"), ds.monthLongString(9));
  assertEqual(F("October"), ds.monthLongString(10));
  assertEqual(F("November"), ds.monthLongString(11));
  assertEqual(F("December"), ds.monthLongString(12));
  assertEqual(F("Error"), ds.monthLongString(13));

  assertEqual(F("Err"), ds.monthShortString(0));
  assertEqual(F("Jan"), ds.monthShortString(1));
  assertEqual(F("Feb"), ds.monthShortString(2));
  assertEqual(F("Mar"), ds.monthShortString(3));
  assertEqual(F("Apr"), ds.monthShortString(4));
  assertEqual(F("May"), ds.monthShortString(5));
  assertEqual(F("Jun"), ds.monthShortString(6));
  assertEqual(F("Jul"), ds.monthShortString(7));
  assertEqual(F("Aug"), ds.monthShortString(8));
  assertEqual(F("Sep"), ds.monthShortString(9));
  assertEqual(F("Oct"), ds.monthShortString(10));
  assertEqual(F("Nov"), ds.monthShortString(11));
  assertEqual(F("Dec"), ds.monthShortString(12));
  assertEqual(F("Err"), ds.monthShortString(13));
}

test(DateStringsTest, monthStringsFitInBuffer) {
  DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

test(DateStringsTest, dayOfWeekStrings) {
  DateStrings ds;

  assertEqual(F("Error"), ds.dayOfWeekLongString(0));
  assertEqual(F("Monday"), ds.dayOfWeekLongString(1));
  assertEqual(F("Tuesday"), ds.dayOfWeekLongString(2));
  assertEqual(F("Wednesday"), ds.dayOfWeekLongString(3));
  assertEqual(F("Thursday"), ds.dayOfWeekLongString(4));
  assertEqual(F("Friday"), ds.dayOfWeekLongString(5));
  assertEqual(F("Saturday"), ds.dayOfWeekLongString(6));
  assertEqual(F("Sunday"), ds.dayOfWeekLongString(7));
  assertEqual(F("Error"), ds.dayOfWeekLongString(8));

  assertEqual(F("Err"), ds.dayOfWeekShortString(0));
  assertEqual(F("Mon"), ds.dayOfWeekShortString(1));
  assertEqual(F("Tue"), ds.dayOfWeekShortString(2));
  assertEqual(F("Wed"), ds.dayOfWeekShortString(3));
  assertEqual(F("Thu"), ds.dayOfWeekShortString(4));
  assertEqual(F("Fri"), ds.dayOfWeekShortString(5));
  assertEqual(F("Sat"), ds.dayOfWeekShortString(6));
  assertEqual(F("Sun"), ds.dayOfWeekShortString(7));
  assertEqual(F("Err"), ds.dayOfWeekShortString(8));
}

test(DateStringsTest, dayOfWeekStringsFitInBuffer) {
  DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t dayOfWeek = 0; dayOfWeek <= 7; dayOfWeek++) {
    const char* dayOfWeekString = ds.dayOfWeekLongString(dayOfWeek);
    uint8_t length = strlen(dayOfWeekString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
