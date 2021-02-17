#line 2 "LinkRegistrarTest.ino"

/*
 * Tests for LinkRegistrar. It is sufficient to test just basic::LinkRegistrar
 * since extended::LinkRegistrar is derived from the exact same
 * LinkRegistrarTemplate class.
 */

#include <AUnit.h>
#include <AceTime.h>

using aunit::TestRunner;
using namespace ace_time;
using ace_time::basic::LinkRegistrar;

test(LinkRegistrarTest, linkRegistrySize) {
  LinkRegistrar linkRegistrar(zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);
  assertEqual(zonedb::kLinkRegistrySize, linkRegistrar.linkRegistrySize());
}

test(LinkRegistrarTest, emptyRegistry_returns_nullptr) {
  LinkRegistrar linkRegistrar(0, nullptr);
  assertEqual(
      nullptr,
      linkRegistrar.getLinkEntryForId(zonedb::kZoneIdUS_Pacific));
}

test(LinkRegistrarTest, isSorted) {
  assertTrue(LinkRegistrar::isSorted(
      zonedb::kLinkRegistry, zonedb::kLinkRegistrySize));
}

test(LinkRegistrarTest, getLinkEntry_US_Pacific) {
  LinkRegistrar linkRegistrar(zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);
  assertNotEqual(
      nullptr,
      linkRegistrar.getLinkEntryForId(zonedb::kZoneIdUS_Pacific));
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
