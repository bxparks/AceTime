#line 2 "ZoneSpecifierCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneSpecifierCache
// --------------------------------------------------------------------------

test(BasicZoneSpecifierCacheTest, getZoneSpecifier) {
  BasicZoneSpecifierCache<2> cache;

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

// --------------------------------------------------------------------------
// ExtendedZoneSpecifierCache
// --------------------------------------------------------------------------

test(ExtendedZoneSpecifierCacheTest, getZoneSpecifier) {
  ExtendedZoneSpecifierCache<2> cache;

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
