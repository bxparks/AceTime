#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <aunit/fake/FakePrint.h>
#include <AceTime.h>

using namespace aunit;
using namespace aunit::fake;
using namespace ace_time;

// --------------------------------------------------------------------------
// Check TimeZone::kType* values
// --------------------------------------------------------------------------

test(TimeZoneTest, kType_distinct) {
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeManual);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtended);
}

// --------------------------------------------------------------------------
// Default UTC TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, error) {
  FakePrint fakePrint;
  auto tz = TimeZone::forError();
  assertEqual(TimeZone::kTypeError, tz.getType());
  tz.printTo(fakePrint);
  assertEqual("<Error>", fakePrint.getBuffer());
}

test(TimeZoneTest, utc) {
  FakePrint fakePrint;

  auto tz = TimeZone::forUtc();
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

const BasicZoneManager basicZoneManager(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
BasicZoneSpecifierCache<2> basicZoneSpecifierCache(basicZoneManager);

test(TimeZoneTest_Basic, operatorEqualEqual) {
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);

  TimeZone a = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_New_York);

  assertTrue(a != b);
}

test(TimeZoneTest_Basic, copyConstructor) {
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);

  TimeZone a = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Basic, Los_Angeles) {
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
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

const ExtendedZoneManager extendedZoneManager(
    zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
ExtendedZoneSpecifierCache<2> extendedZoneSpecifierCache(extendedZoneManager);

test(TimeZoneTest_Extended, operatorEqualEqual) {
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);

  TimeZone a = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_New_York);

  assertTrue(a != b);
}

test(TimeZoneTest_Extended, copyConstructor) {
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);
  TimeZone a = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Extended, Los_Angeles) {
  TimeZone::setZoneSpecifierCache(&extendedZoneSpecifierCache);
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
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
  TimeZone::setZoneSpecifierCache(&basicZoneSpecifierCache);

  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeBasic, tzd.type);
  assertEqual((intptr_t) &zonedb::kZoneAmerica_Los_Angeles,
      (intptr_t) tzd.zoneInfo);

  auto tzCircle = TimeZone::forTimeZoneData(tzd);
  assertTrue(tz == tzCircle);
}

test(TimeZoneDataTest, TypeExtended) {
  TimeZone::setZoneSpecifierCache(&extendedZoneSpecifierCache);

  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeExtended, tzd.type);
  assertEqual((intptr_t) &zonedbx::kZoneAmerica_Los_Angeles,
      (intptr_t) tzd.zoneInfo);

  auto tzCircle = TimeZone::forTimeZoneData(tzd);
  assertTrue(tz == tzCircle);
}

test(TimeZoneDataTest, operatorEqualEqual) {
  auto tz1 = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
  auto tzd1 = tz1.toTimeZoneData();
  auto tzd1Copy = tz1.toTimeZoneData();

  auto tz2 = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto tzd2 = tz2.toTimeZoneData();
  auto tzd2Copy = tz2.toTimeZoneData();

  auto tz3 = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
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
