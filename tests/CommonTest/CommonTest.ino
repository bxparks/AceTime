#line 2 "CommonTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// DateStrings
// --------------------------------------------------------------------------

test(DateStringsTest, monthString) {
  common::DateStrings ds;

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
  common::DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, common::DateStrings::kBufferSize - 1);
}

test(DateStringsTest, dayOfWeekStrings) {
  common::DateStrings ds;

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
  common::DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t dayOfWeek = 0; dayOfWeek <= 7; dayOfWeek++) {
    const char* dayOfWeekString = ds.dayOfWeekLongString(dayOfWeek);
    uint8_t length = strlen(dayOfWeekString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, common::DateStrings::kBufferSize - 1);
}

// --------------------------------------------------------------------------
// common::decToBcd(), common::bcdToDec()
// --------------------------------------------------------------------------

test(decToBcd) {
  assertEqual(0x00, decToBcd(0));
  assertEqual(0x10, decToBcd(10));
  assertEqual(0x23, decToBcd(23));
  assertEqual(0x99, decToBcd(99));
}

test(bcdToDec) {
  assertEqual(0, bcdToDec(0x00));
  assertEqual(10, bcdToDec(0x10));
  assertEqual(23, bcdToDec(0x23));
  assertEqual(99, bcdToDec(0x99));
}

test(incrementMod) {
  int counter = 0;
  incrementMod(counter, 3);
  assertEqual(1, counter);

  incrementMod(counter, 3);
  assertEqual(2, counter);

  incrementMod(counter, 3);
  assertEqual(0, counter);
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
