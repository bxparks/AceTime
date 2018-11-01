#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ZoneManager
// --------------------------------------------------------------------------

test(ZoneManagerTest, init_2001) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  manager.mYear = 1;
  manager.mNumMatches = 0;

  manager.addLastYear();
  assertEqual(0, manager.mNumMatches);
  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual(0, manager.mPreviousMatch.rule->fromYear);
  assertEqual(6, manager.mPreviousMatch.rule->toYear);
  assertEqual(10, manager.mPreviousMatch.rule->inMonth);

  manager.addCurrentYear();
  assertEqual(2, manager.mNumMatches);

  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual(0, manager.mMatches[0].rule->fromYear);
  assertEqual(6, manager.mMatches[0].rule->toYear);
  assertEqual(4, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual(0, manager.mMatches[1].rule->fromYear);
  assertEqual(6, manager.mMatches[1].rule->toYear);
  assertEqual(10, manager.mMatches[1].rule->inMonth);
  assertEqual(-32, manager.mMatches[1].entry->offsetCode);

  manager.calcTransitions();
  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 39434400, manager.mMatches[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 57574800, manager.mMatches[1].startEpochSeconds);
}

test(ZoneManagerTest, init_2018) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  LocalDate ld = LocalDate::forComponents(18, 1, 1);
  manager.init(ld);

  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual(7, manager.mPreviousMatch.rule->fromYear);
  assertEqual(255, manager.mPreviousMatch.rule->toYear);
  assertEqual(11, manager.mPreviousMatch.rule->inMonth);

  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual(7, manager.mMatches[0].rule->fromYear);
  assertEqual(255, manager.mMatches[0].rule->toYear);
  assertEqual(3, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual(7, manager.mMatches[1].rule->fromYear);
  assertEqual(255, manager.mMatches[1].rule->toYear);
  assertEqual(11, manager.mMatches[1].rule->inMonth);
  assertEqual(-32, manager.mMatches[1].entry->offsetCode);

  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 574077600, manager.mMatches[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 594637200, manager.mMatches[1].startEpochSeconds);
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ZoneManagerTest, getZoneOffset_Los_Angeles) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59,
      ZoneOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 0,
      ZoneOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 0, 0,
      ZoneOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 59, 59,
      ZoneOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 2, 0, 0,
      ZoneOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(ZoneManagerTest, getZoneOffset_Sydney) {
  ZoneManager manager(&ZoneInfo::kSydney);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(7, 3, 25, 2, 59, 59,
      ZoneOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 3, 25, 3, 0, 0,
      ZoneOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 10, 28, 1, 59, 59,
      ZoneOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 10, 28, 2, 0, 0,
      ZoneOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(ZoneManagerTest, getZoneOffset_Johannesburg) {
  ZoneManager manager(&ZoneInfo::kJohannesburg);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      ZoneOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(8, manager.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("SAST", manager.getAbbrev(epochSeconds));
}

test(ZoneManagerTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  ZoneManager::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// TimeZone
// --------------------------------------------------------------------------

test(TimeZone, operatorEqualEqual) {
  TimeZone tz1;
  TimeZone tz2 = TimeZone::forZoneOffset(
      ZoneOffset::forHour(-8), false, "PST");
  TimeZone tz3 = TimeZone::forZone(&ZoneInfo::kLosAngeles);

  assertTrue(tz1 != tz2);
  assertTrue(tz1 != tz3);
  assertTrue(tz2 != tz3);

  TimeZone tz4 = tz3;
  assertTrue(tz4 == tz3);
}

test(FixedTimeZone, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(0, tz.getStandardZoneOffset().toOffsetCode());
  assertEqual(false, tz.getStandardDst());
  assertEqual("", tz.getStandardAbbrev());
}

test(FixedTimeZone, standardTime) {
  TimeZone tz = TimeZone::forZoneOffset(
      ZoneOffset::forHour(-8), false, "PST");
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(-32, tz.getStandardZoneOffset().toOffsetCode());
  assertEqual(false, tz.getStandardDst());
  assertEqual("PST", tz.getStandardAbbrev());
}

test(FixedTimeZone, daylightTime) {
  TimeZone tz = TimeZone::forZoneOffset(
      ZoneOffset::forHour(-8), true, "PDT");
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(-28, tz.getStandardZoneOffset().toOffsetCode());
  assertEqual(true, tz.getStandardDst());
  assertEqual("PDT", tz.getStandardAbbrev());
}

// TODO: add tests for forOffsetString()

test(AutoTimeZone, LosAngeles) {
  OffsetDateTime dt;
  uint32_t epochSeconds;

  TimeZone tz = TimeZone::forZone(&ZoneInfo::kLosAngeles);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59,
      ZoneOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, tz.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 0,
      ZoneOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, tz.getZoneOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", tz.getAbbrev(epochSeconds));
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
