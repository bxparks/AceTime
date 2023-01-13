#line 2 "LinkManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/tzonedb/zone_policies.h>
#include <ace_time/testing/tzonedb/zone_infos.h>
#include <ace_time/testing/tzonedb/zone_registry.h>
#include <ace_time/testing/tzonedbx/zone_policies.h>
#include <ace_time/testing/tzonedbx/zone_infos.h>
#include <ace_time/testing/tzonedbx/zone_registry.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// BasicLinkManager
//---------------------------------------------------------------------------

BasicLinkManager basicLinkManager(
    tzonedb::kLinkRegistrySize,
    tzonedb::kLinkRegistry
);

// Test that US/Pacific maps to America/Los_Angeles.
test(BasicLinkManager, zoneIdForLinkId) {
  uint32_t zoneId = basicLinkManager.zoneIdForLinkId(
      tzonedb::kZoneIdUS_Pacific);
  assertEqual(tzonedb::kZoneIdAmerica_Los_Angeles, zoneId);
}

//---------------------------------------------------------------------------
// ExtendedLinkManager
//---------------------------------------------------------------------------

ExtendedLinkManager extendedLinkManager(
    tzonedbx::kLinkRegistrySize,
    tzonedbx::kLinkRegistry
);

// Test that US/Pacific maps to America/Los_Angeles.
test(ExtendedLinkManager, zoneIdForLinkId) {
  uint32_t zoneId = extendedLinkManager.zoneIdForLinkId(
      tzonedbx::kZoneIdUS_Pacific);
  assertEqual(tzonedbx::kZoneIdAmerica_Los_Angeles, zoneId);
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
  aunit::TestRunner::run();
}
