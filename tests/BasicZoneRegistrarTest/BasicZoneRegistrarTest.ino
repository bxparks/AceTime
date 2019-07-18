#line 2 "BasicZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/common/flash.h>

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
  assertTrue(zoneInfo != nullptr);

  assertEqual(F("America/Los_Angeles"), BasicZone(zoneInfo).name());
  assertEqual(F("Los_Angeles"), BasicZone(zoneInfo).shortName());
}

test(BasicZoneRegistrarTest, getZoneInfo_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName(
      "not found");
  assertTrue(zoneInfo == nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfo_Index_0) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfo((uint16_t)0);
  assertTrue(zoneInfo != nullptr);
  assertEqual(F("Africa/Abidjan"), BasicZone(zoneInfo).name());
}

test(BasicZoneRegistrarTest, getZoneInfo_Index_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfo(
      zonedb::kZoneRegistrySize);
  assertTrue(zoneInfo == nullptr);
}

test(BasicZoneRegistrarTest, getZoneInfoForId) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0xb7f7e8f2);
  const basic::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertFalse(zoneInfo == nullptr);
  assertTrue((intptr_t) zoneInfo == (intptr_t) zoneInfoExpected);
}

test(BasicZoneRegistrarTest, getZoneInfoForId_not_found) {
  BasicZoneRegistrar zoneRegistrar(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  const basic::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x11111111);
  assertTrue(zoneInfo == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted(), binarySearch(), linearSearch() for *sorted*
// registry. Sufficient to test BasicZoneRegistrar only since they are the same
// for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kSortedRegistry[] ACE_TIME_BASIC_PROGMEM = {
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

  const char* name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif
}

test(BasicZoneRegistrarTest_Sorted, linearSearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::linearSearch(kSortedRegistry,
      kNumSortedEntries, "America/NotFound");
  assertTrue(zi == nullptr);
}

test(BasicZoneRegistrarTest_Sorted, binarySearch) {
  const basic::ZoneInfo* zi;
  const char* name;

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Chicago");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Chicago");
#else
  assertEqual(name, "America/Chicago");
#endif

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Denver");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Denver");
#else
  assertEqual(name, "America/Denver");
#endif

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/Los_Angeles");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif

  zi = BasicZoneRegistrar::binarySearch(
      kSortedRegistry, kNumSortedEntries, "America/New_York");
  name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/New_York");
#else
  assertEqual(name, "America/New_York");
#endif

}

test(BasicZoneRegistrarTest_Sorted, binarySearch_not_found) {
  const basic::ZoneInfo* zi = BasicZoneRegistrar::binarySearch(kSortedRegistry,
      sizeof(kSortedRegistry)/sizeof(basic::ZoneInfo*), "America/NotFound");
  assertTrue(zi == nullptr);
}

// --------------------------------------------------------------------------
// Test ZoneRegistrar::isSorted(), binarySearch(), linearSearch() for *unsorted*
// registry. Sufficient to test BasicZoneRegistrar only since they are the same
// for ExtendedZoneRegistrar.
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kUnsortedRegistry[] ACE_TIME_BASIC_PROGMEM = {
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

  const char* name = basic::ZoneInfoBroker(zi).name();
#if ACE_TIME_USE_BASIC_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif

  zi = BasicZoneRegistrar::linearSearch(kUnsortedRegistry,
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
