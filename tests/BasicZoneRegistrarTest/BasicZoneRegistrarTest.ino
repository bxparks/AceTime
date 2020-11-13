#line 2 "BasicZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/common/compat.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneRegistrar
// --------------------------------------------------------------------------

test(BasicZoneRegistrarTest, getZoneInfo_Los_Angeles) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  assertTrue(zoneRegistrar.isSorted());

  const basic::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("America/Los_Angeles");
  assertNotEqual(zoneInfo, nullptr);

  assertEqual(F("America/Los_Angeles"), BasicZone(zoneInfo).name());
  assertEqual(F("Los_Angeles"), BasicZone(zoneInfo).shortName());
}

test(BasicZoneRegistrarTest, getZoneInfo_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName(
      "not found");
  assertEqual(zoneInfo, nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfo_Index_0) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(0);
  assertNotEqual(zoneInfo, nullptr);
  assertEqual(F("Africa/Abidjan"), BasicZone(zoneInfo).name());
}

test(BasicZoneRegistrarTest, getZoneInfo_Index_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(
      zonedb::kZoneRegistrySize);
  assertEqual(zoneInfo, nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfoForId) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0xb7f7e8f2);
  const basic::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertEqual(zoneInfo, zoneInfoExpected);
}

test(BasicZoneRegistrarTest, getZoneInfoForId_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x11111111);
  assertEqual(zoneInfo, nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted(), binarySearch(), linearSearch() for *sorted*
// registry. Sufficient to test BasicZoneRegistrar only since they are the same
// for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kSortedRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kNumSortedEntries =
    sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*);

test(BasicZoneRegistrarTest_Sorted, isSorted) {
  assertTrue(BasicZoneRegistrar::isSorted(kSortedRegistry, kNumSortedEntries));
}

test(BasicZoneRegistrarTest_Sorted, linearSearch) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::linearSearch(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");

  assertEqual(F("America/Los_Angeles"), BasicZone(zi).name());
}

test(BasicZoneRegistrarTest_Sorted, linearSearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::linearSearch(kSortedRegistry,
      kNumSortedEntries, "America/NotFound");
  assertEqual(zi, nullptr);
}

test(BasicZoneRegistrarTest_Sorted, binarySearch) {
  const basic::ZoneInfo* zi;

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Chicago");
  assertEqual(F("America/Chicago"), BasicZone(zi).name());

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Denver");
  assertEqual(F("America/Denver"), BasicZone(zi).name());

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");
  assertEqual(F("America/Los_Angeles"), BasicZone(zi).name());

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/New_York");
  assertEqual(F("America/New_York"), BasicZone(zi).name());

}

test(BasicZoneRegistrarTest_Sorted, binarySearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::binarySearch(kSortedRegistry,
      sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertEqual(zi, nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted(), binarySearch(), linearSearch() for *unsorted*
// registry. Sufficient to test BasicZoneRegistrar only since they are the same
// for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kUnsortedRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
};

test(BasicZoneRegistrarTest_Unsorted, isSorted) {
  bool isSorted = BasicZoneRegistrar::isSorted(
      kUnsortedRegistry, sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*));
  assertFalse(isSorted);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearch) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::linearSearch(
      kUnsortedRegistry, sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*),
      "America/Los_Angeles");

  assertEqual(F("America/Los_Angeles"), BasicZone(zi).name());

  zi = BasicZoneRegistrar::linearSearch(kUnsortedRegistry,
      sizeof(kUnsortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertEqual(zi, nullptr);
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
