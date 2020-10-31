#line 2 "ZonedDateTimeExtendedTest.ino"

#include <AUnit.h>
#include <AceCommon.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZonedDateTime + ExtendedZoneProcessor
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, printTo) {
  ExtendedZoneProcessor zoneProcessor;
  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  auto dt = ZonedDateTime::forComponents(2020, 1, 2, 3, 4, 5, tz);

  ace_common::PrintStr<64> dateString;
  dt.printTo(dateString);
  assertEqual(
      dateString.getCstr(),
      "2020-01-02T03:04:05-08:00[America/Los_Angeles]"
  );
}

// --------------------------------------------------------------------------
// ExtendedZoneManager
// --------------------------------------------------------------------------

const extended::ZoneInfo* const kExtendedZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneManager<2> extendedZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);

// --------------------------------------------------------------------------
// ZonedDateTime + ExtendedZoneManager
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, forComponents_isError) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // outside [2000, 2050) range, should generate error
  ZonedDateTime dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
  dt = ZonedDateTime::forComponents(2051, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeExtendedTest, forComponents_beforeDst) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // 01:59 should resolve to 01:59-08:00
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeExtendedTest, forComponents_inDstGap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // 02:01 doesn't exist. The expected TimeOffset in the gap is the previous
  // timeOffset, i.e. the most recent matching Transition, so this is
  // interpreted as 02:01-08:00 which gets normalized to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeExtendedTest, forComponents_inDst) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // 03:01 should resolve to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeExtendedTest, forComponents_beforeStd) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // 00:59 is an hour before the DST->STD transition, so should return
  // 00:59-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeExtendedTest, forComponents_inOverlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // There were two instances of 01:01. The algorithm picks the later offset,
  // i.e. the most recent matching Transition, so should resolve to 01:01-08:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 1, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeExtendedTest, forComponents_afterOverlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);

  // 02:01 should resolve to 02:01-08:00
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 2, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

// Test the linked zones are same as the target zones.
test(ZonedDateTimeExtendedTest, linked_zones) {
  assertEqual((intptr_t) &zonedbx::kZoneAmerica_Los_Angeles,
      (intptr_t) &zonedbx::kZoneUS_Pacific);
}

// --------------------------------------------------------------------------
// Validate some changes in tzdb 2020a
// --------------------------------------------------------------------------

// Morocco springs forward on 2020-05-31, not on 2020-05-24 as originally
// scheduled. At 02:00 -> 03:00, the UTC offset goes from UTC+00:00 to
// UTC+01:00.
test(ZonedDateTimeExtendedTest, Morocco2020) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAfrica_Casablanca);

  auto dt = ZonedDateTime::forComponents(2020, 5, 25, 3, 0, 0, tz);
  assertEqual(TimeOffset::forHours(0).toMinutes(),
      dt.timeOffset().toMinutes());
  acetime_t epoch = dt.toEpochSeconds();
  assertEqual("+00", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(-1).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());

  dt = ZonedDateTime::forComponents(2020, 5, 31, 1, 59, 59, tz);
  assertEqual(TimeOffset::forHours(0).toMinutes(),
      dt.timeOffset().toMinutes());
  epoch = dt.toEpochSeconds();
  assertEqual("+00", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(-1).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());

  dt = ZonedDateTime::forComponents(2020, 5, 31, 3, 0, 0, tz);
  assertEqual(TimeOffset::forHours(1).toMinutes(),
      dt.timeOffset().toMinutes());
  epoch = dt.toEpochSeconds();
  assertEqual("+01", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(0).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());
}

// --------------------------------------------------------------------------
// Validate some changes in tzdb 2020c
// --------------------------------------------------------------------------

// Yukon (e.g. America/Whitehorse) goes to permanent daylight saving time
// starting on 2020-11-01T00:00. It goes from PDT (UTC-07:00) to permanent MST
// (UTC-07:00) at 2020-11-01 00:00.
test(ZonedDateTimeExtendedTest, Yukon2020) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Whitehorse);

  auto dt = ZonedDateTime::forComponents(2020, 3, 8, 1, 59, 59, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  acetime_t epoch = dt.toEpochSeconds();
  assertEqual("PST", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(0).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());

  // Time in the 2:00->3:00 transition gap. Gets normalized to 03:00.
  dt = ZonedDateTime::forComponents(2020, 3, 8, 2, 0, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2020, 3, 8, 3, 0, 0);
  assertTrue(expected == dt.localDateTime());
  epoch = dt.toEpochSeconds();
  assertEqual("PDT", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(1).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());

  // 23:59->00:00, but there's a change in abbreviation and DST offset.
  dt = ZonedDateTime::forComponents(2020, 10, 31, 23, 59, 59, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  epoch = dt.toEpochSeconds();
  assertEqual("PDT", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(1).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());

  // 00:00->00:00, but there's a change in abbreviation and DST offset.
  dt = ZonedDateTime::forComponents(2020, 11, 1, 0, 0, 0, tz);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  epoch = dt.toEpochSeconds();
  assertEqual("MST", tz.getAbbrev(epoch));
  assertEqual(TimeOffset::forHours(0).toMinutes(),
      tz.getDeltaOffset(epoch).toMinutes());
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
