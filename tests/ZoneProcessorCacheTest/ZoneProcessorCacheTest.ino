#line 2 "ZoneProcessorCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------
// BasicZoneProcessorCache
//---------------------------------------------------------------------------

test(BasicZoneProcessorCacheTest, getZoneProcessor) {
  BasicZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      &zonedb::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------
// ExtendedZoneProcessorCache
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorCacheTest, getZoneProcessor) {
  ExtendedZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      &zonedbx::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
