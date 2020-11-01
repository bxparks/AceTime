#line 2 "ExtendedZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ExtendedZoneRegistrar
// --------------------------------------------------------------------------

test(ExtendedZoneRegistrarTest, getZoneInfoForName_Los_Angeles) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  assertTrue(zoneRegistrar.isSorted());

  const extended::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("America/Los_Angeles");
  assertNotEqual(zoneInfo, nullptr);

  assertEqual(F("America/Los_Angeles"), ExtendedZone(zoneInfo).name());
  assertEqual(F("Los_Angeles"), ExtendedZone(zoneInfo).shortName());
}

test(ExtendedZoneRegistrarTest, getZoneInfoForName_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("not found");
  assertEqual(zoneInfo, nullptr);
}

test(ExtendedZoneRegistrarTest, getZoneInfo_Index_0) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(0);
  assertNotEqual(zoneInfo, nullptr);
  assertEqual(F("Africa/Abidjan"), ExtendedZone(zoneInfo).name());
}

test(ExtendedZoneRegistrarTest, getZoneInfo_Index_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(
      zonedbx::kZoneRegistrySize);
  assertEqual(zoneInfo, nullptr);
}

test(ExtendedZoneRegistrarTest, getZoneInfoForId) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0xb7f7e8f2);
  const extended::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertEqual(zoneInfo, zoneInfoExpected);
}

test(ExtendedZoneRegistrarTest, getZoneInfoForId_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x11111111);
  assertEqual(zoneInfo, nullptr);
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
