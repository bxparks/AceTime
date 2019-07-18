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
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeBasicSpecifier);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeExtendedSpecifier);

  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasicSpecifier);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtendedSpecifier);

  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeBasicSpecifier);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtendedSpecifier);

  assertNotEqual(TimeZone::kTypeExtended, TimeZone::kTypeBasicSpecifier);
  assertNotEqual(TimeZone::kTypeExtended, TimeZone::kTypeExtendedSpecifier);

  assertNotEqual(TimeZone::kTypeBasicSpecifier,
      TimeZone::kTypeExtendedSpecifier);
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

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, manual_utc) {
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
// TimeZoneData
// --------------------------------------------------------------------------

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
