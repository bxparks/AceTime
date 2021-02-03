#line 2 "TimeZoneTest.ino"

/*
 * The TimeZone, TimeZoneData, ManualZoneManager, BasicZoneManager and
 * ExtendedZoneManager classes are tightly interrelated, so we will test all of
 * those in this single test file.
 */

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>

using namespace aunit;
using ace_common::PrintStr;
using namespace ace_time;

// --------------------------------------------------------------------------
// Check that TimeZone::kType* are distinct
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

BasicZoneManager<1> basicZoneManager(
    kBasicZoneRegistrySize, kBasicZoneRegistry);

// --------------------------------------------------------------------------

// Use ExtendedZoneManager only for non AVR because we run out of RAM.
#if ! defined(ARDUINO_ARCH_AVR)

const extended::ZoneInfo* const kExtendedZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneManager<1> extendedZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

#endif

// --------------------------------------------------------------------------
// kTypeError
// --------------------------------------------------------------------------

test(TimeZoneTest, forError) {
  PrintStr<16> printStr;
  auto tz = TimeZone::forError();
  assertEqual(TimeZone::kTypeError, tz.getType());
  tz.printTo(printStr);
  assertEqual(F("<Error>"), printStr.getCstr());
}

// --------------------------------------------------------------------------
// TimeZone (kTypeManual)
// --------------------------------------------------------------------------

test(TimeZoneTest, manual_utc) {
  PrintStr<16> printStr;

  auto tz = TimeZone::forUtc();
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(0, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());
  assertTrue(tz.isUtc());
  assertEqual(F("UTC"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("UTC"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("UTC"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_no_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());
  assertEqual(F("STD"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("-08:00+00:00"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-08:00(STD)"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(
      TimeOffset::forHours(-8),
      TimeOffset::forHours(1)
  );

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(60, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());
  assertEqual(F("DST"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("-08:00+01:00"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-07:00(DST)"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forHours) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(1*60, tz.getDstOffset().toMinutes());
}

test(TimeZoneTest, forMinutes) {
  TimeZone tz = TimeZone::forMinutes(-120, 60);
  assertEqual(-120, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());
}

test(TimeZoneTest, forHourMinute) {
  TimeZone tz = TimeZone::forHourMinute(-4, -15, 1, 30);
  assertEqual(-4*60-15, tz.getStdOffset().toMinutes());
  assertEqual(1*60+30, tz.getDstOffset().toMinutes());
}

// --------------------------------------------------------------------------
// TimeZone + ManualZoneManager
// --------------------------------------------------------------------------

test(TimeZoneManualTest, registrySize) {
  ManualZoneManager manualZoneManager;
  assertEqual((uint16_t) 0, manualZoneManager.registrySize());
}

test(TimeZoneManualTest, createForTimeZoneData) {
  ManualZoneManager manualZoneManager;
  TimeZoneData zone{-8 * 60, 1 * 60};
  TimeZone tz = manualZoneManager.createForTimeZoneData(zone);
  TimeZoneData tzd = tz.toTimeZoneData();
  assertTrue(zone == tzd);
}

test(TimeZoneManualTest, createForTimeZoneData_error) {
  ManualZoneManager manualZoneManager;
  TimeZoneData zone{}; // error
  TimeZone tz = manualZoneManager.createForTimeZoneData(zone);
  assertTrue(tz.isError());
}

test(TimeZoneManualTest, createForZoneId) {
  ManualZoneManager manualZoneManager;
  TimeZone tz = manualZoneManager.createForZoneId(
      zonedb::kZoneIdAmerica_Los_Angeles);
  assertTrue(tz.isError());
}

// --------------------------------------------------------------------------
// TimeZone + BasicZoneManager
// --------------------------------------------------------------------------

test(TimeZoneBasicTest, registrySize) {
  assertEqual((uint16_t) 4, basicZoneManager.registrySize());
}

test(TimeZoneBasicTest, createForZoneName) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tzn = basicZoneManager.createForZoneName("America/Los_Angeles");
  assertTrue(tz == tzn);
}

test(TimeZoneBasicTest, createForZoneId) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  TimeZone tzid = basicZoneManager.createForZoneId(
      zonedb::kZoneIdAmerica_New_York);
  assertTrue(tz == tzid);
  assertEqual((uint32_t) 0x1e2a7654, tz.getZoneId());
  assertEqual((uint32_t) 0x1e2a7654, tzid.getZoneId());
}

test(TimeZoneBasicTest, createForZoneIndex) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Chicago);
  TimeZone tzidx = basicZoneManager.createForZoneIndex(0);
  assertTrue(tz == tzidx);
}

test(TimeZoneBasicTest, indexForZoneName) {
  uint16_t index = basicZoneManager.indexForZoneName("America/Los_Angeles");
  assertEqual((uint16_t) 2, index);

  index = basicZoneManager.indexForZoneName("America/not_found");
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(TimeZoneBasicTest, indexForZoneId) {
  uint16_t index = basicZoneManager.indexForZoneId(
      zonedb::kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 3, index);

  index = basicZoneManager.indexForZoneId(0 /* not found */);
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(TimeZoneBasicTest, createForXxx_create_same_timezone) {
  TimeZone a = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  assertTrue(a != b);

  TimeZone aa = basicZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = basicZoneManager.createForZoneId(0x1e2a7654U); // New_York

  assertTrue(a == aa);
  assertTrue(b == bb);
  assertEqual((uint32_t) 0x1e2a7654, bb.getZoneId());
}

test(TimeZoneBasicTest, Los_Angeles) {
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeBasic, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PST"), tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PDT"), tz.getAbbrev(epochSeconds));
}

// --------------------------------------------------------------------------
// TimeZoneData
// --------------------------------------------------------------------------

// We can use initializer lists, just like regular structs!
test(TimeZoneDataTest, array_initialization) {
  TimeZoneData zones[3] = {
    {}, // kTypeError
    {1, 2}, // kTypeManual
    {zonedb::kZoneIdAmerica_Los_Angeles}, // kTypeZoneId
  };
  assertTrue(TimeZoneData() == zones[0]);
  assertTrue(TimeZoneData(1, 2) == zones[1]);
  assertTrue(TimeZoneData(zonedb::kZoneIdAmerica_Los_Angeles) == zones[2]);
}

test(TimeZoneDataTest, error) {
  TimeZone tz = TimeZone::forError();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{};
  assertTrue(expected == tzd);

  TimeZone tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, utc) {
  TimeZone tz = TimeZone::forUtc();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{0, 0};
  assertTrue(expected == tzd);

  TimeZone tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, manual) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{-8 * 60, 1 * 60};
  assertTrue(expected == tzd);

  TimeZone tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

test(TimeZoneDataTest, basic) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedb::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  TimeZone tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

#if ! defined(ARDUINO_ARCH_AVR)
test(TimeZoneDataTest, extended) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedbx::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  TimeZone tzCycle = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzCycle);
}

// If we convert a kTypeExtended, we can read it back as a
// kTypeBasic if the ZoneManager supports it. The reverse also works.
test(TimeZoneDataTest, crossed) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedbx::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  TimeZone tzCycle = basicZoneManager.createForTimeZoneData(tzd);
  assertEqual(tz.getZoneId(), tzCycle.getZoneId());
  assertEqual(TimeZone::kTypeBasic, tzCycle.getType());
}
#endif

#if ! defined(ARDUINO_ARCH_AVR)
// --------------------------------------------------------------------------
// operator==() for kTypeExtended.
// --------------------------------------------------------------------------

test(TimeZoneExtendedTest, operatorEqualEqual) {
  TimeZone manual = TimeZone::forHours(-8);
  TimeZone manual2 = TimeZone::forHours(-7);
  assertTrue(manual != manual2);

  TimeZone basic = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone basic2 = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  assertTrue(basic != basic2);

  TimeZone extended = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone extended2 = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);
  assertTrue(extended != extended2);

  assertTrue(manual != basic);
  assertTrue(manual != extended);
  assertTrue(basic != extended);
}
#endif

// --------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
