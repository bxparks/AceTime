#line 2 "ExtendedLinkRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;

test(ExtendedLinkRegistrarTest, linkRegistrySize) {
  ExtendedLinkRegistrar linkRegistrar(
      zonedbx::kLinkRegistrySize, zonedbx::kLinkRegistry);
  assertEqual(zonedbx::kLinkRegistrySize, linkRegistrar.linkRegistrySize());
}

test(ExtendedLinkRegistrarTest, isSorted) {
  assertTrue(ExtendedLinkRegistrar::isSorted(
      zonedbx::kLinkRegistry, zonedbx::kLinkRegistrySize));
}

test(ExtendedLinkRegistrarTest, getLinkEntry_US_Pacific) {
  ExtendedLinkRegistrar linkRegistrar(
      zonedbx::kLinkRegistrySize, zonedbx::kLinkRegistry);

  const extended::LinkEntry* linkEntry =
      linkRegistrar.getLinkEntryForId(zonedbx::kZoneIdUS_Pacific);
  assertNotEqual(linkEntry, nullptr);
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
