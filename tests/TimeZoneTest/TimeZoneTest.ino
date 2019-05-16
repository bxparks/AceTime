#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <aunit/fake/FakePrint.h>
#include <AceTime.h>

using namespace aunit;
using namespace aunit::fake;
using namespace ace_time;

// --------------------------------------------------------------------------
// Default UTC TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, utc) {
  FakePrint fakePrint;
  TimeZone tz;

  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("UTC", fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// Fixed TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, fixed) {
  FakePrint fakePrint;
  TimeZone tz(UtcOffset::forHour(-8));

  assertEqual(TimeZone::kTypeFixed, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("-08:00", fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, operatorEqualEqual) {
  // PST
  ManualZoneSpecifier spa(UtcOffset::forHour(-8), false, "PST", "PDT");
  ManualZoneSpecifier spb(UtcOffset::forHour(-8), false, "PST", "PDT");

  // Two time zones with same zoneSpecifier should be equal.
  TimeZone a(&spa);
  TimeZone b(&spb);
  assertTrue(a == b);

  // One of them goes to DST. Should be different.
  spb.isDst(true);
  assertTrue(a != b);

  // Should be different from EST.
  ManualZoneSpecifier spc(UtcOffset::forHour(-5), false, "EST", "EDT");
  TimeZone c(&spc);
  assertTrue(a != c);
}

test(TimeZoneTest_Manual, forUtcOffset) {
  FakePrint fakePrint;
  ManualZoneSpecifier spec(UtcOffset::forHour(-8), false, "PST", "PDT");
  TimeZone tz(&spec);

  assertEqual(TimeZone::kTypeZoneSpecifier, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("PST", fakePrint.getBuffer());
  fakePrint.flush();

  spec.isDst(true);
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("PDT", fakePrint.getBuffer());
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

test(TimeZoneTest_Basic, LosAngeles) {
  FakePrint fakePrint;
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&zoneSpecifier);
  assertEqual(TimeZone::kTypeZoneSpecifier, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PST", fakePrint.getBuffer());
  fakePrint.flush();

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PDT", fakePrint.getBuffer());
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
