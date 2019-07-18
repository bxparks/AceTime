#line 2 "TimeZoneExtendedTest.ino"

#include <AUnit.h>
#include <aunit/fake/FakePrint.h>
#include <AceTime.h>

using namespace aunit;
using namespace aunit::fake;
using namespace ace_time;

// --------------------------------------------------------------------------
// TimeZone using ExtendedZoneManager
// --------------------------------------------------------------------------

test(TimeZoneTest_Extended, operatorEqualEqual) {
  TimeZone a = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_New_York);

  assertTrue(a != b);
}

test(TimeZoneTest_Extended, copyConstructor) {
  TimeZone a = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Extended, Los_Angeles) {
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeManaged, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PST", fakePrint.getBuffer());
  fakePrint.flush();

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  tz.printAbbrevTo(fakePrint, epochSeconds);
  assertEqual("PDT", fakePrint.getBuffer());
}

// --------------------------------------------------------------------------

ExtendedZoneManager<2> extendedZoneManager(
    zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only

  TimeZone::setZoneManager(&extendedZoneManager);
}

void loop() {
  TestRunner::run();
}
