#line 2 "ExtendedZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ExtendedZoneRegistrar
// --------------------------------------------------------------------------

test(ExtendedZoneRegistrarTest, getZoneInfo_Los_Angeles) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  assertTrue(zoneRegistrar.isSorted());

  const extended::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfo("America/Los_Angeles");
  assertTrue(zoneInfo != nullptr);

  assertEqual("America/Los_Angeles", ExtendedZone(zoneInfo).name());
  assertEqual(F("Los_Angeles"), ExtendedZone(zoneInfo).shortName());
}

test(ExtendedZoneRegistrarTest, getZoneInfo_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfo("not found");
  assertTrue(zoneInfo == nullptr);
}

test(ExtendedZoneRegistrarTest, getZoneInfo_Index_0) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfo((uint16_t)0);
  assertTrue(zoneInfo != nullptr);
  assertEqual(F("Africa/Abidjan"), ExtendedZone(zoneInfo).name());
}

test(ExtendedZoneRegistrarTest, getZoneInfo_Index_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfo(
      zonedbx::kZoneRegistrySize);
  assertTrue(zoneInfo == nullptr);
}

test(ExtendedZoneRegistrarTest, getZoneInfoFromId) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoFromId(
      0xb7f7e8f2);
  const extended::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfo(
      "America/Los_Angeles");
  assertFalse(zoneInfo == nullptr);
  assertTrue((intptr_t) zoneInfo == (intptr_t) zoneInfoExpected);
}

test(ExtendedZoneRegistrarTest, getZoneInfoFromId_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoFromId(
      0x11111111);
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
