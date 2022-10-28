#line 2 "ExtendedZoneProcessorMoreTest.ino"

// Spillovers from ExtendedZoneProcessorTest.ino after it became too big for a
// SparkFun Pro Micro.

#include <AUnit.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::internal::ZoneContext;
using ace_time::extended::DateTuple;
using ace_time::extended::normalizeDateTuple;
using ace_time::extended::subtractDateTuple;

//---------------------------------------------------------------------------
// DateTuple.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, dateTupleOperatorLessThan) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 1, 2, 4, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 1, 3, 3, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 2, 2, 3, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2001, 1, 2, 3, ZoneContext::kSuffixS}));
}

test(ExtendedZoneProcessorTest, dateTupleOperatorEquals) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}));

  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixS}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 4, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 3, 3, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 2, 2, 3, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2001, 1, 2, 3, ZoneContext::kSuffixW}));
}

test(ExtendedZoneProcessorTest, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {2000, 1, 1, 0, ZoneContext::kSuffixW};
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*95, ZoneContext::kSuffixW}; // 23:45
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, 15*95, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*96, ZoneContext::kSuffixW}; // 24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*97, ZoneContext::kSuffixW}; // 24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 15, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, -15*96, ZoneContext::kSuffixW}; // -24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, -15*97, ZoneContext::kSuffixW}; // -24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, -15, ZoneContext::kSuffixW}));
}

test(ExtendedZoneProcessorTest, substractDateTuple) {
  DateTuple dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  DateTuple dtb = {2000, 1, 1, 1, ZoneContext::kSuffixW}; // 2000-01-01 00:01
  acetime_t diff = subtractDateTuple(dta, dtb);
  assertEqual(-60, diff);

  dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 1, 2, 0, ZoneContext::kSuffixW}; // 2000-01-02 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400, diff);

  dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 2, 1, 0, ZoneContext::kSuffixW}; // 2000-02-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 31, diff); // January has 31 days

  dta = {2000, 2, 1, 0, ZoneContext::kSuffixW}; // 2000-02-01 00:00
  dtb = {2000, 3, 1, 0, ZoneContext::kSuffixW}; // 2000-03-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 29, diff); // Feb 2000 is leap, 29 days
}

//---------------------------------------------------------------------------
// Test high level public methods of ExtendedZoneProcessor.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, setZoneKey) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  zoneProcessor.setZoneKey((uintptr_t) &zonedbx::kZoneAustralia_Darwin);
  assertFalse(zoneProcessor.mIsFilled);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneProcessor.setZoneKey((uintptr_t) &zonedbx::kZoneAustralia_Darwin);
  assertTrue(zoneProcessor.mIsFilled);
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ExtendedZoneProcessorTest, Los_Angeles) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  PrintStr<32> printStr;
  zoneProcessor.printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());
  printStr.flush();
  zoneProcessor.printShortNameTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());

  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

test(ExtendedZoneProcessorTest, Los_Angeles_outOfBounds) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedbx::kZoneContext.startYear);
  assertEqual(10000, zonedbx::kZoneContext.untilYear);

  // 1998 > LocalDate::kMinYear so dt is valid, and
  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  assertFalse(dt.isError());
  epochSeconds = dt.toEpochSeconds();
  // 1998 is within roughly 50 years of LocalDate::currentEpochYear() of 2050
  // so toEpochSeconds() still works.
  assertNotEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // 1998 < ZoneContext.startYear, so getUtcOffset() fails
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  // 1998 < ZoneContext.startYear, so getDeltaOffset() fails
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  // getAbbrev() returns "" on lookup failure
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(10001, 2, 1, 1, 0, 0,
      TimeOffset::forHours(-8));
  // 10001 > LocalDate::kMaxYear, so fails
  assertTrue(dt.isError());
  // toEpochSeconds() returns invalid seconds
  epochSeconds = dt.toEpochSeconds();
  assertEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // getUtcOffset() fails for kInvalidEpochSeconds
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  // getDeltaOffset() fails for kInvalidEpochSeconds
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  // getAbbrev() returns "" on lookup failure
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));
}

//---------------------------------------------------------------------------
// Test that getOffsetDateTime(acetime_t) returns correct fold parameter.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, forEpochSeconds_during_fall_back) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // Start our sampling at 01:29:00-07:00, which is 31 minutes before the DST
  // fall-back.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Verify fold==0 because this is the first time we're seeing this datetime.
  OffsetDateTime observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes later, we are at 01:59:00-07:00, a minute before fall-back, and
  // fold should be 0 because this is the first time seeing the datetime.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 59, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes into the overlap, we have either 02:29:00-07:00 or
  // 01:29:00-08:00. DST fall-back has occurred, so ExtendedZoneProcessor should
  // return 01:29:00-08:00, but with fold==1 because it's the second time we are
  // seeing this datetime.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(1, observed.fold());

  // Another 30 minutes into the overlap, we have either 02:59:00-07:00 or
  // 01:59:00-08:00. ExtendedZoneProcessor should return 01:59:00-08:00, but
  // with fold==1 because we are seeing this datetime a second time.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 59, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(1, observed.fold());

  // One more minute into the overlap, we have either 03:00:00-07:00 or
  // 02:00:00-08:00. ExtendedZoneProcessor should return 02:00:00-08:00,
  // with fold==0 because 02:00:00 was the exact point of fall-back and never
  // occurred twice.
  epochSeconds += 60;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 2, 0, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());
}

test(ExtendedZoneProcessorTest, forEpochSeconds_during_spring_forward) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // Start our sampling at 01:29:00-08:00, which is 31 minutes before the DST
  // spring-forward.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Verify fold==0 always, because spring-forward never causes repeats.
  OffsetDateTime observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes later, we are at 01:59:00-07:00, a minute before spring-forward.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 1, 59, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());

  // One minute later, we are at 02:00:00-08:00, which immediately turns into
  // 03:00:00-07:00.
  epochSeconds += 60;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 3, 0, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());
}

//---------------------------------------------------------------------------
// Test that getOffsetDateTime(const LocalDateTime&) handles fold parameter
// correctly.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, forComponents_during_fall_back) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // 01:29:00, before fall-back
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 11, 6, 1, 29, 0, 0 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7))
        == observed
    );

    // Verify fold remains unchanged.
    assertEqual(0, observed.fold());
  }

  // 01:29:00, after fall-back
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 11, 6, 1, 29, 0, 1 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-8))
        == observed
    );

    // Verify fold remains unchanged.
    assertEqual(1, observed.fold());
  }
}

test(ExtendedZoneProcessorTest, forComponents_during_spring_forward) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // 02:29:00 in gap, fold==0, uses earlier transition, so maps to the later UTC
  // time.
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 3, 13, 2, 29, 0, 0 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 3, 13, 3, 29, 0, TimeOffset::forHours(-7))
        == observed
    );

    // Verify that fold has flipped.
    assertEqual(1, observed.fold());
  }

  // 02:29:00 in gap, fold==1, uses later transition, so maps to the earlier UTC
  // time.
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 3, 13, 2, 29, 0, 1 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8))
        == observed
    );

    // Verify that fold has flipped.
    assertEqual(0, observed.fold());
  }
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
