#line 2 "ZonedDateTimeExtendedTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>
#include <testingzonedbx/zone_policies.h>
#include <testingzonedbx/zone_infos.h>

using namespace ace_time;
using ace_time::extended::Info;
using ace_time::testingzonedbx::kZoneAmerica_Chicago;
using ace_time::testingzonedbx::kZoneAmerica_Denver;
using ace_time::testingzonedbx::kZoneAmerica_Los_Angeles;
using ace_time::testingzonedbx::kZoneAmerica_New_York;
using ace_time::testingzonedbx::kZoneAfrica_Casablanca;
using ace_time::testingzonedbx::kZonePacific_Apia;

// --------------------------------------------------------------------------

const Info::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_PROGMEM = {
  &kZoneAmerica_Chicago,
  &kZoneAmerica_Denver,
  &kZoneAmerica_Los_Angeles,
  &kZoneAmerica_New_York,
};

const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

ExtendedZoneProcessorCache<1> zoneProcessorCache;

ExtendedZoneManager extendedZoneManager(
    kExtendedZoneRegistrySize,
    kExtendedZoneRegistry,
    zoneProcessorCache);

// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, printTo) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(2020, 1, 2, 3, 4, 5, tz);

  ace_common::PrintStr<64> dateString;
  dt.printTo(dateString);
  assertEqual(
      dateString.cstr(),
      F("2020-01-02T03:04:05-08:00[America/Los_Angeles]")
  );
}

// --------------------------------------------------------------------------
// ZonedDateTime::forComponents()
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, forComponents_isError) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // outside [0, 10000) range, should generate error
  ZonedDateTime dt = ZonedDateTime::forComponents(-200, 3, 11, 1, 59, 59, tz);
  const OffsetDateTime &odt = dt.offsetDateTime();
  assertTrue(odt.isError());
  const PlainDateTime &pdt = dt.plainDateTime();
  assertTrue(pdt.isError());
  assertTrue(dt.isError());

  // outside [0, 10000) range, should generate error
  dt = ZonedDateTime::forComponents(10001, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeExtendedTest, forComponents_beforeGap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 01:59 before spring forward should resolve to 01:59-08:00
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 0, tz);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 3, 11, 1, 59, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);
}

test(ZonedDateTimeExtendedTest, forComponents_inGap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 02:01 is in the gap, select the later time, normalized to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kGapLater);

  // Same 02:01, select the earlier time, normalized to 01:01-08:00.
  dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz,
      Disambiguate::kReversed);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  expected = PlainDateTime::forComponents(2018, 3, 11, 1, 1, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kGapEarlier);
}

test(ZonedDateTimeExtendedTest, forComponents_afterGap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 03:01 is after the gap, should resolve to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 1, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);
}

test(ZonedDateTimeExtendedTest, forComponents_beforeOverlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 00:59 is an hour before the DST->STD transition, so should return
  // 00:59-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);
}

test(ZonedDateTimeExtendedTest, forComponents_inOverlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 01:02 occurs twice, select the earlier, should resolve to 01:02-08:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 2, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 11, 4, 1, 2, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kOverlapEarlier);

  // select earlier explicitly, resolves to 01:02-07:00.
  dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 2, 0, tz,
      Disambiguate::kEarlier);
  assertEqual(TimeOffset::forHours(-7).toMinutes(),
      dt.timeOffset().toMinutes());
  expected = PlainDateTime::forComponents(2018, 11, 4, 1, 2, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kOverlapEarlier);

  // Select later should resolve to 01:02-08:00.
  dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 2, 0, tz,
      Disambiguate::kLater);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  expected = PlainDateTime::forComponents(2018, 11, 4, 1, 2, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kOverlapLater);

  // Disambiguate::kReversed also selects later, resolves to 01:02-07:00.
  dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 2, 0, tz,
      Disambiguate::kReversed);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  expected = PlainDateTime::forComponents(2018, 11, 4, 1, 2, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kOverlapLater);
}

test(ZonedDateTimeExtendedTest, forComponents_afterOverlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 02:00 actually occurs only once, so should resolve to 02:00-08:00
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 0, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  auto expected = PlainDateTime::forComponents(2018, 11, 4, 2, 0, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);

  // 02:00 with kLater should give identical results, because no overlap
  dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 0, 0, tz,
      Disambiguate::kLater);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);

  // 02:00 with kEarlier should give same result, no overlap
  dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz,
      Disambiguate::kEarlier);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  expected = PlainDateTime::forComponents(2018, 11, 4, 2, 1, 0);
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);

  // 02:00 with kReversed should give same result, no overlap
  dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz,
      Disambiguate::kReversed);
  assertEqual(TimeOffset::forHours(-8).toMinutes(),
      dt.timeOffset().toMinutes());
  assertTrue(expected == dt.plainDateTime());
  assertEqual((uint8_t) dt.resolved(), (uint8_t) Resolved::kUnique);
}

// --------------------------------------------------------------------------
// ZonedDateTime::forEpochSeconds()
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, forEpochSecond_around_overlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // Start at 01:29:00-07:00, in overlap, 31 minutes before the fall-back.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7));
  acetime_t epochSeconds = odt.toEpochSeconds();
  auto dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2022, dt.year());
  assertEqual(11, dt.month());
  assertEqual(6, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-7*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // Go forward an hour. Should return 01:29:00-08:00, the second time this was
  // seen.
  epochSeconds += 3600;
  dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2022, dt.year());
  assertEqual(11, dt.month());
  assertEqual(6, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-8*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // Go forward another hour. Should return 02:29:00-08:00, which is unique.
  epochSeconds += 3600;
  dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2022, dt.year());
  assertEqual(11, dt.month());
  assertEqual(6, dt.day());
  assertEqual(2, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-8*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());
}

test(ZonedDateTimeExtendedTest, forEpochSecond_around_gap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // Start at 01:29:00-08:00, 31 minutes before the gap.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8));
  acetime_t epochSeconds = odt.toEpochSeconds();

  auto dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2022, dt.year());
  assertEqual(3, dt.month());
  assertEqual(13, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-8*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // An hour later, we spring forward to 03:29:00-07:00.
  epochSeconds += 3600;
  dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2022, dt.year());
  assertEqual(3, dt.month());
  assertEqual(13, dt.day());
  assertEqual(3, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-7*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());
}

// --------------------------------------------------------------------------
// ZonedDateTime::forComponents()
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, forComponents_overlap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 01:29:00, kCompatible should select the earlier, -07:00.
  auto dt = ZonedDateTime::forComponents(2022, 11, 6, 1, 29, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(2022, dt.year());
  assertEqual(11, dt.month());
  assertEqual(6, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-7*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kOverlapEarlier, (uint8_t) dt.resolved());

  // 01:29:00, kReversed should select the later, -08:00.
  dt = ZonedDateTime::forComponents(2022, 11, 6, 1, 29, 0, tz,
      Disambiguate::kReversed);
  assertEqual(2022, dt.year());
  assertEqual(11, dt.month());
  assertEqual(6, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-8*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kOverlapLater, (uint8_t) dt.resolved());
}

test(ZonedDateTimeExtendedTest, forComponents_gap) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // 02:29:00 is in the gap, kCompatible selects the later, -07:00
  auto dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz,
      Disambiguate::kCompatible);
  assertEqual(2022, dt.year());
  assertEqual(3, dt.month());
  assertEqual(13, dt.day());
  assertEqual(3, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-7*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapLater, (uint8_t) dt.resolved());

  // 02:29:00 is in the gap, kReversed selects the earlier, -08:00
  dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz,
      Disambiguate::kReversed);
  assertEqual(2022, dt.year());
  assertEqual(3, dt.month());
  assertEqual(13, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(29, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-8*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapEarlier, (uint8_t) dt.resolved());
}

// --------------------------------------------------------------------------
// ZonedDateTime::normalize()
// --------------------------------------------------------------------------

test(ZonedDateTimeExtendedTest, normalize) {
  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAmerica_Los_Angeles);

  // Start with epochSeconds = 0. Should translate to 1999-12-31T16:00:00-08:00.
  auto dt = ZonedDateTime::forEpochSeconds(0, tz);
  assertEqual(Epoch::currentEpochYear() - 1, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(16, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());

  // Set the date/time to 2021-04-20T09:00:00, which happens to be in DST.
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
  dt.normalize(Disambiguate::kCompatible);
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

// Validation for Morocco to test dates after the last explicit transition
// in the TZ database. The last explicit DST transition is the year 2087, so
// let's use 2090 which is 40 years after the custom epoch year of 2050,
// which makes it easy to calculate the expected epochSeconds. Even though the
// entry for 2087 is marked as "only" for 2087, the very last transition remains
// in effect for all years afterwards.
test(ZonedDateTimeExtendedTest, morocco_2090) {
  // Reconfigure the current epoch year to 2050 to allow calculations in the
  // year 2090.
  testing::EpochYearContext context(2050);

  TimeZone tz = extendedZoneManager.createForZoneInfo(
      &kZoneAfrica_Casablanca);

  // Normally Morocco is UTC+01:00, so epochSeconds==0 should translate to
  // 2050-01-01T01:00:00+01:00.
  auto dt = ZonedDateTime::forEpochSeconds(0, tz);
  assertEqual(2050, dt.year());
  assertEqual(1, dt.month());
  assertEqual(1, dt.day());
  assertEqual(1, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // During Ramadan, Morocco observes negative DST, and falls back to UTC+00:00.
  // In the year 2050, that happens at 2050-05-15T03:00:00. So 02:59:59 becomes
  // 02:00:00 one second later.
  dt = ZonedDateTime::forComponents(2050, 5, 15, 2, 59, 59, tz,
      Disambiguate::kCompatible);
  acetime_t epochSeconds = dt.toEpochSeconds();
  epochSeconds += 1;
  dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  assertEqual(2050, dt.year());
  assertEqual(5, dt.month());
  assertEqual(15, dt.day());
  assertEqual(2, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // Validate the epochSeconds of 2090-01-01. That date is exactly 40 years
  // after the custom epoch of 2050-01-01. So the number of elapsed epoch days
  // is 365 * 40 + 10 leap-days, or 14610 days.
  dt = ZonedDateTime::forComponents(2090, 1, 1, 1, 0, 0, tz);
  epochSeconds = dt.toEpochSeconds();
  assertEqual(epochSeconds, (int32_t) 14610 * 86400);
}

// --------------------------------------------------------------------------

// Pacific/Apia switches internation dateline, going from UTC-10 to UTC+14
// on Dec 29, 2011. It essentially skips Dec 30, 2011 using a mega "spring
// forward".
//
// Dateline Change skip Friday 30th Dec 2011
// Thursday 29th December 2011	23:59:59 Hours UTC-10
// Saturday 31st December 2011	00:00:00 Hours UTC+14
test(ZonedDateTimeExtendedTest, Pacific_Apia) {
  // Reconfigure the current epoch year to 2050 to allow calculations in the
  // year 2090.
  testing::EpochYearContext context(2050);

  TimeZone tz = extendedZoneManager.createForZoneInfo(&kZonePacific_Apia);

  // Dec 29 23:59:59 UTC-10:00, one second before the mega gap
  auto dt = ZonedDateTime::forComponents(
      2011, 12, 29, 23, 59, 59, tz, Disambiguate::kCompatible);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(29, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(-10*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // Dec 30, 00:00 is in the gap, kCompatible selects the later, Dec 31,
  // 00:00+14:00.
  dt = ZonedDateTime::forComponents(
      2011, 12, 30, 0, 0, 0, tz, Disambiguate::kCompatible);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(0, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(14*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapLater, (uint8_t) dt.resolved());

  // Dec 30, 00:00:00 is in the gap, kReversed selects the earlier, Dec 29,
  // 00:00-10:00.
  dt = ZonedDateTime::forComponents(
      2011, 12, 30, 0, 0, 0, tz, Disambiguate::kReversed);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(29, dt.day());
  assertEqual(0, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-10*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapEarlier, (uint8_t) dt.resolved());

  // Dec 30, 12:00:00 is still in the gap. Using fold=0 means using the first
  // transition of UTC-10, which then gets normalized to Dec 31, 12:00:00
  // UTC+14:00.
  dt = ZonedDateTime::forComponents(
      2011, 12, 30, 12, 0, 0, tz, Disambiguate::kCompatible);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(12, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(14*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapLater, (uint8_t) dt.resolved());

  // Dec 30, 12:00:00 is still in the gap, kReversed selects earlier,
  // Dec 29, 12:00:00-10:00.
  dt = ZonedDateTime::forComponents(
      2011, 12, 30, 12, 0, 0, tz, Disambiguate::kReversed);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(29, dt.day());
  assertEqual(12, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(-10*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kGapEarlier, (uint8_t) dt.resolved());

  // Dec 31, 00:00:00 is just after the gap, kCompatible selects unique.
  dt = ZonedDateTime::forComponents(
      2011, 12, 31, 0, 0, 0, tz, Disambiguate::kCompatible);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(0, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(14*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());

  // Dec 31, 00:00:00 is outside the gap, kReversed selects same unique.
  dt = ZonedDateTime::forComponents(
      2011, 12, 31, 0, 0, 0, tz, Disambiguate::kReversed);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(0, dt.hour());
  assertEqual(0, dt.minute());
  assertEqual(0, dt.second());
  assertEqual(14*60, dt.timeOffset().toMinutes());
  assertEqual((uint8_t) Resolved::kUnique, (uint8_t) dt.resolved());
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // for Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
