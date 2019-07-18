#line 2 "BasicZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/common/flash.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneManager
// --------------------------------------------------------------------------

test(BasicZoneManagerTest, getZoneInfo_Los_Angeles) {
  BasicZoneManager zoneManager(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  assertTrue(zoneManager.isSorted());

  const basic::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);

  assertEqual(F("America/Los_Angeles"), BasicZone(zoneInfo).name());
  assertEqual(F("Los_Angeles"), BasicZone(zoneInfo).shortName());
}

test(BasicZoneManagerTest, getZoneInfo_not_found) {
  BasicZoneManager zoneManager(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneManager.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
}

test(BasicZoneManagerTest, getZoneInfo_Index_0) {
  BasicZoneManager zoneManager(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneManager.getZoneInfo((uint16_t)0);
  assertTrue(zoneInfo != nullptr);
  assertEqual(F("Africa/Abidjan"), BasicZone(zoneInfo).name());
}

test(BasicZoneManagerTest, getZoneInfo_Index_not_found) {
  BasicZoneManager zoneManager(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneManager.getZoneInfo(
      zonedb::kZoneRegistrySize);
  assertTrue(zoneInfo == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneManager::isSorted(), binarySearch(), linearSearch() for *sorted*
// registry. Sufficient to test BasicZoneManager only since they are the same
// for ExtendedZoneManager.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kSortedRegistry[] ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kNumSortedEntries =
    sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*);

test(BasicZoneManagerTest_Sorted, isSorted) {
  assertTrue(BasicZoneManager::isSorted(kSortedRegistry, kNumSortedEntries));
}

test(BasicZoneManagerTest_Sorted, linearSearch) {
  const basic::ZoneInfo* zi = BasicZoneManager::linearSearch(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");

  const char* name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif
}

test(BasicZoneManagerTest_Sorted, linearSearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneManager::linearSearch(kSortedRegistry,
      kNumSortedEntries, "America/NotFound");
  assertTrue(zi == nullptr);
}

test(BasicZoneManagerTest_Sorted, binarySearch) {
  const basic::ZoneInfo* zi;
  const char* name;

  zi = BasicZoneManager::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Chicago");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Chicago");
#else
  assertEqual(name, "America/Chicago");
#endif

  zi = BasicZoneManager::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Denver");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Denver");
#else
  assertEqual(name, "America/Denver");
#endif

  zi = BasicZoneManager::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif

  zi = BasicZoneManager::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/New_York");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/New_York");
#else
  assertEqual(name, "America/New_York");
#endif

}

test(BasicZoneManagerTest_Sorted, binarySearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneManager::binarySearch(kSortedRegistry,
      sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertTrue(zi == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneManager::isSorted(), binarySearch(), linearSearch() for *unsorted*
// registry. Sufficient to test BasicZoneManager only since they are the same
// for ExtendedZoneManager.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kUnsortedRegistry[] ACE_TIME_BASIC_PROGMEM = {
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
  const basic::ZoneInfo* zi = BasicZoneManager::linearSearch(
      kUnsortedRegistry, sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*),
      "America/Los_Angeles");

  const char* name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif

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
