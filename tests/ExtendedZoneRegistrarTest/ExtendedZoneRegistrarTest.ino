#line 2 "ExtendedZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------

// Verify that we can use kZoneIdAmerica_Los_Angeles everywhere.
test(ExtendedZoneRegistrarTest, kZoneId) {
  assertEqual((uint32_t) 0xb7f7e8f2, zonedbx::kZoneIdAmerica_Los_Angeles);
}

// --------------------------------------------------------------------------
// ExtendedZoneRegistrar
// --------------------------------------------------------------------------

test(ExtendedZoneRegistrarTest, registrySize) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  assertEqual(zonedbx::kZoneRegistrySize, zoneRegistrar.registrySize());
}

test(ExtendedZoneRegistrarTest, getZoneInfoForName_Los_Angeles) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  // FIXME: tzcompiler.py must be updated to sort the kZoneRegistry using
  // zoneId, not zoneName.
  //assertTrue(zoneRegistrar.isSorted());

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
      zoneRegistrar.getZoneInfoForName("doesNotExist");
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
      zonedbx::kZoneIdAmerica_Los_Angeles);
  const extended::ZoneInfo* zoneInfoExpected = zoneRegistrar.getZoneInfoForName(
      "America/Los_Angeles");
  assertEqual(zoneInfo, zoneInfoExpected);
}

test(ExtendedZoneRegistrarTest, getZoneInfoForId_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForId(
      0x0 /* should not exist */);
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
