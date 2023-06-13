#line 2 "ExtendedBrokerTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include <zonedbxtesting/zone_policies.h>
#include <zonedbxtesting/zone_infos.h>
#include <zonedbxtesting/zone_registry.h>

using namespace ace_time;
using ace_time::extended::ZoneContext;
using ace_time::extended::ZoneContextBroker;
using ace_time::extended::ZoneInfoBroker;
using ace_time::extended::ZoneEraBroker;
using ace_time::extended::ZoneRuleBroker;
using ace_time::extended::ZonePolicyBroker;
using ace_time::extended::ZoneEra;
using ace_time::zonedbxtesting::kZoneContext;
using ace_time::zonedbxtesting::kTzDatabaseVersion;
using ace_time::zonedbxtesting::kZonePolicyUS;
using ace_time::zonedbxtesting::kZoneAmerica_Los_Angeles;
using ace_time::zonedbxtesting::kZonePolicyNamibia;
using ace_time::zonedbxtesting::kZoneIdUS_Pacific;
using ace_time::zonedbxtesting::kZoneIdAmerica_Los_Angeles; 

//---------------------------------------------------------------------------

test(timeCodeToMinutes) {
  uint8_t code = 1;
  uint8_t modifier = 0x01;
  assertEqual((uint16_t)16,
      ace_time::zoneinfomid::timeCodeToMinutes(code, modifier));
}

//---------------------------------------------------------------------------

test(BasicBrokerTest, ZoneContextBroker) {
  auto broker = ZoneContextBroker(&kZoneContext);
  assertEqual(kTzDatabaseVersion, broker.tzVersion());
  assertEqual("CAT", broker.letter(1));
}

test(ExtendedBrokerTest, ZoneRuleBroker) {
  ZoneRuleBroker rule(&kZoneContext, &kZonePolicyUS.rules[1]);
  assertFalse(rule.isNull());
  assertEqual(1967, rule.fromYear());
  assertEqual(2006, rule.toYear());
  assertEqual(10, rule.inMonth());
  assertEqual(7, rule.onDayOfWeek());
  assertEqual(0, rule.onDayOfMonth());
  assertEqual((uint32_t)2*60*60, rule.atTimeSeconds());
  assertEqual(ZoneContext::kSuffixW, rule.atTimeSuffix());
  assertEqual(0, rule.deltaSeconds());
  char letter[internal::kAbbrevSize];
  rule.letter(letter);
  assertEqual("S", letter);
}

test(ExtendedBrokerTest, ZonePolicyBroker) {
  ZonePolicyBroker policy(&kZoneContext, &kZonePolicyUS);
  assertFalse(policy.isNull());
  assertEqual(7, policy.numRules());
}

test(ExtendedBrokerTest, ZonePolicyBroker_with_letters) {
  ZonePolicyBroker policy(&kZoneContext, &kZonePolicyNamibia);
  assertFalse(policy.isNull());
  assertEqual(4, policy.numRules());
}

test(ExtendedBrokerTest, ZoneEraBroker) {
  ZoneEraBroker era(&kZoneContext, &kZoneAmerica_Los_Angeles.eras[0]);
  assertFalse(era.isNull());
  assertEqual(-8*60*60, era.offsetSeconds());
  assertEqual(0, era.deltaSeconds());
  assertEqual("P%T", era.format());
  assertEqual(ZoneContext::kMaxUntilYear, era.untilYear());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint32_t)0, era.untilTimeSeconds());
  assertEqual(ZoneContext::kSuffixW, era.untilTimeSuffix());

  ZoneEraBroker era2(&kZoneContext, &kZoneAmerica_Los_Angeles.eras[0]);
  assertTrue(era.equals(era2));
}

test(ExtendedBrokerTest, ZoneInfoBroker) {
  ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
  assertEqual(&kZoneContext, info.zoneContext().raw());
  assertEqual("America/Los_Angeles", info.name());
  assertEqual((uint32_t) 0xb7f7e8f2, info.zoneId());
  assertEqual(1980, info.zoneContext().startYear());
  assertEqual(10000, info.zoneContext().untilYear());
  assertEqual(1, info.numEras());
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
