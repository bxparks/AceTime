#line 2 "ZonedDateTimeBasicTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include <zonedbtesting/zone_policies.h>
#include <zonedbtesting/zone_infos.h>

using namespace ace_time;
using ace_time::zonedbtesting::kZoneAmerica_Chicago;
using ace_time::zonedbtesting::kZoneAmerica_Denver;
using ace_time::zonedbtesting::kZoneAmerica_Los_Angeles;
using ace_time::zonedbtesting::kZoneAmerica_New_York;

// --------------------------------------------------------------------------
// Create BasicZoneManager
// --------------------------------------------------------------------------

const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &kZoneAmerica_Chicago,
  &kZoneAmerica_Denver,
  &kZoneAmerica_Los_Angeles,
  &kZoneAmerica_New_York,
};

const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

BasicZoneProcessorCache<1> zoneProcessorCache;

BasicZoneManager basicZoneManager(
    kBasicZoneRegistrySize, kBasicZoneRegistry, zoneProcessorCache);

// --------------------------------------------------------------------------
// ZonedDateTime + BasicZoneManager
// --------------------------------------------------------------------------

test(ZonedDateTimeBasicTest, printTo) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(2020, 1, 2, 3, 4, 5, tz);

  ace_common::PrintStr<64> dateString;
  dt.printTo(dateString);
  assertEqual(
      dateString.cstr(),
      F("2020-01-02T03:04:05-08:00[America/Los_Angeles]")
  );
}

test(ZonedDateTimeBasicTest, forComponents_isError) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // outside [1980, 10000) range, should generate error
  ZonedDateTime dt = ZonedDateTime::forComponents(1970, 3, 11, 1, 59, 0, tz);
  assertTrue(dt.isError());
  dt = ZonedDateTime::forComponents(10001, 3, 11, 1, 59, 0, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeBasicTest, forComponents_beforeDst) {
  TimeZone tz = basicZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 01:59 should resolve to 01:59-08:00
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

// TODO: Fix gap
/*
test(ZonedDateTimeBasicTest, forComponents_inDstGap) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // 02:01 doesn't exist, but BasicZoneProcessor picks the later epochSeconds
  // and offset, so should push forward the 02:01 to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  dt.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();
  assertTrue(expected == dt.localDateTime());
}
*/

test(ZonedDateTimeBasicTest, forComponents_inDst) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // 03:01 should resolve to 03:01-07:00
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeBasicTest, forComponents_beforeStd) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // 00:59 is more than an hour before the DST->STD transition so should
  // resolve to 00:59-07:00
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeBasicTest, forComponents_inOverlap) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // There were two instances of 01:01. BasicZoneProcessor chooses the earlier
  // occurrence, giving 01:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 1, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeBasicTest, forComponents_afterOverlap) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // 02:01 should always be 02:01-08:00
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 2, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

// --------------------------------------------------------------------------

test(ZonedDateTimeBasicTest, normalize) {
  TimeZone tz = basicZoneManager.createForZoneInfo(&kZoneAmerica_Los_Angeles);

  // Start with epochSeconds = 0. Should translate to 1999-12-31T16:00:00-08:00.
  auto dt = ZonedDateTime::forEpochSeconds(0, tz);
  assertEqual(Epoch::currentEpochYear() - 1, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(16, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());

  // Set the date/time to 2021-04-20T11:00:00, which happens to be in DST.
  dt.year(2021);
  dt.month(4);
  dt.day(20);
  dt.hour(9);
  dt.minute(0);
  dt.second(0);

  // If we blindly use the resulting epochSeconds to set the SystemClock, we
  // will be off by one hour, because the TimeOffset stored internally (-08:00)
  // does not match the TimeOffset that should appy at the new date (-07:00).
  acetime_t epochSeconds = dt.toEpochSeconds();
  auto newDt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2021, newDt.year());
  assertEqual(4, newDt.month());
  assertEqual(20, newDt.day());
  assertEqual(10, newDt.hour()); // should be 9, but becomes converted to 10.
  assertEqual(0, newDt.minute());
  assertEqual(0, newDt.second());

  // We must normalize() after mutation.
  dt.normalize();
  epochSeconds = dt.toEpochSeconds();
  newDt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2021, newDt.year());
  assertEqual(4, newDt.month());
  assertEqual(20, newDt.day());
  assertEqual(9, newDt.hour()); // will now be correct
  assertEqual(0, newDt.minute());
  assertEqual(0, newDt.second());
}

// --------------------------------------------------------------------------

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
