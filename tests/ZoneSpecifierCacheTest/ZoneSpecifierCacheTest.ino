#line 2 "ZoneSpecifierCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneSpecifierCache
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

const BasicZoneRegistrar kBasicZoneRegistrar(
    kBasicZoneRegistrySize, kBasicZoneRegistry);

test(BasicZoneSpecifierCacheTest, getZoneSpecifier) {
  BasicZoneSpecifierCache<2> cache(kBasicZoneRegistrar);

  ZoneSpecifier* zspec1 = cache.getZoneSpecifier(
      &zonedb::kZoneAmerica_Los_Angeles);

  ZoneSpecifier* zspec2 = cache.getZoneSpecifier(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneSpecifier* zspec3 = cache.getZoneSpecifier(
      &zonedb::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneSpecifier* zspec4 = cache.getZoneSpecifier(
      &zonedb::kZoneAmerica_Denver);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec4);
}

test(BasicZoneSpecifierCacheTest, getZoneSpecifier_not_found) {
  BasicZoneSpecifierCache<2> cache(kBasicZoneRegistrar);
  const uint32_t notFound = 0x24323244;
  ZoneSpecifier* zspec = cache.getZoneSpecifier(notFound);
  assertTrue(zspec == nullptr);
}

// --------------------------------------------------------------------------
// ExtendedZoneSpecifierCache
// --------------------------------------------------------------------------

const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_BASIC_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

const ExtendedZoneRegistrar kExtendedZoneRegistrar(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

test(ExtendedZoneSpecifierCacheTest, getZoneSpecifier) {
  ExtendedZoneSpecifierCache<2> cache(kExtendedZoneRegistrar);

  ZoneSpecifier* zspec1 = cache.getZoneSpecifier(
      &zonedbx::kZoneAmerica_Los_Angeles);

  ZoneSpecifier* zspec2 = cache.getZoneSpecifier(
      &zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneSpecifier* zspec3 = cache.getZoneSpecifier(
      &zonedbx::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneSpecifier* zspec4 = cache.getZoneSpecifier(
      &zonedbx::kZoneAmerica_Denver);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec4);
}

test(ExtendedZoneSpecifierCacheTest, getZoneSpecifier_not_found) {
  ExtendedZoneSpecifierCache<2> cache(kExtendedZoneRegistrar);
  const uint32_t notFound = 0x24323244;
  ZoneSpecifier* zspec = cache.getZoneSpecifier(notFound);
  assertTrue(zspec == nullptr);
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
