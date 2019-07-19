#line 2 "BasicZoneProcessor.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneProcessor: test private methods
// --------------------------------------------------------------------------

test(BasicZoneProcessorTest, tzVersion) {
  assertEqual("2019a", zonedb::kTzDatabaseVersion);
}

test(BasicZoneProcessorTest, operatorEqualEqual) {
  BasicZoneProcessor a(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneProcessor b(&zonedb::kZoneAustralia_Darwin);
  assertTrue(a != b);
}

test(BasicZoneProcessorTest, setZoneInfo) {
  BasicZoneProcessor zoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  zoneInfo.setZoneInfo(&zonedb::kZoneAustralia_Darwin);
  assertFalse(zoneInfo.mIsFilled);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneInfo.setZoneInfo(&zonedb::kZoneAustralia_Darwin);
  assertTrue(zoneInfo.mIsFilled);
}

test(BasicZoneProcessorTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  basic::MonthDay monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1);
  assertEqual(11, monthDay.month);
  assertEqual(4, monthDay.day);

  // 2018-11, lastSun
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0);
  assertEqual(11, monthDay.month);
  assertEqual(25, monthDay.day);

  // 2018-11, Sun>=30, should shift to 2018-12-2
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 30);
  assertEqual(12, monthDay.month);
  assertEqual(2, monthDay.day);

  // 2018-11, Mon<=7
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -7);
  assertEqual(11, monthDay.month);
  assertEqual(5, monthDay.day);

  // 2018-11, Mon<=1, shifts back into October
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -1);
  assertEqual(10, monthDay.month);
  assertEqual(29, monthDay.day);

  // 2018-03, Thu>=9
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9);
  assertEqual(3, monthDay.month);
  assertEqual(15, monthDay.day);

  // 2018-03-30 exactly
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(2018, 3, 0, 30);
  assertEqual(3, monthDay.month);
  assertEqual(30, monthDay.day);
}

test(BasicZoneProcessorTest, calcRuleOffsetCode) {
  assertEqual(0, BasicZoneProcessor::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, BasicZoneProcessor::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, BasicZoneProcessor::calcRuleOffsetCode(1, 2, 's'));
}

test(BasicZoneProcessorTest, init_primitives) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  zoneProcessor.mYear = 2001;
  zoneProcessor.mNumTransitions = 0;

  zoneProcessor.addRulePriorToYear(2001);
  assertEqual(0, zoneProcessor.mNumTransitions);
  assertEqual(-32, zoneProcessor.mPrevTransition.era.offsetCode());
  assertEqual("P%T", zoneProcessor.mPrevTransition.era.format());
  assertEqual(1967-2000, zoneProcessor.mPrevTransition.rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mPrevTransition.rule.toYearTiny());
  assertEqual(10, zoneProcessor.mPrevTransition.rule.inMonth());

  zoneProcessor.addRulesForYear(2001);
  assertEqual(2, zoneProcessor.mNumTransitions);

  assertEqual(-32, zoneProcessor.mTransitions[0].era.offsetCode());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(1987-2000, zoneProcessor.mTransitions[0].rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mTransitions[0].rule.toYearTiny());
  assertEqual(4, zoneProcessor.mTransitions[0].rule.inMonth());

  assertEqual(-32, zoneProcessor.mTransitions[1].era.offsetCode());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(1967-2000, zoneProcessor.mTransitions[1].rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mTransitions[1].rule.toYearTiny());
  assertEqual(10, zoneProcessor.mTransitions[1].rule.inMonth());

  zoneProcessor.calcTransitions();
  assertEqual((acetime_t) BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneProcessor.mPrevTransition.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneProcessor.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 39434400,
      zoneProcessor.mTransitions[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneProcessor.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 57574800,
      zoneProcessor.mTransitions[1].startEpochSeconds);
}

test(BasicZoneProcessorTest, init) {
  // Test using 2018-01-02. If we use 2018-01-01, the code will populate the
  // cache with transitions from 2017.
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 2);
  zoneProcessor.init(ld);

  assertEqual(2, zoneProcessor.mNumTransitions);

  assertEqual(-32, zoneProcessor.mPrevTransition.era.offsetCode());
  assertEqual("P%T", zoneProcessor.mPrevTransition.era.format());
  assertEqual(2007-2000, zoneProcessor.mPrevTransition.rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mPrevTransition.rule.toYearTiny());
  assertEqual(11, zoneProcessor.mPrevTransition.rule.inMonth());

  assertEqual(-32, zoneProcessor.mTransitions[0].era.offsetCode());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(2007-2000, zoneProcessor.mTransitions[0].rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mTransitions[0].rule.toYearTiny());
  assertEqual(3, zoneProcessor.mTransitions[0].rule.inMonth());

  assertEqual(-32, zoneProcessor.mTransitions[1].era.offsetCode());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(2007-2000, zoneProcessor.mTransitions[1].rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mTransitions[1].rule.toYearTiny());
  assertEqual(11, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual((acetime_t) BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneProcessor.mPrevTransition.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneProcessor.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 574077600,
      zoneProcessor.mTransitions[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneProcessor.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 594637200,
      zoneProcessor.mTransitions[1].startEpochSeconds);
}

test(BasicZoneProcessorTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// Test public methods
// --------------------------------------------------------------------------

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(BasicZoneProcessorTest, kZoneAmerica_Los_Angeles) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(BasicZoneProcessorTest, kZoneAfrica_Johannesburg) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAfrica_Johannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(BasicZoneProcessorTest, kZoneAustralia_Darwin) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAustralia_Darwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHourMinute(9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

test(BasicZoneProcessorTest, kZoneAmerica_Los_Angeles_outOfBounds) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedb::kZoneAmerica_Los_Angeles.zoneContext->startYear);
  assertEqual(2050, zonedb::kZoneAmerica_Los_Angeles.zoneContext->untilYear);

  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2051, 2, 1, 1, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));
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
