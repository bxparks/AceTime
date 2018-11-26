#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ZoneManager
// --------------------------------------------------------------------------

test(ZoneManagerTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  assertEqual(4, ZoneManager::calcStartDayOfMonth(
      18, 11, LocalDate::kSunday, 1));

  // 2018-11, lastSun
  assertEqual(25, ZoneManager::calcStartDayOfMonth(
      18, 11, LocalDate::kSunday, 0));

  // 2018-03, Thu>=9
  assertEqual(15, ZoneManager::calcStartDayOfMonth(
      18, 3, LocalDate::kThursday, 9));

  // 2018-03-30
  assertEqual(30, ZoneManager::calcStartDayOfMonth(18, 3, 0, 30));
}

test(ZoneManagerTest, calcRuleOffsetCode) {
  assertEqual(0, ZoneManager::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, ZoneManager::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, ZoneManager::calcRuleOffsetCode(1, 2, 's'));
}

test(ZoneManagerTest, init_primitives) {
  ZoneManager manager(&zonedb::kZoneLos_Angeles);
  manager.mYear = 1;
  manager.mNumMatches = 0;

  manager.addRulePriorToYear(1);
  assertEqual(0, manager.mNumMatches);
  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual((uint16_t) 1967, manager.mPreviousMatch.rule->fromYearFull);
  assertEqual((uint16_t) 2006, manager.mPreviousMatch.rule->toYearFull);
  assertEqual(10, manager.mPreviousMatch.rule->inMonth);

  manager.addRulesForYear(1);
  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mMatches[0].entry->offsetCode);
  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual((uint16_t) 1987, manager.mMatches[0].rule->fromYearFull);
  assertEqual((uint16_t) 2006, manager.mMatches[0].rule->toYearFull);
  assertEqual(4, manager.mMatches[0].rule->inMonth);

  assertEqual(-32, manager.mMatches[1].entry->offsetCode);
  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual((uint16_t) 1967, manager.mMatches[1].rule->fromYearFull);
  assertEqual((uint16_t) 2006, manager.mMatches[1].rule->toYearFull);
  assertEqual(10, manager.mMatches[1].rule->inMonth);

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

test(ZoneManagerTest, init) {
  ZoneManager manager(&zonedb::kZoneLos_Angeles);
  LocalDate ld = LocalDate::forComponents(18, 1, 1); // 2018-01-01
  manager.init(ld);

  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual((uint16_t) 2007, manager.mPreviousMatch.rule->fromYearFull);
  assertEqual(ZoneRule::kMaxYear, manager.mPreviousMatch.rule->toYearFull);
  assertEqual(11, manager.mPreviousMatch.rule->inMonth);

  assertEqual(-32, manager.mMatches[0].entry->offsetCode);
  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual((uint16_t) 2007, manager.mMatches[0].rule->fromYearFull);
  assertEqual(ZoneRule::kMaxYear, manager.mMatches[0].rule->toYearFull);
  assertEqual(3, manager.mMatches[0].rule->inMonth);

  assertEqual(-32, manager.mMatches[1].entry->offsetCode);
  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual((uint16_t) 2007, manager.mMatches[1].rule->fromYearFull);
  assertEqual(ZoneRule::kMaxYear, manager.mMatches[1].rule->toYearFull);
  assertEqual(11, manager.mMatches[1].rule->inMonth);

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
test(ZoneManagerTest, getUtcOffset_Los_Angeles) {
  ZoneManager manager(&zonedb::kZoneLos_Angeles);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 59, 59,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 11, 4, 2, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(ZoneManagerTest, getUtcOffset_Sydney) {
  ZoneManager manager(&zonedb::kZoneSydney);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(7, 3, 25, 2, 59, 59,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 3, 25, 3, 0, 0,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 10, 28, 1, 59, 59,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(7, 10, 28, 2, 0, 0,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(ZoneManagerTest, getUtcOffset_Johannesburg) {
  ZoneManager manager(&zonedb::kZoneJohannesburg);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      UtcOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(8, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("SAST", manager.getAbbrev(epochSeconds));
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(ZoneManagerTest, getUtcOffset_Darwin) {
  ZoneManager manager(&zonedb::kZoneDarwin);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      UtcOffset::forHourMinute(1, 9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(38, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("ACST", manager.getAbbrev(epochSeconds));
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
  const TimeZone &tz1 = TimeZone::sUtc;
  const TimeZone tz2 = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST");
  const TimeZone tz3 = TimeZone::forZone(&zonedb::kZoneLos_Angeles);

  assertTrue(tz1 != tz2);
  assertTrue(tz1 != tz3);
  assertTrue(tz2 != tz3);

  TimeZone tz4 = tz3;
  assertTrue(tz4 == tz3);
}

test(FixedTimeZone, default) {
  const TimeZone &tz = TimeZone::sUtc;
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(0, tz.getBaseUtcOffset().toOffsetCode());
  assertEqual(false, tz.getBaseDst());
  assertEqual((uintptr_t) 0, (uintptr_t) tz.getStdAbbrev());
  assertEqual((uintptr_t) 0, (uintptr_t) tz.getDstAbbrev());
}

test(FixedTimeZone, standardTime) {
  const TimeZone tz = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(-32, tz.getUtcOffset().toOffsetCode());
  assertEqual(false, tz.getBaseDst());
  assertEqual("PST", tz.getAbbrev());
}

test(FixedTimeZone, daylightTime) {
  TimeZone tz = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), true, "PST", "PDT");
  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(-28, tz.getUtcOffset().toOffsetCode());
  assertEqual(true, tz.getBaseDst());
  assertEqual("PDT", tz.getAbbrev());
}

// TODO: add tests for forOffsetString()

test(AutoTimeZone, LosAngeles) {
  OffsetDateTime dt;
  uint32_t epochSeconds;

  TimeZone tz = TimeZone::forZone(&zonedb::kZoneLos_Angeles);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, tz.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, tz.getUtcOffset(epochSeconds).toOffsetCode());
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
