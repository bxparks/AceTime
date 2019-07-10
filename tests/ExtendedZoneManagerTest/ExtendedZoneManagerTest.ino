#line 2 "ExtendedZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ExtendedZoneManager
// --------------------------------------------------------------------------

test(ExtendedZoneManagerTest, getZoneInfo_Los_Angeles) {
  ExtendedZoneManager zoneManager(
      zonedbx::kZoneRegistry, zonedbx::kZoneRegistrySize);
  assertTrue(zoneManager.isSorted());

  const extended::ZoneInfo* zoneInfo =
      zoneManager.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);

  const char* name = extended::ZoneInfoBroker(zoneInfo).name();
#if ACE_TIME_USE_EXTENDED_PROGMEM
  assertEqual(FPSTR(name), "America/Los_Angeles");
#else
  assertEqual(name, "America/Los_Angeles");
#endif
}

test(ExtendedZoneManagerTest, getZoneInfo_not_found) {
  ExtendedZoneManager zoneManager(
      zonedbx::kZoneRegistry, zonedbx::kZoneRegistrySize);
  const extended::ZoneInfo* zoneInfo = zoneManager.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
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
