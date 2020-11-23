#line 2 "BasicZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/common/compat.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// Define some registries used later on.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kSortedRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kNumSortedEntries =
    sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*);

const basic::ZoneInfo* const kUnsortedRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
};

const uint16_t kNumUnsortedEntries =
    sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*);

// --------------------------------------------------------------------------
// BasicZoneRegistrar public methods
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
// Tests for findIndexFor*() need a stable registry.
// --------------------------------------------------------------------------

test(BasicZoneRegistrarTest, findIndexForName) {
  BasicZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/Los_Angeles");
  assertEqual(2, index);
}

test(BasicZoneRegistrarTest, findIndexForName_not_found) {
  BasicZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/not_found");
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest, findIndexForId) {
  BasicZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(
      0xb7f7e8f2 /* America/Los_Angeles */);
  assertEqual(2, index);
}

test(BasicZoneRegistrarTest, findIndexForId_not_found) {
  BasicZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(0x12345678 /* not exist */);
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar protected methods: ZoneRegistrar::isSorted(),
// binarySearchByName(), linearSearchByName() linearSearchyById() for *sorted*
// registry. Sufficient to test BasicZoneRegistrar only since they are the same
// for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Sorted, isSorted) {
  assertTrue(BasicZoneRegistrar::isSorted(kSortedRegistry, kNumSortedEntries));
}

test(BasicZoneRegistrarTest_Sorted, linearSearchByName) {
  uint16_t index = BasicZoneRegistrar::linearSearchByName(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");
  assertEqual(2, index);
}

test(BasicZoneRegistrarTest_Sorted, linearSearchByName_not_found) {
  uint16_t index = BasicZoneRegistrar::linearSearchByName(kSortedRegistry,
      kNumSortedEntries, "America/NotFound");
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest_Sorted, binarySearchByName) {
  uint16_t index;

  index = BasicZoneRegistrar::binarySearchByName(
      kSortedRegistry, kNumSortedEntries, "America/Chicago");
  assertEqual(0, index);

  index = BasicZoneRegistrar::binarySearchByName(
      kSortedRegistry, kNumSortedEntries, "America/Denver");
  assertEqual(1, index);

  index = BasicZoneRegistrar::binarySearchByName(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");
  assertEqual(2, index);

  index = BasicZoneRegistrar::binarySearchByName(
      kSortedRegistry, kNumSortedEntries, "America/New_York");
  assertEqual(3, index);
}

test(BasicZoneRegistrarTest_Sorted, binarySearchByName_not_found) {
  uint16_t index = BasicZoneRegistrar::binarySearchByName(
      kSortedRegistry, kNumSortedEntries, "America/NotFound");
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest_Sorted, linearSearchById) {
  uint16_t index = BasicZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, 0xb7f7e8f2 /* America/Los_Angeles */);
  assertEqual(2, index);
}

test(BasicZoneRegistrarTest_Sorted, linearSearchById_not_found) {
  uint16_t index = BasicZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, 0 /* hopefully no */);
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar protected methods: ZoneRegistrar::isSorted(),
// binarySearchByName(), linearSearchByName(), and linearSearchById() for
// *unsorted* registry. Sufficient to test BasicZoneRegistrar only since they
// are the same for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

test(BasicZoneRegistrarTest_Unsorted, isSorted) {
  bool isSorted = BasicZoneRegistrar::isSorted(
      kUnsortedRegistry, kNumUnsortedEntries);
  assertFalse(isSorted);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearchByName) {
  uint16_t index = BasicZoneRegistrar::linearSearchByName(
      kUnsortedRegistry, kNumUnsortedEntries, "America/Los_Angeles");
  assertEqual(3, index);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearchByName_not_found) {
  uint16_t index = BasicZoneRegistrar::linearSearchByName(
      kUnsortedRegistry, kNumUnsortedEntries, "America/NotFound");
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearchById) {
  uint16_t index = BasicZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumSortedEntries,
      0xb7f7e8f2 /* America/Los_Angeles */);
  assertEqual(3, index);
}

test(BasicZoneRegistrarTest_Unsorted, linearSearchById_not_found) {
  uint16_t index = BasicZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumSortedEntries, 0 /* hopefully no */);
  assertEqual(BasicZoneRegistrar::kInvalidIndex, index);
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
