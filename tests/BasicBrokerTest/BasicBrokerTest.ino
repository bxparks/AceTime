#line 2 "BasicBrokerTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include <zonedbtesting/zone_policies.h>
#include <zonedbtesting/zone_infos.h>
#include <zonedbtesting/zone_registry.h>

using namespace ace_time;
using ace_time::basic::ZoneContext;
using ace_time::basic::ZoneContextBroker;
using ace_time::basic::ZoneInfoBroker;
using ace_time::basic::ZoneEraBroker;
using ace_time::basic::ZoneRuleBroker;
using ace_time::basic::ZonePolicyBroker;
using ace_time::zonedbtesting::kZoneContext;
using ace_time::zonedbtesting::kTzDatabaseVersion;
using ace_time::zonedbtesting::kZonePolicyUS;
using ace_time::zonedbtesting::kZoneAmerica_Los_Angeles;
using ace_time::zonedbtesting::kZoneIdUS_Pacific;
using ace_time::zonedbtesting::kZoneIdAmerica_Los_Angeles;

//---------------------------------------------------------------------------

test(BasicBrokerTest, ZoneContextBroker) {
  auto broker = ZoneContextBroker(&kZoneContext);
  assertEqual(kTzDatabaseVersion, broker.tzVersion());
  assertEqual("D", broker.letter(1));
}

test(BasicBrokerTest, ZoneRuleBroker_toYearFromTiny) {
  ZoneRuleBroker rule(&kZoneContext, &kZonePolicyUS.rules[1]);
  assertEqual(-32768, rule.toYearFromTiny(-128, 2100));
  assertEqual(-32767, rule.toYearFromTiny(-127, 2100));
  assertEqual(2100-126, rule.toYearFromTiny(-126, 2100));
  assertEqual(2100, rule.toYearFromTiny(0, 2100));
  assertEqual(2100+125, rule.toYearFromTiny(125, 2100));
  assertEqual(32766, rule.toYearFromTiny(126, 2100));
  // int16 toYear is limited to 32766 (one less than the maximum untilYear),
  // so toYearTiny should never be 127. But if it is, let's peg it to 32766.
  assertEqual(32766, rule.toYearFromTiny(127, 2100));
}

test(BasicBrokerTest, ZoneRuleBroker) {
  ZoneRuleBroker rule(&kZoneContext, &kZonePolicyUS.rules[1]);
  assertFalse(rule.isNull());
  assertEqual(-32767, rule.fromYear());
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

test(BasicBrokerTest, ZonePolicyBroker) {
  ZonePolicyBroker policy(&kZoneContext, &kZonePolicyUS);
  assertFalse(policy.isNull());
  assertEqual(7, policy.numRules());
}

test(BasicBrokerTest, ZoneEraBroker_toUntilYearFromTiny) {
  const basic::ZoneEra* eras = kZoneAmerica_Los_Angeles.eras;
  ZoneEraBroker era(&kZoneContext, &eras[0]);
  assertEqual(-32768, era.toUntilYearFromTiny(-128, 2100));
  assertEqual(-32767, era.toUntilYearFromTiny(-127, 2100));
  assertEqual(2100-126, era.toUntilYearFromTiny(-126, 2100));
  assertEqual(2100, era.toUntilYearFromTiny(0, 2100));
  assertEqual(2100+125, era.toUntilYearFromTiny(125, 2100));
  assertEqual(2100+126, era.toUntilYearFromTiny(126, 2100));
  assertEqual(32767, era.toUntilYearFromTiny(127, 2100));
}

test(BasicBrokerTest, ZoneEraBroker) {
  const basic::ZoneEra* eras = kZoneAmerica_Los_Angeles.eras;
  ZoneEraBroker era(&kZoneContext, &eras[0]);
  assertFalse(era.isNull());
  assertEqual(-8*60*60, era.offsetSeconds());
  assertEqual(0, era.deltaSeconds());
  assertEqual("P%T", era.format());
  assertEqual(ZoneContext::kMaxUntilYear, era.untilYear());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint32_t)0, era.untilTimeSeconds());
  assertEqual(ZoneContext::kSuffixW, era.untilTimeSuffix());

  const basic::ZoneEra* eras2 = kZoneAmerica_Los_Angeles.eras;
  ZoneEraBroker era2(&kZoneContext, &eras2[0]);
  assertTrue(era.equals(era2));
}

test(BasicBrokerTest, ZoneInfoBroker) {
  ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
  assertEqual(&kZoneContext, info.zoneContext().raw());
  assertEqual("America/Los_Angeles", info.name());
  assertEqual((uint32_t) 0xb7f7e8f2, info.zoneId());
  assertEqual(1980, info.zoneContext().startYear());
  assertEqual(2200, info.zoneContext().untilYear());
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
