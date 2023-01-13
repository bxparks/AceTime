#line 2 "LinkRegistrarTest.ino"

/*
 * Tests for LinkRegistrar. It is sufficient to test just basic::LinkRegistrar
 * since extended::LinkRegistrar is derived from the exact same
 * LinkRegistrarTemplate class.
 *
 * We use the testing zone database in testing/tzonedb, since we don't need
 * to test the entire zonedb database.
 */

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/tzonedb/zone_policies.h>
#include <ace_time/testing/tzonedb/zone_infos.h>
#include <ace_time/testing/tzonedb/zone_registry.h>

using namespace ace_time;
using ace_time::basic::LinkRegistrar;
using ace_time::tzonedb::kZoneIdUS_Pacific;
using ace_time::tzonedb::kLinkRegistrySize;
using ace_time::tzonedb::kLinkRegistry;

test(LinkRegistrarTest, linkRegistrySize) {
  LinkRegistrar linkRegistrar(kLinkRegistrySize, kLinkRegistry);
  assertEqual(kLinkRegistrySize, linkRegistrar.linkRegistrySize());
}

test(LinkRegistrarTest, emptyRegistry_returns_nullptr) {
  LinkRegistrar linkRegistrar(0, nullptr);
  assertEqual(nullptr, linkRegistrar.getLinkEntryForId(kZoneIdUS_Pacific));
}

test(LinkRegistrarTest, isSorted) {
  assertTrue(LinkRegistrar::isSorted(kLinkRegistry, kLinkRegistrySize));
}

test(LinkRegistrarTest, getLinkEntry_US_Pacific) {
  LinkRegistrar linkRegistrar(kLinkRegistrySize, kLinkRegistry);
  assertNotEqual(nullptr, linkRegistrar.getLinkEntryForId(kZoneIdUS_Pacific));
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
