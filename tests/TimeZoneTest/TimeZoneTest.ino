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

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertTrue(tz.isUtc());

  tz.printTo(fakePrint);
  assertEqual("UTC", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual("UTC", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("UTC", fakePrint.getBuffer());
  fakePrint.flush();
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, manual_no_dst) {
  FakePrint fakePrint;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());

  tz.printTo(fakePrint);
  assertEqual("-08:00+00:00", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual("-08:00(STD)", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("STD", fakePrint.getBuffer());
  fakePrint.flush();
}

test(TimeZoneTest, manual_dst) {
  FakePrint fakePrint;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8),
      TimeOffset::forHour(1));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(60, tz.getDeltaOffset(0).toMinutes());

  tz.printTo(fakePrint);
  assertEqual("-08:00+01:00", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual("-07:00(DST)", fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual("DST", fakePrint.getBuffer());
  fakePrint.flush();
}

// --------------------------------------------------------------------------
// TimeZone using BasicZoneSpecifier
// --------------------------------------------------------------------------

test(TimeZoneTest_Basic, operatorEqualEqual) {
  BasicZoneSpecifier zoneSpecifierLA(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneSpecifier zoneSpecifierNY(&zonedb::kZoneAmerica_New_York);
  TimeZone a = TimeZone::forZoneSpecifier(&zoneSpecifierLA);
  TimeZone b = TimeZone::forZoneSpecifier(&zoneSpecifierNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Basic, copyConstructor) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone a = TimeZone::forZoneSpecifier(&zoneSpecifier);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Basic, Los_Angeles) {
  FakePrint fakePrint;
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  assertEqual(TimeZone::kTypeBasic, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PST", fakePrint.getBuffer());
  fakePrint.flush();

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PDT", fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// TimeZone using ExtendedZoneSpecifier
// --------------------------------------------------------------------------

test(TimeZoneTest_Extended, operatorEqualEqual) {
  ExtendedZoneSpecifier zoneSpecifierLA(&zonedbx::kZoneAmerica_Los_Angeles);
  ExtendedZoneSpecifier zoneSpecifierNY(&zonedbx::kZoneAmerica_New_York);
  TimeZone a = TimeZone::forZoneSpecifier(&zoneSpecifierLA);
  TimeZone b = TimeZone::forZoneSpecifier(&zoneSpecifierNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Extended, copyConstructor) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone a = TimeZone::forZoneSpecifier(&zoneSpecifier);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Extended, Los_Angeles) {
  FakePrint fakePrint;
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  assertEqual(TimeZone::kTypeExtended, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PST", fakePrint.getBuffer());
  fakePrint.flush();

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PDT", fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// TimeZoneData
// --------------------------------------------------------------------------

test(TimeZoneDataTest, TypeBasic) {
  BasicZoneSpecifier spec(&zonedb::kZoneAmerica_Los_Angeles);
  auto tz = TimeZone::forZoneSpecifier(&spec);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeBasic, tzd.type);
  assertEqual((intptr_t) &zonedb::kZoneAmerica_Los_Angeles,
      (intptr_t) tzd.basicZoneInfo);

  auto tzCircle = TimeZone::forTimeZoneData(tzd, &spec, nullptr);
  assertTrue(tz == tzCircle);
}

test(TimeZoneDataTest, TypeExtended) {
  ExtendedZoneSpecifier spec(&zonedbx::kZoneAmerica_Los_Angeles);
  auto tz = TimeZone::forZoneSpecifier(&spec);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeExtended, tzd.type);
  assertEqual((intptr_t) &zonedbx::kZoneAmerica_Los_Angeles,
      (intptr_t) tzd.extendedZoneInfo);

  auto tzCircle = TimeZone::forTimeZoneData(tzd, nullptr, &spec);
  assertTrue(tz == tzCircle);
}

test(TimeZoneDataTest, operatorEqualEqual) {
  auto tz1 = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
  auto tzd1 = tz1.toTimeZoneData();
  auto tzd1Copy = tz1.toTimeZoneData();

  BasicZoneSpecifier spec2(&zonedb::kZoneAmerica_Los_Angeles);
  auto tz2 = TimeZone::forZoneSpecifier(&spec2);
  auto tzd2 = tz2.toTimeZoneData();
  auto tzd2Copy = tz2.toTimeZoneData();

  ExtendedZoneSpecifier spec3(&zonedbx::kZoneAmerica_Los_Angeles);
  auto tz3 = TimeZone::forZoneSpecifier(&spec3);
  auto tzd3 = tz3.toTimeZoneData();
  auto tzd3Copy = tz3.toTimeZoneData();

  assertTrue(tzd1 != tzd2);
  assertTrue(tzd1 != tzd3);
  assertTrue(tzd2 != tzd3);

  assertTrue(tzd1 == tzd1Copy);
  assertTrue(tzd2 == tzd2Copy);
  assertTrue(tzd3 == tzd3Copy);
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
