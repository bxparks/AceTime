#line 2 "BasicZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneManager
// --------------------------------------------------------------------------

test(BasicZoneManagerTest, getZoneInfo_Los_Angeles) {
  BasicZoneManager zoneManager(zonedb::kZoneRegistry, zonedb::kZoneRegistrySize);
  assertTrue(zoneManager.isSorted());

  const basic::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);
  assertEqual(zoneInfo->name, "America/Los_Angeles");
}

test(BasicZoneManagerTest, getZoneInfo_not_found) {
  BasicZoneManager zoneManager(zonedb::kZoneRegistry, zonedb::kZoneRegistrySize);
  const basic::ZoneInfo* zoneInfo = zoneManager.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneManager::isSorted(), binarySearch(), linearSearch() for sorted
// registry. Sufficient to test BasicZoneManager only since they are the same
// for ExtendedZoneManager.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kSortedRegistry[] = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

test(BasicZoneManagerTest_Sorted, isSorted) {
  bool isSorted = BasicZoneManager::isSorted(
      kSortedRegistry, sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*));
  assertTrue(isSorted);
}

test(BasicZoneManagerTest_Sorted, linearSearch) {
  const basic::ZoneInfo *zi = BasicZoneManager::linearSearch(
      kSortedRegistry, sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*),
      "America/Los_Angeles");
  assertEqual(zi->name, "America/Los_Angeles");

  zi = BasicZoneManager::linearSearch(kSortedRegistry,
      sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertTrue(zi == nullptr);
}

test(BasicZoneManagerTest_Sorted, binarySearch) {
  const basic::ZoneInfo *zi = BasicZoneManager::binarySearch(
      kSortedRegistry, sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*),
      "America/Los_Angeles");
  assertEqual(zi->name, "America/Los_Angeles");

  zi = BasicZoneManager::binarySearch(kSortedRegistry,
      sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertTrue(zi == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneManager::isSorted(), binarySearch(), linearSearch() for UNsorted
// registry. Sufficient to test BasicZoneManager only since they are the same
// for ExtendedZoneManager.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kUnsortedRegistry[] = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
};

test(BasicZoneManagerTest_Unsorted, isSorted) {
  bool isSorted = BasicZoneManager::isSorted(
      kUnsortedRegistry, sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*));
  assertFalse(isSorted);
}

test(BasicZoneManagerTest_Unsorted, linearSearch) {
  const basic::ZoneInfo *zi = BasicZoneManager::linearSearch(
      kUnsortedRegistry, sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*),
      "America/Los_Angeles");
  assertEqual(zi->name, "America/Los_Angeles");

  zi = BasicZoneManager::linearSearch(kUnsortedRegistry,
      sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertTrue(zi == nullptr);
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
