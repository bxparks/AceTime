#line 2 "BasicBrokerTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

test(timeCodeToMinutes) {
  uint8_t code = 1;
  uint8_t modifier = 0x01;
  assertEqual((uint16_t)16,
      ace_time::internal::timeCodeToMinutes(code, modifier));
}

// --------------------------------------------------------------------------

static const char kTzDatabaseVersion[] = "2019b";

static const basic::ZoneContext kZoneContext = {
  2000 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
};

static const char kZoneNameAmerica_Los_Angeles[] ACE_TIME_PROGMEM = "America/Los_Angeles";

static const basic::ZoneRule kZoneRulesUS[] ACE_TIME_PROGMEM = {
  // Rule    US    1967    2006    -    Oct    lastSun    2:00    0    S
  {
    -33 /*fromYearTiny*/,
    6 /*toYearTiny*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    basic::ZoneContext::kSuffixW /*atTimeModifier*/,
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
    basic::ZoneContext::kSuffixW + 1/*untilTimeModifier*/,
  },
};

const basic::ZoneInfo kZoneAmerica_Los_Angeles ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Los_Angeles /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  6 /*transitionBufSize*/,
  1 /*numEras*/,
  kZoneEraAmerica_Los_Angeles /*eras*/,
};

test(BasicBrokerTest, ZoneRuleBroker) {
  basic::ZoneRuleBroker rule(kZoneRulesUS);
  assertFalse(rule.isNull());
  assertEqual(-33, rule.fromYearTiny());
  assertEqual(6, rule.toYearTiny());
  assertEqual(10, rule.inMonth());
  assertEqual(7, rule.onDayOfWeek());
  assertEqual(0, rule.onDayOfMonth());
  assertEqual((uint16_t)120, rule.atTimeMinutes());
  assertEqual(basic::ZoneContext::kSuffixW, rule.atTimeSuffix());
  assertEqual(0, rule.deltaMinutes());
  assertEqual((uint8_t)'S', rule.letter());
}

test(BasicBrokerTest, ZonePolicyBroker) {
  basic::ZonePolicyBroker policy(&kPolicyUS);
  assertFalse(policy.isNull());
  assertEqual(1, policy.numRules());
  assertEqual(0, policy.numLetters());
}

test(BasicBrokerTest, ZoneEraBroker) {
  basic::ZoneEraBroker era(kZoneEraAmerica_Los_Angeles);
  assertEqual((intptr_t)kZoneEraAmerica_Los_Angeles, (intptr_t)era.zoneEra());
  assertFalse(era.isNull());
  assertEqual(-32 * 15, era.offsetMinutes());
  assertEqual(0 * 15, era.deltaMinutes());
  assertEqual("P%T", era.format());
  assertEqual((int8_t)127, era.untilYearTiny());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint16_t)31, era.untilTimeMinutes());
  assertEqual(basic::ZoneContext::kSuffixW, era.untilTimeSuffix());
}

test(BasicBrokerTest, ZoneInfoBroker) {
  basic::ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
  assertEqual(kZoneNameAmerica_Los_Angeles, info.name());
  assertEqual((uint32_t) 0xb7f7e8f2, info.zoneId());
  assertEqual(2000, info.startYear());
  assertEqual(2050, info.untilYear());
  assertEqual(1, info.numEras());
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
