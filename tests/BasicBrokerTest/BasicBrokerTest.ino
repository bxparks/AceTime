#line 2 "BasicBrokerTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/tzonedb/zone_policies.h>
#include <ace_time/testing/tzonedb/zone_infos.h>
#include <ace_time/testing/tzonedb/zone_registry.h>

using namespace ace_time;
using ace_time::internal::ZoneContext;
using ace_time::basic::ZoneInfoBroker;
using ace_time::basic::ZoneEraBroker;
using ace_time::basic::ZoneRuleBroker;
using ace_time::basic::ZonePolicyBroker;
using ace_time::basic::LinkEntryBroker;
using ace_time::basic::LinkRegistryBroker;
using ace_time::tzonedb::kZoneContext;
using ace_time::tzonedb::kZonePolicyUS;
using ace_time::tzonedb::kZoneAmerica_Los_Angeles;
using ace_time::tzonedb::kLinkRegistry;
using ace_time::tzonedb::kZoneIdUS_Pacific;
using ace_time::tzonedb::kZoneIdAmerica_Los_Angeles;

//---------------------------------------------------------------------------

test(BasicBrokerTest, ZoneRuleBroker) {
  ZoneRuleBroker rule(&kZonePolicyUS.rules[0]);
  assertFalse(rule.isNull());
  assertEqual(1967, rule.fromYear());
  assertEqual(2006, rule.toYear());
  assertEqual(10, rule.inMonth());
  assertEqual(7, rule.onDayOfWeek());
  assertEqual(0, rule.onDayOfMonth());
  assertEqual((uint16_t)120, rule.atTimeMinutes());
  assertEqual(ZoneContext::kSuffixW, rule.atTimeSuffix());
  assertEqual(0, rule.deltaMinutes());
  assertEqual((uint8_t)'S', rule.letter());
}

test(BasicBrokerTest, ZonePolicyBroker) {
  ZonePolicyBroker policy(&kZonePolicyUS);
  assertFalse(policy.isNull());
  assertEqual(6, policy.numRules());
  assertEqual(0, policy.numLetters());
}

test(BasicBrokerTest, ZoneEraBroker) {
  const basic::ZoneEra* eras =
      (const basic::ZoneEra*) kZoneAmerica_Los_Angeles.eras;
  ZoneEraBroker era(&eras[0]);
  assertFalse(era.isNull());
  assertEqual(-32 * 15, era.offsetMinutes());
  assertEqual(0 * 15, era.deltaMinutes());
  assertEqual("P%T", era.format());
  assertEqual((int16_t)10000, era.untilYear());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint16_t)0, era.untilTimeMinutes());
  assertEqual(ZoneContext::kSuffixW, era.untilTimeSuffix());

  const basic::ZoneEra* eras2 =
      (const basic::ZoneEra*) kZoneAmerica_Los_Angeles.eras;
  ZoneEraBroker era2(&eras2[0]);
  assertTrue(era.equals(era2));
}

test(BasicBrokerTest, ZoneInfoBroker) {
  ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
  assertEqual(&kZoneContext, info.zoneContext());
  assertEqual("America/Los_Angeles", info.name());
  assertEqual((uint32_t) 0xb7f7e8f2, info.zoneId());
  assertEqual(1980, info.zoneContext()->startYear);
  assertEqual(10000, info.zoneContext()->untilYear);
  assertEqual(1, info.numEras());
}

//---------------------------------------------------------------------------

test(BasicBrokerTest, LinkRegistry_LinkEntryBroker) {
  LinkRegistryBroker linkRegistryBroker(kLinkRegistry);
  LinkEntryBroker linkEntryBroker(linkRegistryBroker.linkEntry(0));
  assertEqual(kZoneIdUS_Pacific, linkEntryBroker.linkId());
  assertEqual(kZoneIdAmerica_Los_Angeles, linkEntryBroker.zoneId());
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
