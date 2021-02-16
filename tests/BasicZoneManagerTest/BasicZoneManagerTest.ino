#line 2 "BasicZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;

//---------------------------------------------------------------------------
// BasicZoneManager
//---------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

// Include both the Zone Registry and Link Registry.
BasicZoneManager<1> basicZoneManager(
    kBasicZoneRegistrySize,
    kBasicZoneRegistry,
    zonedb::kLinkRegistrySize,
    zonedb::kLinkRegistry
);

//---------------------------------------------------------------------------

test(BasicZoneManagerTest, registrySize) {
  assertEqual((uint16_t) 4, basicZoneManager.zoneRegistrySize());
  assertEqual(zonedb::kLinkRegistrySize, basicZoneManager.linkRegistrySize());
}

test(BasicZoneManagerTest, createForZoneName) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tzn = basicZoneManager.createForZoneName("America/Los_Angeles");
  assertTrue(tz == tzn);
}

test(BasicZoneManagerTest, createForZoneId) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_New_York);
  TimeZone tzid = basicZoneManager.createForZoneId(
      zonedb::kZoneIdAmerica_New_York);
  assertTrue(tz == tzid);
  assertEqual((uint32_t) 0x1e2a7654, tz.getZoneId());
  assertEqual((uint32_t) 0x1e2a7654, tzid.getZoneId());
}

test(BasicZoneManagerTest, createForZoneIndex) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Chicago);
  TimeZone tzidx = basicZoneManager.createForZoneIndex(0);
  assertTrue(tz == tzidx);
}

test(BasicZoneManagerTest, indexForZoneName) {
  uint16_t index = basicZoneManager.indexForZoneName("America/Los_Angeles");
  assertEqual((uint16_t) 2, index);

  index = basicZoneManager.indexForZoneName("America/not_found");
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(BasicZoneManagerTest, indexForZoneId) {
  uint16_t index = basicZoneManager.indexForZoneId(
      zonedb::kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 3, index);

  index = basicZoneManager.indexForZoneId(0 /* not found */);
  assertEqual(ZoneManager::kInvalidIndex, index);
}

test(BasicZoneManagerTest, createForXxx_create_same_timezone) {
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

//---------------------------------------------------------------------------

test(BasicZoneManagerTest, createForTimeZoneData_error) {
  TimeZone tz = TimeZone::forError();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(BasicZoneManagerTest, createForTimeZoneData_utc) {
  TimeZone tz = TimeZone::forUtc();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{0, 0};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(BasicZoneManagerTest, createForTimeZoneData_manual) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{-8 * 60, 1 * 60};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(BasicZoneManagerTest, createForTimeZoneData_zoneId) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedb::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  // BasicZoneManager should return the same TimeZone
  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

// If we save a kTypeExtended, verify that we can read it back as a kTypeBasic
// if the ZoneManager supports it.
test(BasicZoneManagerTest, createForTimeZoneData_crossed) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{zonedbx::kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertEqual(tz.getZoneId(), tzRoundTrip.getZoneId());
  assertEqual(BasicZoneProcessor::kTypeBasic, tzRoundTrip.getType());
}

//---------------------------------------------------------------------------

// Attempt to create a TimeZone for US/Pacific. There is no entry in the
// kBasicZoneRegistry. But there is a mapping from US/Pacific ->
// America/Los_Angeles in the Link Registry. So we should get back
// America/Los_Angeles.
test(BasicZoneManagerTest, createForZoneId_usingLinkEntry) {
  TimeZone tzPacificByName = basicZoneManager.createForZoneName("US/Pacific");
  assertTrue(tzPacificByName.isError());

  TimeZone tzPacificById = basicZoneManager.createForZoneId(
      zonedb::kZoneIdUS_Pacific);
  TimeZone tzLosAngelesById = basicZoneManager.createForZoneId(
      zonedb::kZoneIdAmerica_Los_Angeles);
  assertTrue(tzPacificById == tzLosAngelesById);
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
