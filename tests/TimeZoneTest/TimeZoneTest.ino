#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ZoneManager
// --------------------------------------------------------------------------

test(ZoneManagerTest, init_2001) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  manager.mYear = 1;
  manager.mNumMatches = 0;

  manager.addLastYear();
  assertEqual(0, manager.mNumMatches);
  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual(0, manager.mPreviousMatch.rule->fromYear);
  assertEqual(6, manager.mPreviousMatch.rule->toYear);
  assertEqual(10, manager.mPreviousMatch.rule->inMonth);

  manager.addCurrentYear();
  assertEqual(2, manager.mNumMatches);

  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual(0, manager.mMatches[0].rule->fromYear);
  assertEqual(6, manager.mMatches[0].rule->toYear);
  assertEqual(4, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual(0, manager.mMatches[1].rule->fromYear);
  assertEqual(6, manager.mMatches[1].rule->toYear);
  assertEqual(10, manager.mMatches[1].rule->inMonth);
  assertEqual(-32, manager.mMatches[1].entry->offsetCode);

  manager.calcTransitions();
  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 39434400, manager.mMatches[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 57574800, manager.mMatches[1].startEpochSeconds);
}

test(ZoneManagerTest, init_2018) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  LocalDate ld = LocalDate::forComponents(18, 1, 1);
  manager.init(ld);

  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.entry->format);
  assertEqual(7, manager.mPreviousMatch.rule->fromYear);
  assertEqual(255, manager.mPreviousMatch.rule->toYear);
  assertEqual(11, manager.mPreviousMatch.rule->inMonth);

  assertEqual("P%T", manager.mMatches[0].entry->format);
  assertEqual(7, manager.mMatches[0].rule->fromYear);
  assertEqual(255, manager.mMatches[0].rule->toYear);
  assertEqual(3, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%T", manager.mMatches[1].entry->format);
  assertEqual(7, manager.mMatches[1].rule->fromYear);
  assertEqual(255, manager.mMatches[1].rule->toYear);
  assertEqual(11, manager.mMatches[1].rule->inMonth);
  assertEqual(-32, manager.mMatches[1].entry->offsetCode);

  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 574077600, manager.mMatches[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 594637200, manager.mMatches[1].startEpochSeconds);
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ZoneManagerTest, getZoneOffset_Los_Angeles) {
  ZoneManager manager(&ZoneInfo::kLosAngeles);
  OffsetDateTime dt;
  const ZoneMatch* match;

  dt = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59,
      ZoneOffset::forHour(-8));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(-32, match->offsetCode);
  assertEqual("PST", match->abbrev);

  dt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 0,
      ZoneOffset::forHour(-8));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(-28, match->offsetCode);
  assertEqual("PDT", match->abbrev);

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 0, 0,
      ZoneOffset::forHour(-7));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(-28, match->offsetCode);
  assertEqual("PDT", match->abbrev);

  dt = OffsetDateTime::forComponents(18, 11, 4, 1, 59, 59,
      ZoneOffset::forHour(-7));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(-28, match->offsetCode);
  assertEqual("PDT", match->abbrev);

  dt = OffsetDateTime::forComponents(18, 11, 4, 2, 0, 0,
      ZoneOffset::forHour(-7));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(-32, match->offsetCode);
  assertEqual("PST", match->abbrev);
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(ZoneManagerTest, getZoneOffset_Sydney) {
  ZoneManager manager(&ZoneInfo::kSydney);
  OffsetDateTime dt;
  const ZoneMatch* match;

  dt = OffsetDateTime::forComponents(7, 3, 25, 2, 59, 59,
      ZoneOffset::forHour(11));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(44, match->offsetCode);
  assertEqual("AEDT", match->abbrev);

  dt = OffsetDateTime::forComponents(7, 3, 25, 3, 0, 0,
      ZoneOffset::forHour(11));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(40, match->offsetCode);
  assertEqual("AEST", match->abbrev);

  dt = OffsetDateTime::forComponents(7, 10, 28, 1, 59, 59,
      ZoneOffset::forHour(10));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(40, match->offsetCode);
  assertEqual("AEST", match->abbrev);

  dt = OffsetDateTime::forComponents(7, 10, 28, 2, 0, 0,
      ZoneOffset::forHour(10));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(44, match->offsetCode);
  assertEqual("AEDT", match->abbrev);
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(ZoneManagerTest, getZoneOffset_Johannesburg) {
  ZoneManager manager(&ZoneInfo::kJohannesburg);
  OffsetDateTime dt;
  const ZoneMatch* match;

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      ZoneOffset::forHour(2));
  match = manager.getZoneMatch(dt.toEpochSeconds());
  assertEqual(8, match->offsetCode);
  assertEqual("SAST", match->abbrev);
}

test(ZoneManagerTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  ZoneManager::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
