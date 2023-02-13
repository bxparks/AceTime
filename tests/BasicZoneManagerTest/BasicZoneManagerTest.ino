#line 2 "BasicZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <tzonedb/zone_policies.h>
#include <tzonedb/zone_infos.h>

using namespace ace_time;
using ace_time::tzonedb::kZoneAmerica_Chicago;
using ace_time::tzonedb::kZoneAmerica_Denver;
using ace_time::tzonedb::kZoneAmerica_Los_Angeles;
using ace_time::tzonedb::kZoneAmerica_New_York;
using ace_time::tzonedb::kZoneAmerica_Toronto;
using ace_time::tzonedb::kZoneAmerica_Vancouver;
using ace_time::tzonedb::kZoneAmerica_Edmonton;
using ace_time::tzonedb::kZoneAmerica_Winnipeg;
using ace_time::tzonedb::kZoneIdAmerica_Chicago;
using ace_time::tzonedb::kZoneIdAmerica_Denver;
using ace_time::tzonedb::kZoneIdAmerica_Los_Angeles;
using ace_time::tzonedb::kZoneIdAmerica_New_York;
using ace_time::tzonedb::kZoneIdAmerica_Toronto;
using ace_time::tzonedb::kZoneIdAmerica_Vancouver;
using ace_time::tzonedb::kZoneIdAmerica_Edmonton;
using ace_time::tzonedb::kZoneIdAmerica_Winnipeg;

//---------------------------------------------------------------------------
// Set up a BasicZoneManager with a handful of zones.
//---------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &kZoneAmerica_Chicago, // 0, -06:00
  &kZoneAmerica_Denver, // 1, -07:00
  &kZoneAmerica_Los_Angeles, // 2, -08:00
  &kZoneAmerica_New_York, // 3, -05:00
  &kZoneAmerica_Toronto, // 4, -05:00
  &kZoneAmerica_Vancouver, // 5, -08:00
  &kZoneAmerica_Edmonton, // 6, -07:00
  &kZoneAmerica_Winnipeg, // 7, -06:00
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

BasicZoneProcessorCache<1> zoneProcessorCache;

BasicZoneManager basicZoneManager(
    kBasicZoneRegistrySize,
    kBasicZoneRegistry,
    zoneProcessorCache
);

//---------------------------------------------------------------------------
// TimeZone + BasicZoneManager
//---------------------------------------------------------------------------

test(BasicZoneManagerTest, registrySize) {
  assertEqual(kBasicZoneRegistrySize, basicZoneManager.zoneRegistrySize());
}

test(BasicZoneManagerTest, createForZoneName) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);
  TimeZone tzn = basicZoneManager.createForZoneName("America/Los_Angeles");
  assertTrue(tz == tzn);
}

test(BasicZoneManagerTest, createForZoneId) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_New_York);
  TimeZone tzid = basicZoneManager.createForZoneId(kZoneIdAmerica_New_York);
  assertTrue(tz == tzid);
  assertEqual((uint32_t) 0x1e2a7654, tz.getZoneId());
  assertEqual((uint32_t) 0x1e2a7654, tzid.getZoneId());
}

test(BasicZoneManagerTest, createForZoneIndex) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Chicago);
  TimeZone tzidx = basicZoneManager.createForZoneIndex(0);
  assertTrue(tz == tzidx);
}

test(BasicZoneManagerTest, indexForZoneName) {
  uint16_t index = basicZoneManager.indexForZoneName("America/Los_Angeles");
  assertEqual((uint16_t) 2, index);

  index = basicZoneManager.indexForZoneName("America/not_found");
  assertEqual(BasicZoneManager::kInvalidIndex, index);
}

test(BasicZoneManagerTest, indexForZoneId) {
  uint16_t index = basicZoneManager.indexForZoneId(kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 3, index);

  index = basicZoneManager.indexForZoneId(0 /* not found */);
  assertEqual(BasicZoneManager::kInvalidIndex, index);
}

test(BasicZoneManagerTest, createForXxx_create_same_timezone) {
  TimeZone a = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);
  TimeZone b = basicZoneManager.createForZoneInfo(&kZoneAmerica_New_York);
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
      &kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  // BasicZoneManager should return the same TimeZone
  TimeZone tzRoundTrip = basicZoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

//---------------------------------------------------------------------------
// ZoneSorter
//---------------------------------------------------------------------------

test(BasicZoneManagerTest, sortIndexes) {
  uint16_t indexes[] = {0, 1, 2, 3, 4, 5, 6, 7};
  ZoneSorterByOffsetAndName<BasicZoneManager> zoneSorter(basicZoneManager);
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
    kZoneIdAmerica_Chicago,
    kZoneIdAmerica_Denver,
    kZoneIdAmerica_Los_Angeles,
    kZoneIdAmerica_New_York,
    kZoneIdAmerica_Toronto,
    kZoneIdAmerica_Vancouver,
    kZoneIdAmerica_Edmonton,
    kZoneIdAmerica_Winnipeg,
  };
  ZoneSorterByOffsetAndName<BasicZoneManager> zoneSorter(basicZoneManager);
  zoneSorter.sortIds(ids, sizeof(ids)/sizeof(ids[0]));
  assertEqual(ids[0], kZoneIdAmerica_Los_Angeles);
  assertEqual(ids[1], kZoneIdAmerica_Vancouver);
  assertEqual(ids[2], kZoneIdAmerica_Denver);
  assertEqual(ids[3], kZoneIdAmerica_Edmonton);
  assertEqual(ids[4], kZoneIdAmerica_Chicago);
  assertEqual(ids[5], kZoneIdAmerica_Winnipeg);
  assertEqual(ids[6], kZoneIdAmerica_New_York);
  assertEqual(ids[7], kZoneIdAmerica_Toronto);
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
  ZoneSorterByOffsetAndName<BasicZoneManager> zoneSorter(basicZoneManager);
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
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
