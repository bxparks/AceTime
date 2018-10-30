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
  assertEqual("P%sT", manager.mPreviousMatch.entry->format);
  assertEqual(0, manager.mPreviousMatch.rule->fromYear);
  assertEqual(6, manager.mPreviousMatch.rule->toYear);
  assertEqual(10, manager.mPreviousMatch.rule->inMonth);

  manager.addCurrentYear();
  assertEqual(2, manager.mNumMatches);

  assertEqual("P%sT", manager.mMatches[0].entry->format);
  assertEqual(0, manager.mMatches[0].rule->fromYear);
  assertEqual(6, manager.mMatches[0].rule->toYear);
  assertEqual(4, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%sT", manager.mMatches[1].entry->format);
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
  manager.init(18);

  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mPreviousMatch.entry->offsetCode);
  assertEqual("P%sT", manager.mPreviousMatch.entry->format);
  assertEqual(7, manager.mPreviousMatch.rule->fromYear);
  assertEqual(255, manager.mPreviousMatch.rule->toYear);
  assertEqual(11, manager.mPreviousMatch.rule->inMonth);

  assertEqual("P%sT", manager.mMatches[0].entry->format);
  assertEqual(7, manager.mMatches[0].rule->fromYear);
  assertEqual(255, manager.mMatches[0].rule->toYear);
  assertEqual(3, manager.mMatches[0].rule->inMonth);
  assertEqual(-32, manager.mMatches[0].entry->offsetCode);

  assertEqual("P%sT", manager.mMatches[1].entry->format);
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

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
