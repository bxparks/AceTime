#line 2 "TzDbTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// Validate some changes in tzdb 2020a
// --------------------------------------------------------------------------

// Morocco springs forward on 2020-05-31, not on 2020-05-24 as originally
// scheduled. At 02:00 -> 03:00, the UTC offset goes from UTC+00:00 to
// UTC+01:00.
test(ZonedDateTimeExtendedTest, Morocco2020) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAfrica_Casablanca,
      &zoneProcessor
  );

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
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Whitehorse,
      &zoneProcessor
  );

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
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // for Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
