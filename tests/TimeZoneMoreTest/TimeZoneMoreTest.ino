#line 2 "TimeZoneMoreTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// operator==() for kTypeBasic and kTypeExtended. This test takes up a lot of
// static RAM, so extracted from TimeZoneTest into a separate test.
// --------------------------------------------------------------------------

BasicZoneSpecifier basicZoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
BasicZoneSpecifier basicZoneSpecifier2(&zonedb::kZoneAmerica_New_York);
ExtendedZoneSpecifier extendedZoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
ExtendedZoneSpecifier extendedZoneSpecifier2(&zonedbx::kZoneAmerica_New_York);

test(TimeZoneMoreTest, operatorEqualEqual) {
  TimeZone manual = TimeZone::forTimeOffset(TimeOffset::forHour(-8));
  TimeZone manual2 = TimeZone::forTimeOffset(TimeOffset::forHour(-7));
  assertTrue(manual != manual2);

  TimeZone basic = TimeZone::forZoneSpecifier(&basicZoneSpecifier);
  TimeZone basic2 = TimeZone::forZoneSpecifier(&basicZoneSpecifier2);
  assertTrue(basic != basic2);

  TimeZone extended = TimeZone::forZoneSpecifier(&extendedZoneSpecifier);
  TimeZone extended2 = TimeZone::forZoneSpecifier(&extendedZoneSpecifier2);
  assertTrue(extended != extended2);

  assertTrue(manual != basic);
  assertTrue(manual != extended);
  assertTrue(basic != extended);
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
