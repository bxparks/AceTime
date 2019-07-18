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
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeManaged);

  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeManaged);

  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeManaged);

  assertNotEqual(TimeZone::kTypeExtended, TimeZone::kTypeManaged);
}

// --------------------------------------------------------------------------
// Error TimeZone
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
// BasicZoneManager
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

BasicZoneManager<2> basicZoneManager(
    kBasicZoneRegistrySize, kBasicZoneRegistry);

// --------------------------------------------------------------------------

test(TimeZoneBasicTest, createFor) {
  TimeZone a = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);

  TimeZone aa = basicZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = basicZoneManager.createForZoneId(0x1e2a7654); // New_York

  assertTrue(a != b);
  assertTrue(a == aa);
  assertTrue(b == bb);
}

test(TimeZoneBasicTest, Los_Angeles) {
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeManaged, tz.getType());

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
// ExtendedZoneManager
// --------------------------------------------------------------------------

const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_EXTENDED_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneManager<2> extendedZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

// --------------------------------------------------------------------------

test(TimeZoneExtendedTest, createFor) {
  TimeZone a = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);

  TimeZone aa = extendedZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = extendedZoneManager.createForZoneId(0x1e2a7654); // New_York

  assertTrue(a != b);
  assertTrue(a == aa);
  assertTrue(b == bb);
}

test(TimeZoneExtendedTest, Los_Angeles) {
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeManaged, tz.getType());

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
