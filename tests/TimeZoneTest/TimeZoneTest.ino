#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <aunit/fake/FakePrint.h>
#include <AceTime.h>

using namespace aunit;
using namespace aunit::fake;
using namespace ace_time;

// --------------------------------------------------------------------------
// Check that TimeZone::kType* are distinct
// --------------------------------------------------------------------------

test(TimeZoneTest, kType_distinct) {
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeManual);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeBasicManaged);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeExtendedManaged);

  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeBasicManaged);
  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeExtendedManaged);

  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtended);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeBasicManaged);
  assertNotEqual(TimeZone::kTypeBasic, TimeZone::kTypeExtendedManaged);

  assertNotEqual(TimeZone::kTypeExtended, TimeZone::kTypeBasicManaged);
  assertNotEqual(TimeZone::kTypeExtended, TimeZone::kTypeExtendedManaged);

  assertNotEqual(TimeZone::kTypeBasicManaged, TimeZone::kTypeExtendedManaged);
}

// --------------------------------------------------------------------------
// Create ZoneManagers for use later.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
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

// Use ExtendedZoneManager only for non AVR because we run out of RAM.
#if !defined(__AVR__)

const extended::ZoneInfo* const kExtendedZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneManager<2> extendedZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

#endif

// --------------------------------------------------------------------------
// kTypeError
// --------------------------------------------------------------------------

test(TimeZoneTest, error) {
  FakePrint fakePrint;
  auto tz = TimeZone::forError();
  assertEqual(TimeZone::kTypeError, tz.getType());
  tz.printTo(fakePrint);
  assertEqual(F("<Error>"), fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// kTypeManual
// --------------------------------------------------------------------------

test(TimeZoneTest, manual_utc) {
  FakePrint fakePrint;

  auto tz = TimeZone::forUtc();
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(0, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());
  assertTrue(tz.isUtc());

  tz.printTo(fakePrint);
  assertEqual(F("UTC"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual(F("UTC"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual(F("UTC"), fakePrint.getBuffer());
  fakePrint.flush();
}

test(TimeZoneTest, manual_no_dst) {
  FakePrint fakePrint;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());

  tz.printTo(fakePrint);
  assertEqual(F("-08:00+00:00"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual(F("-08:00(STD)"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual(F("STD"), fakePrint.getBuffer());
  fakePrint.flush();
}

test(TimeZoneTest, manual_dst) {
  FakePrint fakePrint;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8),
      TimeOffset::forHour(1));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(60, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());

  tz.printTo(fakePrint);
  assertEqual(F("-08:00+01:00"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printShortTo(fakePrint);
  assertEqual(F("-07:00(DST)"), fakePrint.getBuffer());
  fakePrint.flush();

  tz.printAbbrevTo(fakePrint, 0);
  assertEqual(F("DST"), fakePrint.getBuffer());
  fakePrint.flush();
}

// --------------------------------------------------------------------------
// kTypeBasicManaged + BasicZoneManager
// --------------------------------------------------------------------------

test(TimeZoneBasicTest, createFor) {
  TimeZone a = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  assertTrue(a != b);

  TimeZone aa = basicZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = basicZoneManager.createForZoneId(0x1e2a7654U); // New_York

  assertTrue(a == aa);
  assertTrue(b == bb);
  assertEqual(0x1e2a7654U, bb.getZoneId());
}

test(TimeZoneBasicTest, Los_Angeles) {
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeBasicManaged, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual(F("PST"), fakePrint.getBuffer());
  fakePrint.flush();

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual(F("PDT"), fakePrint.getBuffer());
}

// --------------------------------------------------------------------------
// TimeZoneData
// --------------------------------------------------------------------------

test(TimeZoneDataTest, error) {
  auto tz = TimeZone::forError();
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeError, tzd.type);

  auto tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, utc) {
  auto tz = TimeZone::forUtc();
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeManual, tzd.type);
  assertEqual(0, tzd.stdOffsetCode);
  assertEqual(0, tzd.dstOffsetCode);

  auto tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, manual) {
  auto tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8),
      TimeOffset::forHour(1));
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeManual, tzd.type);
  assertEqual(-8 * 4, tzd.stdOffsetCode);
  assertEqual(4, tzd.dstOffsetCode);

  auto tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, basicManaged) {
  auto tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZoneData::kTypeZoneId, tzd.type);
  assertEqual(0xb7f7e8f2, tzd.zoneId);

  auto tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

#if !defined(__AVR__)
test(TimeZoneDataTest, extendedManaged) {
  auto tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZoneData::kTypeZoneId, tzd.type);
  assertEqual(0xb7f7e8f2, tzd.zoneId);

  auto tzCycle = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

// If we convert a ExtendedManaged, we can read it back as a BasicManaged if
// the ZoneManager supports it. The reverse also works.
test(TimeZoneDataTest, crossed) {
  auto tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZoneData::kTypeZoneId, tzd.type);
  assertEqual(0xb7f7e8f2, tzd.zoneId);

  auto tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz.getZoneId() == tzCycle.getZoneId());
  assertEqual(TimeZone::kTypeBasicManaged, tzCycle.getType());
}
#endif

#if !defined(__AVR__)
// --------------------------------------------------------------------------
// operator==() for kTypeExtendedManaged.
// --------------------------------------------------------------------------

test(TimeZoneExtendedTest, operatorEqualEqual_managedZones) {
  TimeZone manual = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
  TimeZone manual2 = TimeZone::forTimeOffset(TimeOffset::forHour(-7));
  assertTrue(manual != manual2);

  TimeZone basicManaged = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone basicManaged2 = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  assertTrue(basicManaged != basicManaged2);

  TimeZone extendedManaged = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone extendedManaged2 = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);
  assertTrue(extendedManaged != extendedManaged2);

  assertTrue(manual != basicManaged);
  assertTrue(manual != extendedManaged);
  assertTrue(basicManaged != extendedManaged);
}
#endif

#if !defined(__AVR__)
// --------------------------------------------------------------------------
// operator==() for kTypeBasic and kTypeExtended. Takes up too much RAM
// and won't run on a Nano or Micro.
// --------------------------------------------------------------------------

BasicZoneProcessor basicZoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
BasicZoneProcessor basicZoneProcessor2(&zonedb::kZoneAmerica_New_York);
ExtendedZoneProcessor extendedZoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
ExtendedZoneProcessor extendedZoneProcessor2(&zonedbx::kZoneAmerica_New_York);

test(TimeZoneTest, operatorEqualEqual_directZone) {
  TimeZone manual = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
  TimeZone manual2 = TimeZone::forTimeOffset(TimeOffset::forHour(-7));
  assertTrue(manual != manual2);

  TimeZone basic = TimeZone::forZoneProcessor(&basicZoneProcessor);
  TimeZone basic2 = TimeZone::forZoneProcessor(&basicZoneProcessor2);
  assertTrue(basic != basic2);

  TimeZone extended = TimeZone::forZoneProcessor(&extendedZoneProcessor);
  TimeZone extended2 = TimeZone::forZoneProcessor(&extendedZoneProcessor2);
  assertTrue(extended != extended2);

  assertTrue(manual != basic);
  assertTrue(manual != extended);
  assertTrue(basic != extended);
}
#endif

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
