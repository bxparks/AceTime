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

test(ZonedExtra, accessors) {
  const char s[] = "test";
  ZonedExtra ze(Resolved::kUnique, 2, 3, 4, 5, s);

  assertEqual((uint8_t)ze.resolved(), (uint8_t)Resolved::kUnique);
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

  // Find epochSeconds for 01:01-8:00.
  auto odt = OffsetDateTime::forComponents(
      2018, 3, 11, 1, 0, 1, TimeOffset::forHours(-8));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Validate ZonedDateTime.
  {
    auto zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forEpochSeconds(epochSeconds, tz);
    assertEqual(-8*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PST", ze.abbrev());
  }

  // One hour after that, 02:01-08:00 should spring forward to 03:01-07:00
  epochSeconds += 3600;

  // Validate ZonedDateTime.
  {
    auto zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forEpochSeconds(epochSeconds, tz);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-7*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }
}

test(ZonedExtra, forPlainDateTime) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 02:01 is in the gap, kCompatible selects the later time, 03:01-07:00
  {
    auto pdt = PlainDateTime::forComponents(2018, 3, 11, 2, 0, 1);
    auto zdt = ZonedDateTime::forPlainDateTime(
        pdt, tz, Disambiguate::kCompatible);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forPlainDateTime(pdt, tz, Disambiguate::kCompatible);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }

  // 02:01 is in the gap, kReversed selects the earlier time 01:01-08:00
  {
    auto pdt = PlainDateTime::forComponents(2018, 3, 11, 2, 0, 1);
    auto zdt = ZonedDateTime::forPlainDateTime(
        pdt, tz, Disambiguate::kReversed);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forPlainDateTime(
        pdt, tz, Disambiguate::kReversed);
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

  // 02:01 is in the gap, kCompatible selects later time, 03:01-07:00
  {
    auto zdt = ZonedDateTime::forComponents(
        2018, 3, 11, 2, 0, 1, tz, Disambiguate::kCompatible);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 3, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-7*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forComponents(
        2018, 3, 11, 2, 0, 1, tz, Disambiguate::kCompatible);
    assertEqual(-7*60, ze.timeOffset().toMinutes());
    assertEqual(-8*60, ze.reqTimeOffset().toMinutes());
    assertEqual("PDT", ze.abbrev());
  }

  // 02:01 is in the gap, kReversed selects the earlier time, 01:01-08:00
  {
    auto zdt = ZonedDateTime::forComponents(
        2018, 3, 11, 2, 0, 1, tz, Disambiguate::kReversed);
    auto expected = PlainDateTime::forComponents(2018, 3, 11, 1, 0, 1);
    assertTrue(expected == zdt.plainDateTime());
    assertEqual(-8*60, zdt.timeOffset().toMinutes());
    // Validate ZonedExtra.
    auto ze = ZonedExtra::forComponents(
        2018, 3, 11, 2, 0, 1, tz, Disambiguate::kReversed);
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
