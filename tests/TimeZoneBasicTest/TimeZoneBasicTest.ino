#line 2 "TimeZoneBasicTest.ino"

#include <AUnit.h>
#include <aunit/fake/FakePrint.h>
#include <AceTime.h>

using namespace aunit;
using namespace aunit::fake;
using namespace ace_time;

// --------------------------------------------------------------------------
// TimeZone using BasicZoneManager
// --------------------------------------------------------------------------

test(TimeZoneBasicTest_Basic, operatorEqualEqual) {
  TimeZone a = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_New_York);

  assertTrue(a != b);
}

test(TimeZoneBasicTest_Basic, copyConstructor) {
  TimeZone a = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneBasicTest_Basic, Los_Angeles) {
  FakePrint fakePrint;
  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  assertEqual(TimeZone::kTypeBasic, tz.getType());

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
// TimeZoneData
// --------------------------------------------------------------------------

test(TimeZoneBasicTest, forTimeZoneData) {
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto tzd = tz.toTimeZoneData();
  assertEqual(TimeZone::kTypeBasic, tzd.type);
  assertEqual((intptr_t) &zonedb::kZoneAmerica_Los_Angeles,
      (intptr_t) tzd.zoneInfo);

  auto tzCircle = TimeZone::forTimeZoneData(tzd);
  assertTrue(tz == tzCircle);
}

// --------------------------------------------------------------------------

BasicZoneManager<2> basicZoneManager(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only

  TimeZone::setZoneManager(&basicZoneManager);
}

void loop() {
  TestRunner::run();
}
