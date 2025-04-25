#line 2 "ZonedExtraTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <testingzonedbx/zone_policies.h>
#include <testingzonedbx/zone_infos.h>

using namespace ace_time;

//---------------------------------------------------------------------------

test(ZonedExtra, isError) {
  ZonedExtra ze;
  assertTrue(ze.isError());
}

test(ZonedExtra, type) {
  assertEqual(ZonedExtra::kTypeNotFound, FindResult::kTypeNotFound);
  assertEqual(ZonedExtra::kTypeExact, FindResult::kTypeExact);
  assertEqual(ZonedExtra::kTypeGap, FindResult::kTypeGap);
  assertEqual(ZonedExtra::kTypeOverlap, FindResult::kTypeOverlap);
}

test(ZonedExtra, accessors) {
  const char s[] = "test";
  ZonedExtra ze(1, 2, 3, 4, 5, s);

  assertEqual(ze.type(), 1);
  assertEqual(ze.stdOffset().toSeconds(), 2);
  assertEqual(ze.dstOffset().toSeconds(), 3);
  assertEqual(ze.timeOffset().toSeconds(), 2+3);
  assertEqual(ze.reqStdOffset().toSeconds(), 4);
  assertEqual(ze.reqDstOffset().toSeconds(), 5);
  assertEqual(ze.reqTimeOffset().toSeconds(), 4+5);
  assertEqual(ze.abbrev(), "test");
}

test(ZonedExtra, forEpochSeconds) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // Find epochSeconds for the gap at 02:01, use fold=1 uses the second
  // transition, which then normalizes to the first transition, so 01:01-08:00
  auto odt = OffsetDateTime::forComponents(
      2018, 3, 11, 1, 0, 1, TimeOffset::forHours(-8));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Validate ZonedDateTime.
  {
    auto zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forEpochSeconds(epochSeconds, tz);
    assertEqual(-8*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PST", ze.abbrev());
  }

  // One hour after that, the local time should spring forward to 03:01-07:00
  epochSeconds += 3600;

  // Validate ZonedDateTime.
  {
    auto zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forEpochSeconds(epochSeconds, tz);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-7*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }
}

test(ZonedExtra, forLocalDateTime) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 02:01 in the gap with fold=0 selects the first transition, then normalizes
  // to the second transition, i.e. 03:01-07:00
  {
    auto ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 1, 0 /*fold*/);
    auto zdt = ZonedDateTime::forLocalDateTime(ldt, tz);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forLocalDateTime(ldt, tz);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }

  // 02:01 in the gap with fold=1 selects the second transition, then normalizes
  // to the first transition, i.e. 01:01-08:00
  {
    auto ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 1, 1 /*fold*/);
    auto zdt = ZonedDateTime::forLocalDateTime(ldt, tz);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forLocalDateTime(ldt, tz);
    assertEqual(-8*60, ze.timeOffset().toMinutes());
    assertEqual(-7*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PST", ze.abbrev());
  }
}

test(ZonedExtra, forComponents) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 02:01 in the gap with fold=0 selects the first transition, then normalizes
  // to the second transition, i.e. 03:01-07:00
  {
    auto zdt = ZonedDateTime::forComponents(
        2018, 3, 11, 2, 0, 1, tz, 0 /*fold*/);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forComponents(2018, 3, 11, 2, 0, 1, tz, 0 /*fold*/);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }

  // 02:01 in the gap with fold=1 selects the second transition, then normalizes
  // to the first transition, i.e. 01:01-08:00
  {
    auto zdt = ZonedDateTime::forComponents(
        2018, 3, 11, 2, 0, 1, tz, 1 /*fold*/);
    auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.localDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forComponents(2018, 3, 11, 2, 0, 1, tz, 1 /*fold*/);
    assertEqual(-8*60, ze.timeOffset().toMinutes());
    assertEqual(-7*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PST", ze.abbrev());
  }
}

//---------------------------------------------------------------------------

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
