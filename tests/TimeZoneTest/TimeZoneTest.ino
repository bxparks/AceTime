#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// Default TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, default) {
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
// TimeZone using BasicZoneSpecifier
// --------------------------------------------------------------------------

test(TimeZoneTest_Basic, operatorEqualEqual) {
  BasicZoneSpecifier zoneSpecifierLA(&zonedb::kZoneLos_Angeles);
  BasicZoneSpecifier zoneSpecifierNY(&zonedb::kZoneNew_York);
  TimeZone a(&zoneSpecifierLA);
  TimeZone b(&zoneSpecifierNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Basic, copyConstructor) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone a(&zoneSpecifier);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Basic, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
}

test(TimeZoneTest_Basic, LosAngeles) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&zoneSpecifier);
  assertEqual(TimeZone::kTypeBasic, tz.getType());

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

// TODO: Add tests for ExtendedZoneSpecifier

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
