#line 2 "ExtendedZoneSpecifierTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::extended;

// --------------------------------------------------------------------------
// ExtendedZoneSpecifier
// --------------------------------------------------------------------------

test(ExtendedZoneSpecifierTest, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {0, 1, 1, 0, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 0, 'w'}));

  dtp = {0, 1, 1, 95, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 95, 'w'}));

  dtp = {0, 1, 1, 96, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 0, 'w'}));

  dtp = {0, 1, 1, 97, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 1, 'w'}));

  dtp = {0, 1, 1, -96, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-01, 12, 31, 0, 'w'}));

  dtp = {0, 1, 1, -97, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-01, 12, 31, -1, 'w'}));
}

test(ExtendedZoneSpecifierTest, expandDateTuple) {
  DateTuple ttw;
  DateTuple tts;
  DateTuple ttu;
  DateTuple tt;
  int8_t offsetCode;
  int8_t deltaCode;

  tt = {0, 1, 30, 12, 'w'};
  offsetCode = 8;
  deltaCode = 4;
  ExtendedZoneSpecifier::expandDateTuple(&ttw, &tts, &ttu,
      tt, offsetCode, deltaCode);
  assertTrue((ttw == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));

  tt = {0, 1, 30, 8, 's'};
  offsetCode = 8;
  deltaCode = 4;
  ExtendedZoneSpecifier::expandDateTuple(&ttw, &tts, &ttu,
      tt, offsetCode, deltaCode);
  assertTrue((ttw == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));

  tt = {0, 1, 30, 0, 'u'};
  offsetCode = 8;
  deltaCode = 4;
  ExtendedZoneSpecifier::expandDateTuple(&ttw, &tts, &ttu,
      tt, offsetCode, deltaCode);
  assertTrue((ttw == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));
}

test(ExtendedZoneSpecifierTest, calcInteriorYears) {
  const uint8_t kMaxInteriorYears = 4;
  int8_t interiorYears[kMaxInteriorYears];

  uint8_t num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 0, 1, 2, 4);
  assertEqual(0, num);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 5, 7, 2, 4);
  assertEqual(0, num);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 0, 2, 2, 4);
  assertEqual(1, num);
  assertEqual(2, interiorYears[0]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 4, 6, 2, 4);
  assertEqual(1, num);
  assertEqual(4, interiorYears[0]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 3, 3, 2, 4);
  assertEqual(1, num);
  assertEqual(3, interiorYears[0]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1, 5, 2, 4);
  assertEqual(3, num);
  assertEqual(2, interiorYears[0]);
  assertEqual(3, interiorYears[1]);
  assertEqual(4, interiorYears[2]);
}

test(ExtendedZoneSpecifierTest, getMostRecentPriorYear) {
  int8_t yearTiny;

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(0, 1, 2, 4);
  assertEqual(1, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(5, 7, 2, 4);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(0, 2, 2, 4);
  assertEqual(1, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(4, 6, 2, 4);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(3, 3, 2, 4);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(1, 5, 2, 4);
  assertEqual(1, yearTiny);
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
