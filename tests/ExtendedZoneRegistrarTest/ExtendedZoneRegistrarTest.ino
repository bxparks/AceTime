#line 2 "ExtendedZoneRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------

// Verify that we can use kZoneIdAmerica_Los_Angeles everywhere.
test(ExtendedZoneRegistrarTest, kZoneId) {
  assertEqual((uint32_t) 0xb7f7e8f2, zonedbx::kZoneIdAmerica_Los_Angeles);
}

//---------------------------------------------------------------------------
// ExtendedZoneRegistrar
//---------------------------------------------------------------------------

test(ExtendedZoneRegistrarTest, zoneRegistrySize) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  assertEqual(zonedbx::kZoneRegistrySize, zoneRegistrar.zoneRegistrySize());
}

test(ExtendedZoneRegistrarTest, getZoneInfoForName_Los_Angeles) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);

  const extended::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("America/Los_Angeles");
  assertNotEqual(zoneInfo, nullptr);

  ace_common::PrintStr<32> printStr;
  ExtendedZone(zoneInfo).printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.getCstr());

  printStr.flush();
  ExtendedZone(zoneInfo).printShortNameTo(printStr);
  assertEqual(F("Los_Angeles"), printStr.getCstr());
}

// Test a zone without separators, "EST".
test(ExtendedZoneRegistrarTest, getZoneInfoForName_EST) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);

  const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForName("EST");
  assertNotEqual(zoneInfo, nullptr);

  ace_common::PrintStr<32> printStr;
  ExtendedZone(zoneInfo).printNameTo(printStr);
  assertEqual(F("EST"), printStr.getCstr());

  printStr.flush();
  ExtendedZone(zoneInfo).printShortNameTo(printStr);
  assertEqual(F("EST"), printStr.getCstr());
}

test(ExtendedZoneRegistrarTest, getZoneInfoForName_not_found) {
  ExtendedZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  const extended::ZoneInfo* zoneInfo =
      zoneRegistrar.getZoneInfoForName("doesNotExist");
  assertEqual(zoneInfo, nullptr);
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
