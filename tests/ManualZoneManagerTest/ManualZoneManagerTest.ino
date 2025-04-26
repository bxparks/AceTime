#line 2 "ManualZoneManagerTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include <testingzonedb/zone_policies.h>
#include <testingzonedb/zone_infos.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::testingzonedb::kZoneAmerica_Los_Angeles;
using ace_time::testingzonedb::kZoneIdAmerica_Los_Angeles;

//---------------------------------------------------------------------------
// ManualZoneManager
//---------------------------------------------------------------------------

ManualZoneManager zoneManager;

//---------------------------------------------------------------------------

test(ManualZoneManagerTest, registrySize) {
  assertEqual((uint16_t) 0, zoneManager.zoneRegistrySize());
}

test(ManualZoneManagerTest, createForTimeZoneData_error) {
  TimeZone tz = TimeZone::forError();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = zoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ManualZoneManagerTest, createForTimeZoneData_utc) {
  TimeZone tz = TimeZone::forUtc();
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{0, 0};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = zoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ManualZoneManagerTest, createForTimeZoneData_manual) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{-8 * 60, 1 * 60};
  assertTrue(expected == tzd);

  TimeZone tzRoundTrip = zoneManager.createForTimeZoneData(tzd);
  assertTrue(tz == tzRoundTrip);
}

test(ManualZoneManagerTest, createForTimeZoneData_zoneId) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  TimeZoneData tzd = tz.toTimeZoneData();

  TimeZoneData expected{kZoneIdAmerica_Los_Angeles};
  assertTrue(expected == tzd);

  // ManualZoneManager cannot handle TimeZone created with ZoneId
  TimeZone tzRoundTrip = zoneManager.createForTimeZoneData(tzd);
  assertTrue(tzRoundTrip.isError());
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
