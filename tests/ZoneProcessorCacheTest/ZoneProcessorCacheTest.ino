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

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Denver);
  assertEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor4);
}

// --------------------------------------------------------------------------
// ExtendedZoneProcessorCache
// --------------------------------------------------------------------------

test(ExtendedZoneProcessorCacheTest, getZoneProcessor) {
  ExtendedZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Denver);
  assertEqual((intptr_t) zoneProcessor1, (intptr_t) zoneProcessor4);
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
