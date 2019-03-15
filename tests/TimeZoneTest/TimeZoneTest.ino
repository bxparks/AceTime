#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ManualZoneSpecifier
// --------------------------------------------------------------------------

test(ManualZoneSpecifierTest, accessors) {
  ManualZoneSpecifier pstSpec(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  assertEqual(-8*60, pstSpec.getUtcOffset(0).toMinutes());
  assertEqual("PST", pstSpec.getAbbrev(0));
  assertEqual(0, pstSpec.getDeltaOffset(0).toMinutes());

  pstSpec.isDst(true);

  assertEqual(-7*60, pstSpec.getUtcOffset(0).toMinutes());
  assertEqual("PDT", pstSpec.getAbbrev(0));
  assertEqual(1*60, pstSpec.getDeltaOffset(0).toMinutes());
}

test(ManualZoneSpecifierTest, copyConstructor) {
  ManualZoneSpecifier a(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpecifier b(a);

  assertEqual(a.isDst(), b.isDst());
  assertEqual(a.stdOffset().toMinutes(), b.stdOffset().toMinutes());
  assertEqual(a.stdAbbrev(), b.stdAbbrev());
  assertEqual(a.deltaOffset().toMinutes(), b.deltaOffset().toMinutes());
  assertEqual(a.dstAbbrev(), b.dstAbbrev());

  b.isDst(true);
  assertNotEqual(a.isDst(), b.isDst());
}

test(ManualZoneSpecifierTest, operatorEqualEqual) {
  ManualZoneSpecifier a(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpecifier b(
      UtcOffset::forHour(2), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpecifier c(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "A", "b");
  ManualZoneSpecifier d(
      UtcOffset::forHour(1), UtcOffset::forHour(2), "a", "b");
  ManualZoneSpecifier e(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "B");

  assertTrue(a != b);
  assertTrue(a != c);
  assertTrue(a != d);
  assertTrue(a != e);

  ManualZoneSpecifier aa(a);
  assertTrue(a == aa);

  aa.isDst(true);
  assertTrue(a != aa);
}

// --------------------------------------------------------------------------
// Default TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, default) {
  TimeZone tz;

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, operatorEqualEqual) {
  // PST
  ManualZoneSpecifier spa(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpecifier spb(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  // Two time zones with same zoneSpecifier should be equal.
  TimeZone a(&spa);
  TimeZone b(&spb);
  assertTrue(a == b);

  // One of them goes to DST. Should be different.
  spb.isDst(true);
  assertTrue(a != b);

  // Should be different from EST.
  ManualZoneSpecifier spc(
      UtcOffset::forHour(-5), UtcOffset::forHour(1), "EST", "EDT");
  TimeZone c(&spc);
  assertTrue(a != c);
}

test(TimeZoneTest_Manual, forUtcOffset) {
  ManualZoneSpecifier zoneSpecifier(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  TimeZone tz(&zoneSpecifier);

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PST", tz.getAbbrev(0));

  zoneSpecifier.isDst(true);
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PDT", tz.getAbbrev(0));
}

// --------------------------------------------------------------------------
// Auto TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Auto, operatorEqualEqual) {
  BasicZoneSpecifier zoneSpecifierLA(&zonedb::kZoneLos_Angeles);
  BasicZoneSpecifier zoneSpecifierNY(&zonedb::kZoneNew_York);
  TimeZone a(&zoneSpecifierLA);
  TimeZone b(&zoneSpecifierNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Auto, copyConstructor) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone a(&zoneSpecifier);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Auto, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
}

test(TimeZoneTest_Auto, LosAngeles) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&zoneSpecifier);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", tz.getAbbrev(epochSeconds));
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
