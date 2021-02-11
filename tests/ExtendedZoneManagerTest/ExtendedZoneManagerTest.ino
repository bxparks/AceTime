#line 2 "ExtendedZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;

//---------------------------------------------------------------------------
// ExtendedZoneManager
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// TimeZone + ExtendedZoneManager
//---------------------------------------------------------------------------

test(ExtendedZoneManagerTest, registrySize) {
  assertEqual((uint16_t) 4, extendedZoneManager.registrySize());
}

test(ExtendedZoneManagerTest, createForZoneName) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tzn = extendedZoneManager.createForZoneName("America/Los_Angeles");
  assertTrue(tz == tzn);
}

test(ExtendedZoneManagerTest, createForZoneId) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);
  TimeZone tzid = extendedZoneManager.createForZoneId(
      zonedb::kZoneIdAmerica_New_York);
  assertTrue(tz == tzid);
  assertEqual((uint32_t) 0x1e2a7654, tz.getZoneId());
  assertEqual((uint32_t) 0x1e2a7654, tzid.getZoneId());
}

test(ExtendedZoneManagerTest, createForZoneIndex) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Chicago);
  TimeZone tzidx = extendedZoneManager.createForZoneIndex(0);
  assertTrue(tz == tzidx);
}

test(ExtendedZoneManagerTest, indexForZoneName) {
  uint16_t index = extendedZoneManager.indexForZoneName("America/Los_Angeles");
  assertEqual((uint16_t) 2, index);

  index = extendedZoneManager.indexForZoneName("America/not_found");
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(ExtendedZoneManagerTest, indexForZoneId) {
  uint16_t index = extendedZoneManager.indexForZoneId(
      zonedbx::kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 3, index);

  index = extendedZoneManager.indexForZoneId(0 /* not found */);
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(ExtendedZoneManagerTest, createForXxx_create_same_timezone) {
  TimeZone a = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);
  assertTrue(a != b);

  TimeZone aa = extendedZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = extendedZoneManager.createForZoneId(0x1e2a7654U); // New_York

  assertTrue(a == aa);
  assertTrue(b == bb);
  assertEqual((uint32_t) 0x1e2a7654, bb.getZoneId());
}

//---------------------------------------------------------------------------

test(ExtendedZoneManagerTest, createForTimeZoneData_error) {
  TimeZone tz = TimeZone::forError();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ExtendedZoneManagerTest, createForTimeZoneData_utc) {
  TimeZone tz = TimeZone::forUtc();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{0, 0};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ExtendedZoneManagerTest, createForTimeZoneData_manual) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{-8 * 60, 1 * 60};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ExtendedZoneManagerTest, createForTimeZoneData_zoneId) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedb::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  // ExtendedZoneManager should return the same TimeZone
  TimeZone tzRoundTrip = extendedZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

// If we save a kTypeBasic, verify that we can read it back as a kTypeExtended
// if the ZoneManager supports it.
test(ExtendedZoneManagerTest, createForTimeZoneData_crossed) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedb::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = extendedZoneManager.createForTimeZoneData(tzd);
  assertEqual(tz.getZoneId(), tzRoundTrip.getZoneId());
  assertEqual(ExtendedZoneProcessor::kTypeExtended, tzRoundTrip.getType());
}

//---------------------------------------------------------------------------

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
