#line 2 "BasicZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;

//---------------------------------------------------------------------------
// Set up a BasicZoneManager with 4 zones.
//---------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago, // 0, -06:00
  &zonedb::kZoneAmerica_Denver, // 1, -07:00
  &zonedb::kZoneAmerica_Los_Angeles, // 2, -08:00
  &zonedb::kZoneAmerica_New_York, // 3, -05:00
  &zonedb::kZoneAmerica_Toronto, // 4, -05:00
  &zonedb::kZoneAmerica_Vancouver, // 5, -08:00
  &zonedb::kZoneAmerica_Edmonton, // 6, -07:00
  &zonedb::kZoneAmerica_Winnipeg, // 7, -06:00
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

// Include both the Zone Registry and Link Registry.
BasicZoneManager<1> basicZoneManager(
    kBasicZoneRegistrySize,
    kBasicZoneRegistry
);

//---------------------------------------------------------------------------
// TimeZone + BasicZoneManager
//---------------------------------------------------------------------------

test(BasicZoneManagerTest, registrySize) {
  assertEqual(kBasicZoneRegistrySize, basicZoneManager.zoneRegistrySize());
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
// createForTimeZoneData()
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
// ZoneSorter
//---------------------------------------------------------------------------

test(BasicZoneManagerTest, sortIndexes) {
  uint16_t indexes[] = {0, 1, 2, 3, 4, 5, 6, 7};
  ZoneSorter<BasicZoneManager<1>> zoneSorter(basicZoneManager);
  zoneSorter.sortIndexes(indexes, sizeof(indexes)/sizeof(indexes[0]));
  assertEqual(indexes[0], 2); // Los_Angeles, -08
  assertEqual(indexes[1], 5); // Vancouver, -08
  assertEqual(indexes[2], 1); // Denver, -07
  assertEqual(indexes[3], 6); // Edmonton, -07
  assertEqual(indexes[4], 0); // Chicago, -06
  assertEqual(indexes[5], 7); // Winnipeg, -06
  assertEqual(indexes[6], 3); // New_York, -05
  assertEqual(indexes[7], 4); // Toronto, -05
}

test(BasicZoneManagerTest, sortIds) {
  uint32_t ids[] = {
    zonedb::kZoneIdAmerica_Chicago,
    zonedb::kZoneIdAmerica_Denver,
    zonedb::kZoneIdAmerica_Los_Angeles,
    zonedb::kZoneIdAmerica_New_York,
    zonedb::kZoneIdAmerica_Toronto,
    zonedb::kZoneIdAmerica_Vancouver,
    zonedb::kZoneIdAmerica_Edmonton,
    zonedb::kZoneIdAmerica_Winnipeg,
  };
  ZoneSorter<BasicZoneManager<1>> zoneSorter(basicZoneManager);
  zoneSorter.sortIds(ids, sizeof(ids)/sizeof(ids[0]));
  assertEqual(ids[0], zonedbx::kZoneIdAmerica_Los_Angeles);
  assertEqual(ids[1], zonedbx::kZoneIdAmerica_Vancouver);
  assertEqual(ids[2], zonedbx::kZoneIdAmerica_Denver);
  assertEqual(ids[3], zonedbx::kZoneIdAmerica_Edmonton);
  assertEqual(ids[4], zonedbx::kZoneIdAmerica_Chicago);
  assertEqual(ids[5], zonedbx::kZoneIdAmerica_Winnipeg);
  assertEqual(ids[6], zonedbx::kZoneIdAmerica_New_York);
  assertEqual(ids[7], zonedbx::kZoneIdAmerica_Toronto);
}

test(BasicZoneManagerTest, sortNames) {
  const char* names[] = {
    "America/Chicago",
    "America/Denver",
    "America/Los_Angeles",
    "America/New_York",
    "America/Toronto",
    "America/Vancouver",
    "America/Edmonton",
    "America/Winnipeg",
  };
  ZoneSorter<BasicZoneManager<1>> zoneSorter(basicZoneManager);
  zoneSorter.sortNames(names, sizeof(names)/sizeof(names[0]));
  assertEqual(names[0], "America/Los_Angeles");
  assertEqual(names[1], "America/Vancouver");
  assertEqual(names[2], "America/Denver");
  assertEqual(names[3], "America/Edmonton");
  assertEqual(names[4], "America/Chicago");
  assertEqual(names[5], "America/Winnipeg");
  assertEqual(names[6], "America/New_York");
  assertEqual(names[7], "America/Toronto");
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
