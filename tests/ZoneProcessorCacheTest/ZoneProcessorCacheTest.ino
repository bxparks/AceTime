#line 2 "ZoneProcessorCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneProcessorCache
// --------------------------------------------------------------------------

test(BasicZoneProcessorCacheTest, getZoneProcessor) {
  BasicZoneProcessorCache<2> cache;

  ZoneProcessor* zspec1 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zspec2 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneProcessor* zspec3 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zspec4 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Denver);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec4);
}

// --------------------------------------------------------------------------
// ExtendedZoneProcessorCache
// --------------------------------------------------------------------------

test(ExtendedZoneProcessorCacheTest, getZoneProcessor) {
  ExtendedZoneProcessorCache<2> cache;

  ZoneProcessor* zspec1 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zspec2 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneProcessor* zspec3 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zspec4 = cache.getZoneProcessor(
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
