#line 2 "BasicZoneSpecifier.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneSpecifier: test private methods
// --------------------------------------------------------------------------

test(BasicZoneSpecifierTest, tzVersion) {
  assertEqual("2019a", zonedb::kTzDatabaseVersion);
}

test(BasicZoneSpecifierTest, operatorEqualEqual) {
  BasicZoneSpecifier a(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneSpecifier b(&zonedb::kZoneAustralia_Darwin);
  assertTrue(a != b);
}

test(BasicZoneSpecifierTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  basic::MonthDay monthDay = BasicZoneSpecifier::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1);
  assertEqual(11, monthDay.month);
  assertEqual(4, monthDay.day);

  // 2018-11, lastSun
  monthDay = BasicZoneSpecifier::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0);
  assertEqual(11, monthDay.month);
  assertEqual(25, monthDay.day);

  // 2018-03, Thu>=9
  monthDay = BasicZoneSpecifier::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9);
  assertEqual(3, monthDay.month);
  assertEqual(15, monthDay.day);

  // 2018-03-30
  monthDay = BasicZoneSpecifier::calcStartDayOfMonth(2018, 3, 0, 30);
  assertEqual(3, monthDay.month);
  assertEqual(30, monthDay.day);
}

test(BasicZoneSpecifierTest, calcRuleOffsetCode) {
  assertEqual(0, BasicZoneSpecifier::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, BasicZoneSpecifier::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, BasicZoneSpecifier::calcRuleOffsetCode(1, 2, 's'));
}

test(BasicZoneSpecifierTest, init_primitives) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  zoneSpecifier.mYear = 2001;
  zoneSpecifier.mNumTransitions = 0;

  zoneSpecifier.addRulePriorToYear(2001);
  assertEqual(0, zoneSpecifier.mNumTransitions);
  assertEqual(-32, zoneSpecifier.mPrevTransition.era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mPrevTransition.era->format);
  assertEqual(1967-2000, zoneSpecifier.mPrevTransition.rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mPrevTransition.rule->toYearTiny);
  assertEqual(10, zoneSpecifier.mPrevTransition.rule->inMonth);

  zoneSpecifier.addRulesForYear(2001);
  assertEqual(2, zoneSpecifier.mNumTransitions);

  assertEqual(-32, zoneSpecifier.mTransitions[0].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[0].era->format);
  assertEqual(1987-2000, zoneSpecifier.mTransitions[0].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mTransitions[0].rule->toYearTiny);
  assertEqual(4, zoneSpecifier.mTransitions[0].rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[1].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[1].era->format);
  assertEqual(1967-2000, zoneSpecifier.mTransitions[1].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mTransitions[1].rule->toYearTiny);
  assertEqual(10, zoneSpecifier.mTransitions[1].rule->inMonth);

  zoneSpecifier.calcTransitions();
  assertEqual((acetime_t) BasicZoneSpecifier::kMinEpochSeconds,
      zoneSpecifier.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneSpecifier.mPrevTransition.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpecifier.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 39434400,
      zoneSpecifier.mTransitions[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpecifier.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 57574800,
      zoneSpecifier.mTransitions[1].startEpochSeconds);
}

test(BasicZoneSpecifierTest, init) {
  // Test using 2018-01-02. If we use 2018-01-01, the code will populate the
  // cache with transitions from 2017.
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 2);
  zoneSpecifier.init(ld);

  assertEqual(2, zoneSpecifier.mNumTransitions);

  assertEqual(-32, zoneSpecifier.mPrevTransition.era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mPrevTransition.era->format);
  assertEqual(2007-2000, zoneSpecifier.mPrevTransition.rule->fromYearTiny);
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mPrevTransition.rule->toYearTiny);
  assertEqual(11, zoneSpecifier.mPrevTransition.rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[0].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[0].era->format);
  assertEqual(2007-2000, zoneSpecifier.mTransitions[0].rule->fromYearTiny);
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mTransitions[0].rule->toYearTiny);
  assertEqual(3, zoneSpecifier.mTransitions[0].rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[1].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[1].era->format);
  assertEqual(2007-2000, zoneSpecifier.mTransitions[1].rule->fromYearTiny);
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mTransitions[1].rule->toYearTiny);
  assertEqual(11, zoneSpecifier.mTransitions[1].rule->inMonth);

  assertEqual((acetime_t) BasicZoneSpecifier::kMinEpochSeconds,
      zoneSpecifier.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneSpecifier.mPrevTransition.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpecifier.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 574077600,
      zoneSpecifier.mTransitions[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpecifier.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 594637200,
      zoneSpecifier.mTransitions[1].startEpochSeconds);
}

test(BasicZoneSpecifierTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  BasicZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// Test public methods
// --------------------------------------------------------------------------

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(BasicZoneSpecifierTest, kZoneAmerica_Los_Angeles) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(BasicZoneSpecifierTest, kZoneAfrica_Johannesburg) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAfrica_Johannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(BasicZoneSpecifierTest, kZoneAustralia_Darwin) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAustralia_Darwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHourMinute(9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());
}

test(BasicZoneSpecifierTest, kZoneAmerica_Los_Angeles_outOfBounds) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedb::kZoneAmerica_Los_Angeles.zoneContext->startYear);
  assertEqual(2050, zonedb::kZoneAmerica_Los_Angeles.zoneContext->untilYear);

  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneSpecifier.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneSpecifier.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2051, 2, 1, 1, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneSpecifier.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneSpecifier.getAbbrev(epochSeconds));
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
