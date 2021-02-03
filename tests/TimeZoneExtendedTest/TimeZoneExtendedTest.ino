#line 2 "TimeZoneExtendedTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------
// ExtendedZoneManager
//---------------------------------------------------------------------------

const extended::ZoneInfo* const kExtendedZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneManager<2> extendedZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

//---------------------------------------------------------------------------

test(TimeZoneExtendedTest, createFor) {
  TimeZone a = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_New_York);
  assertTrue(a != b);

  TimeZone aa = extendedZoneManager.createForZoneName("America/Los_Angeles");
  TimeZone bb = extendedZoneManager.createForZoneId(0x1e2a7654U); // New_York

  assertTrue(a == aa);
  assertTrue(b == bb);
  assertEqual((uint32_t) 0x1e2a7654, bb.getZoneId());
}

test(TimeZoneExtendedTest, Los_Angeles) {
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeExtended, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PST"), tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PDT"), tz.getAbbrev(epochSeconds));
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
