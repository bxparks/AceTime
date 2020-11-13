#line 2 "ExtendedBrokerTest.ino"

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

static const extended::ZoneContext kZoneContext = {
  2000 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
};

static const extended::ZoneRule kZoneRulesUS[] ACE_TIME_PROGMEM = {
  {
    7 /*fromYearTiny*/,
    126 /*toYearTiny*/,
    11 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    extended::ZoneContext::kSuffixW /*atTimeModifier*/,
    (0 + 4) /*deltaCode*/,
    'S' /*letter*/,
  },
};

static const extended::ZonePolicy kPolicyUS ACE_TIME_PROGMEM = {
  kZoneRulesUS /*rules*/,
  nullptr /* letters */,
  1 /*numRules*/,
  0 /* numLetters */,
};


static const extended::ZoneEra kZoneEraAmerica_Los_Angeles[] ACE_TIME_PROGMEM = {
  {
    // offset = -07:55, delta = 02:00
    // until = 01:09
    &kPolicyUS /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    (5 << 4) + (8 + 4) /*deltaCode*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    4 /*untilTimeCode*/,
    extended::ZoneContext::kSuffixW + 9 /*untilTimeModifier*/,
  },
};

static const char kZoneNameAmerica_Los_Angeles[] ACE_TIME_PROGMEM = "America/Los_Angeles";

const extended::ZoneInfo kZoneAmerica_Los_Angeles ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Los_Angeles /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  6 /*transitionBufSize*/,
  1 /*numEras*/,
  kZoneEraAmerica_Los_Angeles /*eras*/,
};

test(ExtendedBrokerTest, ZoneRuleBroker) {
  extended::ZoneRuleBroker rule(kZoneRulesUS);
  assertFalse(rule.isNull());
  assertEqual(7, rule.fromYearTiny());
  assertEqual(126, rule.toYearTiny());
  assertEqual(11, rule.inMonth());
  assertEqual(7, rule.onDayOfWeek());
  assertEqual(1, rule.onDayOfMonth());
  assertEqual((uint16_t)120, rule.atTimeMinutes());
  assertEqual(extended::ZoneContext::kSuffixW, rule.atTimeSuffix());
  assertEqual(0, rule.deltaMinutes());
  assertEqual((uint8_t)'S', rule.letter());
}

test(ExtendedBrokerTest, ZonePolicyBroker) {
  extended::ZonePolicyBroker policy(&kPolicyUS);
  assertFalse(policy.isNull());
  assertEqual(1, policy.numRules());
  assertEqual(0, policy.numLetters());
}

test(ExtendedBrokerTest, ZoneEraBroker) {
  extended::ZoneEraBroker era(kZoneEraAmerica_Los_Angeles);
  assertEqual(kZoneEraAmerica_Los_Angeles, era.zoneEra());
  assertFalse(era.isNull());
  assertEqual(-32 * 15 + 5, era.offsetMinutes());
  assertEqual(120, era.deltaMinutes());
  assertEqual("P%T", era.format());
  assertEqual((int8_t)127, era.untilYearTiny());
  assertEqual((uint8_t)1, era.untilMonth());
  assertEqual((uint8_t)1, era.untilDay());
  assertEqual((uint16_t)69, era.untilTimeMinutes());
  assertEqual(extended::ZoneContext::kSuffixW, era.untilTimeSuffix());
}

test(ExtendedBrokerTest, ZoneInfoBroker) {
  extended::ZoneInfoBroker info(&kZoneAmerica_Los_Angeles);
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
