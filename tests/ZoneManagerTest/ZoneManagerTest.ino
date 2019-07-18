#line 2 "ZoneManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// BasicZoneManager
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

test(BasicZoneManagerTest, getZoneSpecifier_by_zoneInfo) {
  BasicZoneManager<2> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);

  ZoneSpecifier* zspec1 = manager.getZoneSpecifier(
      &zonedb::kZoneAmerica_Los_Angeles);

  ZoneSpecifier* zspec2 = manager.getZoneSpecifier(
      "America/Los_Angeles");
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneSpecifier* zspec3 = manager.getZoneSpecifier(
      &zonedb::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneSpecifier* zspec4 = manager.getZoneSpecifier(
      &zonedb::kZoneAmerica_Denver);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec4);
}

test(BasicZoneManagerTest, getZoneSpecifier_by_id) {
  BasicZoneManager<2> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);
  ZoneSpecifier* zspec = manager.getZoneSpecifier(0xb7f7e8f2);
  assertTrue(zspec != nullptr);
}

test(BasicZoneManagerTest, getZoneSpecifier_by_id_not_found) {
  BasicZoneManager<2> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);
  ZoneSpecifier* zspec = manager.getZoneSpecifier(0x11111111);
  assertTrue(zspec == nullptr);
}

// --------------------------------------------------------------------------
// ExtendedZoneManager
// --------------------------------------------------------------------------

const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_EXTENDED_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

test(ExtendedZoneManagerTest, getZoneSpecifier_by_zoneInfo) {
  ExtendedZoneManager<2> manager(kExtendedZoneRegistrySize,
      kExtendedZoneRegistry);

  ZoneSpecifier* zspec1 = manager.getZoneSpecifier(
      &zonedbx::kZoneAmerica_Los_Angeles);

  ZoneSpecifier* zspec2 = manager.getZoneSpecifier(
      "America/Los_Angeles");
  assertEqual((intptr_t) zspec1, (intptr_t) zspec2);

  ZoneSpecifier* zspec3 = manager.getZoneSpecifier(
      &zonedbx::kZoneAmerica_New_York);
  assertNotEqual((intptr_t) zspec1, (intptr_t) zspec3);

  // The 3rd unique ZoneInfo reuses zpec1
  ZoneSpecifier* zspec4 = manager.getZoneSpecifier(
      &zonedbx::kZoneAmerica_Denver);
  assertEqual((intptr_t) zspec1, (intptr_t) zspec4);
}

test(ExtendedZoneManagerTest, getZoneSpecifier_by_id) {
  ExtendedZoneManager<2> manager(kExtendedZoneRegistrySize,
      kExtendedZoneRegistry);
  ZoneSpecifier* zspec = manager.getZoneSpecifier(0xb7f7e8f2);
  assertTrue(zspec != nullptr);
}

test(ExtendedZoneManagerTest, getZoneSpecifier_by_id_not_found) {
  ExtendedZoneManager<2> manager(kExtendedZoneRegistrySize,
      kExtendedZoneRegistry);
  ZoneSpecifier* zspec = manager.getZoneSpecifier(0x11111111);
  assertTrue(zspec == nullptr);
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
