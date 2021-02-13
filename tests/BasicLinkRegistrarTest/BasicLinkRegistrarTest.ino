#line 2 "BasicLinkRegistrarTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;

test(BasicLinkRegistrarTest, linkRegistrySize) {
  BasicLinkRegistrar linkRegistrar(
      zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);
  assertEqual(zonedb::kLinkRegistrySize, linkRegistrar.linkRegistrySize());
}

test(BasicLinkRegistrarTest, isSorted) {
  assertTrue(BasicLinkRegistrar::isSorted(
      zonedb::kLinkRegistry, zonedb::kLinkRegistrySize));
}

test(BasicLinkRegistrarTest, getLinkEntry_US_Pacific) {
  BasicLinkRegistrar linkRegistrar(
      zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);

  const basic::LinkEntry* linkEntry =
      linkRegistrar.getLinkEntryForId(zonedb::kZoneIdUS_Pacific);
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
