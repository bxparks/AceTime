#line 2 "TimeZoneMoreTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>

using namespace aunit;
using ace_common::PrintStr;
using namespace ace_time;

// --------------------------------------------------------------------------
// operator==() for kTypeManual, kTypeBasic and kTypeExtended.
// --------------------------------------------------------------------------

// These ZoneProcessors consume a lot of static RAM, so moved to a separate
// test so that they can be run on a Nano with only 2kB of memory. These
// processors aren't actually used in the unit tests, but are needed just to
// construct the different types of TimeZone instances.
BasicZoneProcessor basicZoneProcessor;
ExtendedZoneProcessor extendedZoneProcessor;

test(TimeZoneMoreTest, operatorEqualEqual_directZone) {
  TimeZone manual = TimeZone::forHours(-8);
  TimeZone manual2 = TimeZone::forHours(-7);
  assertTrue(manual != manual2);

  TimeZone basic = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles, &basicZoneProcessor);
  TimeZone basic2 = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_New_York, &basicZoneProcessor);
  assertTrue(basic != basic2);

  TimeZone extended = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles, &extendedZoneProcessor);
  TimeZone extended2 = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_New_York, &extendedZoneProcessor);
  assertTrue(extended != extended2);

  assertTrue(manual != basic);
  assertTrue(manual != extended);
  assertTrue(basic != extended);
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
