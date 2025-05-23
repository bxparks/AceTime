#line 2 "ZoneProcessorCacheTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <testingzonedb/zone_infos.h>
#include <testingzonedbx/zone_infos.h>
#include <testingzonedbc/zone_infos.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// BasicZoneProcessorCache
//---------------------------------------------------------------------------

test(BasicZoneProcessorCacheTest, getZoneProcessor) {
  BasicZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedb::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedb::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedb::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedb::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------
// ExtendedZoneProcessorCache
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorCacheTest, getZoneProcessor) {
  ExtendedZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbx::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbx::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbx::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbx::kZoneAmerica_Denver);
  assertEqual(zoneProcessor1, zoneProcessor4);
}

//---------------------------------------------------------------------------
// CompleteZoneProcessorCache
//---------------------------------------------------------------------------

test(CompleteZoneProcessorCacheTest, getZoneProcessor) {
  CompleteZoneProcessorCache<2> cache;

  ZoneProcessor* zoneProcessor1 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbc::kZoneAmerica_Los_Angeles);

  ZoneProcessor* zoneProcessor2 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbc::kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor1, zoneProcessor2);

  ZoneProcessor* zoneProcessor3 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbc::kZoneAmerica_New_York);
  assertNotEqual(zoneProcessor1, zoneProcessor3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneProcessor* zoneProcessor4 = cache.getZoneProcessor(
      (uintptr_t) &testingzonedbc::kZoneAmerica_Denver);
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
