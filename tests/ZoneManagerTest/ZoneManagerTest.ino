#line 2 "ZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZoneManager w/ basic::ZoneInfo and zonedb::kZoneRegistry
// --------------------------------------------------------------------------

test(ZoneManagerTest_Basic, getZoneInfo_Los_Angeles) {
  ZoneManager<basic::ZoneInfo> zoneManager(
      zonedb::kZoneRegistry, zonedb::kZoneRegistrySize);
  assertTrue(zoneManager.isSorted());

  const basic::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);
  assertEqual(zoneInfo->name, "America/Los_Angeles");
}

test(ZoneManagerTest_Basic, getZoneInfo_not_found) {
  ZoneManager<basic::ZoneInfo> zoneManager(zonedb::kZoneRegistry,
      sizeof(zonedb::kZoneRegistry)/sizeof(basic::ZoneInfo*));
  const basic::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
}

// --------------------------------------------------------------------------
// ZoneManager w/ extended::ZoneInfo and zonedbx::kZoneRegistry
// --------------------------------------------------------------------------

test(ZoneManagerTest_Extended, getZoneInfo_Los_Angeles) {
  ZoneManager<extended::ZoneInfo> zoneManager(
      zonedbx::kZoneRegistry, zonedbx::kZoneRegistrySize);
  assertTrue(zoneManager.isSorted());

  const extended::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);
  assertEqual(zoneInfo->name, "America/Los_Angeles");
}

test(ZoneManagerTest_Extended, getZoneInfo_not_found) {
  ZoneManager<extended::ZoneInfo> zoneManager(zonedbx::kZoneRegistry,
      sizeof(zonedbx::kZoneRegistry)/sizeof(extended::ZoneInfo*));
  const extended::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
}

// --------------------------------------------------------------------------
// ZoneManager::isSorted(), binarySearch(), linearSearch()
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kRegistry[] = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

test(ZoneManagerTest_Basic, isSorted) {
  bool isSorted = ZoneManager<basic::ZoneInfo>::isSorted(
      kRegistry, sizeof(kRegistry)/sizeof(basic::ZoneInfo*));
  assertTrue(isSorted);
}

test(ZoneManagerTest_Basic, linearSearch) {
  const basic::ZoneInfo *zi = ZoneManager<basic::ZoneInfo>::linearSearch(
      kRegistry, sizeof(kRegistry)/sizeof(basic::ZoneInfo*), "America/Los_Angeles");
  assertEqual(zi->name, "America/Los_Angeles");
}

test(ZoneManagerTest_Basic, binarySearch) {
  const basic::ZoneInfo *zi = ZoneManager<basic::ZoneInfo>::binarySearch(
      kRegistry, sizeof(kRegistry)/sizeof(basic::ZoneInfo*), "America/Los_Angeles");
  assertEqual(zi->name, "America/Los_Angeles");
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
