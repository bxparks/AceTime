#line 2 "LinkManagerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// BasicLinkManager
//---------------------------------------------------------------------------

BasicLinkManager basicLinkManager(
    zonedb::kLinkRegistrySize,
    zonedb::kLinkRegistry
);

// Test that US/Pacific maps to America/Los_Angeles.
test(BasicLinkManager, zoneIdForLinkId) {
  uint32_t zoneId = basicLinkManager.zoneIdForLinkId(zonedb::kZoneIdUS_Pacific);
  assertEqual(zonedb::kZoneIdAmerica_Los_Angeles, zoneId);
}

//---------------------------------------------------------------------------
// ExtendedLinkManager
//---------------------------------------------------------------------------

ExtendedLinkManager extendedLinkManager(
    zonedbx::kLinkRegistrySize,
    zonedbx::kLinkRegistry
);

// Test that US/Pacific maps to America/Los_Angeles.
test(ExtendedLinkManager, zoneIdForLinkId) {
  uint32_t zoneId = extendedLinkManager.zoneIdForLinkId(
      zonedbx::kZoneIdUS_Pacific);
  assertEqual(zonedbx::kZoneIdAmerica_Los_Angeles, zoneId);
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
