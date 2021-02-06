#line 2 "BasicBrokerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using ace_time::basic::ZoneInfoBroker;
using ace_time::basic::ZoneEraBroker;
using ace_time::basic::ZoneRuleBroker;
using ace_time::basic::ZonePolicyBroker;
using ace_time::internal::ZoneContext;

test(timeCodeToMinutes) {
  uint8_t code = 1;
  uint8_t modifier = 0x01;
  assertEqual((uint16_t)16,
      ace_time::internal::timeCodeToMinutes(code, modifier));
}

//---------------------------------------------------------------------------

static const char kTzDatabaseVersion[] = "2019b";

static const ZoneContext kZoneContext = {
  2000 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
  0 /*numFragments*/,
  nullptr /*fragments*/,
};

static const char kZoneNameAmerica_Los_Angeles[] ACE_TIME_PROGMEM =
  "America/Los_Angeles";

static const basic::ZoneRule kZoneRulesUS[] ACE_TIME_PROGMEM = {
  // Rule    US    1967    2006    -    Oct    lastSun    2:00    0    S
  {
    -33 /*fromYearTiny*/,
    6 /*toYearTiny*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/,
  },
};

static const basic::ZonePolicy kPolicyUS ACE_TIME_PROGMEM = {
  kZoneRulesUS /*rules*/,
  nullptr /* letters */,
  1 /*numRules*/,
  0 /* numLetters */,
};

static const basic::ZoneEra kZoneEraAmerica_Los_Angeles[] ACE_TIME_PROGMEM = {
  //             -8:00    US    P%sT
  {
    &kPolicyUS /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    0 /*deltaCode*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    2 /*untilTimeCode*/, // 00:31 = 2*15 + 1
    ZoneContext::kSuffixW + 1/*untilTimeModifier*/,
  },
};

const basic::ZoneInfo kZoneAmerica_Los_Angeles ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Los_Angeles /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Los_Angeles /*eras*/,
};

test(BasicBrokerTest, ZoneRuleBroker) {
  ZoneRuleBroker rule(kZoneRulesUS);
  assertFalse(rule.isNull());
  assertEqual(-33, rule.fromYearTiny());
  assertEqual(6, rule.toYearTiny());
  assertEqual(10, rule.inMonth());
  assertEqual(7, rule.onDayOfWeek());
  assertEqual(0, rule.onDayOfMonth());
  assertEqual((uint16_t)120, rule.atTimeMinutes());
  assertEqual(ZoneContext::kSuffixW, rule.atTimeSuffix());
  assertEqual(0, rule.deltaMinutes());
  assertEqual((uint8_t)'S', rule.letter());
}

test(BasicBrokerTest, ZonePolicyBroker) {
  ZonePolicyBroker policy(&kPolicyUS);
  assertFalse(policy.isNull());
  assertEqual(1, policy.numRules());
  assertEqual(0, policy.numLetters());
}

test(BasicBrokerTest, ZoneEraBroker) {
  ZoneEraBroker era(kZoneEraAmerica_Los_Angeles);
  assertFalse(era.isNull());
  assertEqual(-32 * 15, era.offsetMinutes());
  assertEqual(0 * 15, era.deltaMinutes());
  assertEqual("P%T", era.format());
  assertEqual((int8_t)127, era.untilYearTiny());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint16_t)31, era.untilTimeMinutes());
  assertEqual(ZoneContext::kSuffixW, era.untilTimeSuffix());

  ZoneEraBroker era2(kZoneEraAmerica_Los_Angeles);
  assertTrue(era.equals(era2));
}

test(BasicBrokerTest, ZoneInfoBroker) {
  ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
  assertEqual(&kZoneContext, info.zoneContext());
  assertEqual(kZoneNameAmerica_Los_Angeles, info.name());
  assertEqual((uint32_t) 0xb7f7e8f2, info.zoneId());
  assertEqual(2000, info.zoneContext()->startYear);
  assertEqual(2050, info.zoneContext()->untilYear);
  assertEqual(1, info.numEras());
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
