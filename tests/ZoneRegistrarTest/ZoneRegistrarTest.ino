#line 2 "ZoneRegistrarTest.ino"

/*
 * Test for ZoneRegistrar. It is sufficient to test just Info::ZoneRegistrar
 * since extended::ZoneRegistrar is derived from the exact same
 * ZoneRegistrarTemplate class.
 */

#include <AUnit.h>
#include <AceTime.h>
#include <testingzonedb/zone_policies.h>
#include <testingzonedb/zone_infos.h>
#include <testingzonedb/zone_registry.h>

using namespace ace_time;
using namespace ace_time::basic;
using ace_time::basic::Info;
using ace_time::testingzonedb::kZoneAmerica_Chicago;
using ace_time::testingzonedb::kZoneAmerica_Denver;
using ace_time::testingzonedb::kZoneAmerica_Los_Angeles;
using ace_time::testingzonedb::kZoneAmerica_New_York;
using ace_time::testingzonedb::kZoneIdAmerica_Chicago;
using ace_time::testingzonedb::kZoneIdAmerica_Denver;
using ace_time::testingzonedb::kZoneIdAmerica_Los_Angeles;
using ace_time::testingzonedb::kZoneIdAmerica_New_York;
using ace_time::testingzonedb::kZoneRegistrySize;
using ace_time::testingzonedb::kZoneRegistry;

//---------------------------------------------------------------------------
// Define some registries used later on.
//---------------------------------------------------------------------------

const uint16_t kNumSortedEntries = 4;

const Info::ZoneInfo* const kSortedRegistry[kNumSortedEntries]
    ACE_TIME_PROGMEM = {
  &kZoneAmerica_New_York,     // 0x1e2a7654
  &kZoneAmerica_Chicago,      // 0x4b92b5d4
  &kZoneAmerica_Denver,       // 0x97d10b2a
  &kZoneAmerica_Los_Angeles,  // 0xb7f7e8f2
};

const uint16_t kNumUnsortedEntries = 4;

const Info::ZoneInfo* const kUnsortedRegistry[kNumUnsortedEntries]
    ACE_TIME_PROGMEM = {
  &kZoneAmerica_Chicago,      // 0x4b92b5d4
  &kZoneAmerica_Denver,       // 0x97d10b2a
  &kZoneAmerica_Los_Angeles,  // 0xb7f7e8f2
  &kZoneAmerica_New_York,     // 0x1e2a7654
};

//---------------------------------------------------------------------------

// Verify that we can use kZoneIdAmerica_Los_Angeles everywhere.
test(ZoneRegistrarTest, kZoneId) {
  assertEqual((uint32_t) 0xb7f7e8f2, kZoneIdAmerica_Los_Angeles);
}

//---------------------------------------------------------------------------
// ZoneRegistrar public methods
//---------------------------------------------------------------------------

test(ZoneRegistrarTest, zoneRegistrySize) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
  assertEqual(kZoneRegistrySize, zoneRegistrar.zoneRegistrySize());
}

test(ZoneRegistrarTest, getZoneInfo_Los_Angeles) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);

  const Info::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("America/Los_Angeles");
  assertNotEqual(zoneInfo, nullptr);

  ace_common::PrintStr<32> printStr;
  BasicZone(zoneInfo).printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());

  printStr.flush();
  BasicZone(zoneInfo).printShortNameTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());
}

test(ZoneRegistrarTest, getZoneInfo_not_found) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
  const Info::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName(
      "doesNotExist");
  assertEqual(zoneInfo, nullptr);
}

test(ZoneRegistrarTest, getZoneInfo_Index_not_found) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
  const Info::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(
      kZoneRegistrySize);
  assertEqual(zoneInfo, nullptr);
}

test(ZoneRegistrarTest, getZoneInfoForId) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
  const Info::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      kZoneIdAmerica_Los_Angeles);
  const Info::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertEqual(zoneInfo, zoneInfoExpected);
}

test(ZoneRegistrarTest, getZoneInfoForId_not_found) {
  ZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
  const Info::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x0 /* should not exist */);
  assertEqual(zoneInfo, nullptr);
}

//---------------------------------------------------------------------------
// Tests for findIndexFor*() need a stable registry.
//---------------------------------------------------------------------------

test(ZoneRegistrarTest, findIndexForName) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/Los_Angeles");
  assertEqual((uint16_t) 3, index);
}

test(ZoneRegistrarTest, findIndexForName_not_found) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForName("America/not_found");
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

test(ZoneRegistrarTest, findIndexForId) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(
      kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(ZoneRegistrarTest, findIndexForId_not_found) {
  ZoneRegistrar zoneRegistrar(kNumSortedEntries, kSortedRegistry);
  uint16_t index = zoneRegistrar.findIndexForId(0x0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted().
//---------------------------------------------------------------------------

test(ZoneRegistrarTest_Sorted, isSorted) {
  assertTrue(ZoneRegistrar::isSorted(kSortedRegistry, kNumSortedEntries));
}

test(ZoneRegistrarTest_Unsorted, isSorted) {
  assertFalse(ZoneRegistrar::isSorted(
      kUnsortedRegistry, kNumUnsortedEntries));
}

//---------------------------------------------------------------------------
// ZoneRegistrar::linearSearchById() w/ sorted registry.
//---------------------------------------------------------------------------

test(ZoneRegistrarTest_Sorted, linearSearchById) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(ZoneRegistrarTest_Sorted, linearSearchById_not_found) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kSortedRegistry, kNumSortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// ZoneRegistrar::binarySearchById() w/ sorted registry.
//---------------------------------------------------------------------------

test(ZoneRegistrarTest_Sorted, binarySearchById_zeroEntries) {
  uint16_t index = ZoneRegistrar::binarySearchById(
      kSortedRegistry,
      0 /* kNumSortedEntries */,
      kZoneIdAmerica_Los_Angeles);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

test(ZoneRegistrarTest_Sorted, binarySearchById) {
  uint16_t index;

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, kZoneIdAmerica_New_York);
  assertEqual((uint16_t) 0, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, kZoneIdAmerica_Chicago);
  assertEqual((uint16_t) 1, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, kZoneIdAmerica_Denver);
  assertEqual((uint16_t) 2, index);

  index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 3, index);
}

test(ZoneRegistrarTest_Sorted, binarySearchById_not_found) {
  uint16_t index = ZoneRegistrar::binarySearchById(
      kSortedRegistry, kNumSortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
}

//---------------------------------------------------------------------------
// ZoneRegistrar::linearSearchById() with *unsorted* registry.
//---------------------------------------------------------------------------

test(ZoneRegistrarTest_Unsorted, linearSearchById) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumUnsortedEntries,
      kZoneIdAmerica_Los_Angeles);
  assertEqual((uint16_t) 2, index);
}

test(ZoneRegistrarTest_Unsorted, linearSearchById_not_found) {
  uint16_t index = ZoneRegistrar::linearSearchById(
      kUnsortedRegistry, kNumUnsortedEntries, 0);
  assertEqual(ZoneRegistrar::kInvalidIndex, index);
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
