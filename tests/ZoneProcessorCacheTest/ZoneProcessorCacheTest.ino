#line 2 "ZoneProcessorCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <zonedbtesting/zone_infos.h>
#include <zonedbxtesting/zone_infos.h>
#include <zonedbctesting/zone_infos.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// BasicZoneProcessorCache
//---------------------------------------------------------------------------

test(BasicZoneProcessorCacheTest, getZoneProcessor) {
  BasicZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &zonedbtesting::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &zonedbtesting::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &zonedbtesting::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &zonedbtesting::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------
// ExtendedZoneProcessorCache
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorCacheTest, getZoneProcessor) {
  ExtendedZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &zonedbxtesting::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &zonedbxtesting::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &zonedbxtesting::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &zonedbxtesting::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------
// CompleteZoneProcessorCache
//---------------------------------------------------------------------------

test(CompleteZoneProcessorCacheTest, getZoneProcessor) {
  CompleteZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &zonedbctesting::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &zonedbctesting::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &zonedbctesting::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &zonedbctesting::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
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
