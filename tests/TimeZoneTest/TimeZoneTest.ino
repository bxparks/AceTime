#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>
#include <testingzonedb/zone_infos.h>
#include <testingzonedbx/zone_infos.h>
#include <testingzonedbc/zone_infos.h>

using ace_common::PrintStr;
using namespace ace_time;

//---------------------------------------------------------------------------
// Check that TimeZone::kType* are distinct
//---------------------------------------------------------------------------

test(TimeZoneTest, kType_distinct) {
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeManual);
  assertNotEqual(TimeZone::kTypeError, TimeZone::kTypeReserved);
  assertNotEqual(TimeZone::kTypeError, BasicZoneProcessor::kTypeBasic);
  assertNotEqual(TimeZone::kTypeError, ExtendedZoneProcessor::kTypeExtended);
  assertNotEqual(TimeZone::kTypeError, CompleteZoneProcessor::kTypeComplete);

  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeReserved);
  assertNotEqual(TimeZone::kTypeManual, BasicZoneProcessor::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, ExtendedZoneProcessor::kTypeExtended);
  assertNotEqual(TimeZone::kTypeManual, CompleteZoneProcessor::kTypeComplete);

  assertNotEqual(TimeZone::kTypeReserved, BasicZoneProcessor::kTypeBasic);
  assertNotEqual(TimeZone::kTypeReserved, ExtendedZoneProcessor::kTypeExtended);
  assertNotEqual(TimeZone::kTypeReserved, CompleteZoneProcessor::kTypeComplete);

  assertNotEqual(
      BasicZoneProcessor::kTypeBasic,
      ExtendedZoneProcessor::kTypeExtended);
  assertNotEqual(
      BasicZoneProcessor::kTypeBasic,
      CompleteZoneProcessor::kTypeComplete);

  assertNotEqual(
      ExtendedZoneProcessor::kTypeExtended,
      CompleteZoneProcessor::kTypeComplete);
}

//---------------------------------------------------------------------------
// TimeZone (kTypeError)
//---------------------------------------------------------------------------

test(TimeZoneTest, forError) {
  PrintStr<16> printStr;
  auto tz = TimeZone::forError();
  assertEqual(TimeZone::kTypeError, tz.getType());
  tz.printTo(printStr);
  assertEqual(F("<Error>"), printStr.cstr());
}

//---------------------------------------------------------------------------
// TimeZone (kTypeManual)
//---------------------------------------------------------------------------

test(TimeZoneTest, forUtc) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forUtc();

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertTrue(tz.isUtc());
  assertEqual(0, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());

  ZonedExtra ze = tz.getZonedExtra(0);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(0, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(0, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("UTC"), ze.abbrev());

  tz.printTo(printStr);
  assertEqual(F("UTC"), printStr.cstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("UTC"), printStr.cstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_no_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertFalse(tz.isUtc());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(0*60, tz.getDstOffset().toMinutes());

  ZonedExtra ze = tz.getZonedExtra(0);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("STD"), ze.abbrev());

  tz.printTo(printStr);
  assertEqual(F("-08:00+00:00"), printStr.cstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-08:00(S)"), printStr.cstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(
      TimeOffset::forHours(-8),
      TimeOffset::forHours(1)
  );

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertFalse(tz.isUtc());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());

  ZonedExtra ze = tz.getZonedExtra(0);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(60, ze.reqDstOffset().toMinutes());
  assertEqual(F("DST"), ze.abbrev());

  tz.printTo(printStr);
  assertEqual(F("-08:00+01:00"), printStr.cstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-07:00(D)"), printStr.cstr());
  printStr.flush();
}

test(TimeZoneTest, forHours) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(1*60, tz.getDstOffset().toMinutes());
}

test(TimeZoneTest, forMinutes) {
  TimeZone tz = TimeZone::forMinutes(-480, 60);
  assertEqual(-480, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());
}

test(TimeZoneTest, forHourMinute) {
  TimeZone tz = TimeZone::forHourMinute(-4, -15, 1, 30);
  assertEqual(-4*60-15, tz.getStdOffset().toMinutes());
  assertEqual(1*60+30, tz.getDstOffset().toMinutes());
}

//---------------------------------------------------------------------------
// TimeZone (BasicZoneProcessor::kTypeBasic)
//---------------------------------------------------------------------------

test(TimeZoneBasicTest, printTo) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  assertEqual(BasicZoneProcessor::kTypeBasic, tz.getType());
  assertFalse(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());
  printStr.flush();
  tz.printShortTo(printStr);
  assertEqual("Los Angeles", printStr.cstr());
}

test(TimeZoneBasicTest, getZonedExtra) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // right after spring forward to DST in the gap
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
}

// Test a Link: US/Pacific -> America/Los_Angeles
test(TimeZoneBasicTest, link) {
  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedb::kZoneUS_Pacific,
      &zoneProcessor);

  assertEqual(BasicZoneProcessor::kTypeBasic, tz.getType());
  assertTrue(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("US/Pacific", printStr.cstr());

  printStr.flush();
  tz.printTargetNameTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());

  assertEqual(testingzonedb::kZoneIdUS_Pacific, tz.getZoneId());

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // just before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // just after spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
}

// A ZoneManager could bind the same ZoneProcessor to 2 different TimeZone if
// it runs out of free processors in the cache. Verify that various methods on
// TimeZone calls getBoundZoneProcessor() properly to rebind the zoneProcessor
// properly before performing any calculations on the zoneProcessor.
//
// In theory we should do the same testing for ExtendedZoneProcessor, but the
// code path in TimeZone should be identical so we'll just implement only this
// version to save flash memory.
test(TimeZoneBasicTest, zoneProcessor_rebinding) {
  BasicZoneProcessor basicZoneProcessor;
  TimeZone tzla = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_Los_Angeles, &basicZoneProcessor);
  TimeZone tzny = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_New_York, &basicZoneProcessor);

  assertEqual(testingzonedb::kZoneIdAmerica_Los_Angeles, tzla.getZoneId());
  assertEqual(testingzonedb::kZoneIdAmerica_New_York, tzny.getZoneId());

  // 2018-03-10 was still STD time, so both Los Angeles and New York should
  // return STD offset.
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 10, 0, 0, 0,
      TimeOffset::forHours(-8));
  acetime_t epochSeconds = dt.toEpochSeconds();

  // Calls getBoundProcessor() and rebinds.
  ZonedExtra zela = tzla.getZonedExtra(epochSeconds);
  assertEqual(-8*60, zela.stdOffset().toMinutes());
  assertEqual(0*60, zela.dstOffset().toMinutes());
  assertEqual(-8*60, zela.reqStdOffset().toMinutes());
  assertEqual(0*60, zela.reqDstOffset().toMinutes());
  assertEqual(F("PST"), zela.abbrev());

  // Calls getBoundProcessor() and rebinds.
  ZonedExtra zeny = tzny.getZonedExtra(epochSeconds);
  assertEqual(-5*60, zeny.stdOffset().toMinutes());
  assertEqual(0*60, zeny.dstOffset().toMinutes());
  assertEqual(-5*60, zeny.reqStdOffset().toMinutes());
  assertEqual(0*60, zeny.reqDstOffset().toMinutes());
  assertEqual(F("EST"), zeny.abbrev());

  // printTo() calls getBoundProcessor() and rebinds.
  PrintStr<32> printStr;
  tzla.printTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());
  printStr.flush();
  tzny.printTo(printStr);
  assertEqual(F("America/New_York"), printStr.cstr());
  printStr.flush();

  // printShortTo() calls getBoundProcessor() and rebinds.
  tzla.printShortTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());
  printStr.flush();
  tzny.printShortTo(printStr);
  assertEqual(F("New York"), printStr.cstr());
  printStr.flush();
}

//---------------------------------------------------------------------------
// TimeZone (ExtendedZoneProcessor::kTypeExtended)
//---------------------------------------------------------------------------

test(TimeZoneExtendedTest, printTo) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  assertEqual(ExtendedZoneProcessor::kTypeExtended, tz.getType());
  assertFalse(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());
  printStr.flush();
  tz.printShortTo(printStr);
  assertEqual("Los Angeles", printStr.cstr());
}

test(TimeZoneExtendedTest, getZoneExtra) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // right after spring forward to DST, this is a gap
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // just before fall back overlap
  ldt = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // right at fall back overlap, 01:00 occurs twice, fold=0 picks the earlier
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // fall back overlap, 01:00 occurs twice, fold=1 picks the later
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 0, 0, 1 /*fold*/);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // 02:00 occurs once, after an hour of overlap
  ldt = LocalDateTime::forComponents(2018, 11, 4, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
}

// Test a Link: US/Pacific -> America/Los_Angeles
test(TimeZoneExtendedTest, link) {
  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneUS_Pacific,
      &zoneProcessor);

  assertEqual(ExtendedZoneProcessor::kTypeExtended, tz.getType());
  assertTrue(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("US/Pacific", printStr.cstr());

  printStr.flush();
  tz.printTargetNameTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());

  assertEqual(testingzonedb::kZoneIdUS_Pacific, tz.getZoneId());

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // just before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // just after spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
}

//---------------------------------------------------------------------------
// TimeZone (CompleteZoneProcessor::kTypeComplete)
//---------------------------------------------------------------------------

test(TimeZoneCompleteTest, printTo) {
  CompleteZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbc::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  assertEqual(CompleteZoneProcessor::kTypeComplete, tz.getType());
  assertFalse(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());
  printStr.flush();
  tz.printShortTo(printStr);
  assertEqual("Los Angeles", printStr.cstr());
}

test(TimeZoneCompleteTest, getZoneExtra) {
  CompleteZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbc::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // right after spring forward to DST, this is a gap
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // just before fall back overlap
  ldt = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // right at fall back overlap, 01:00 occurs twice, fold=0 picks the earlier
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());

  // fall back overlap, 01:00 occurs twice, fold=1 picks the later
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 0, 0, 1 /*fold*/);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeOverlap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // 02:00 occurs once, after an hour of overlap
  ldt = LocalDateTime::forComponents(2018, 11, 4, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
}

// Test a Link: US/Pacific -> America/Los_Angeles
test(TimeZoneCompleteTest, link) {
  CompleteZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &testingzonedbc::kZoneUS_Pacific,
      &zoneProcessor);

  assertEqual(CompleteZoneProcessor::kTypeComplete, tz.getType());
  assertTrue(tz.isLink());

  PrintStr<32> printStr;
  tz.printTo(printStr);
  assertEqual("US/Pacific", printStr.cstr());

  printStr.flush();
  tz.printTargetNameTo(printStr);
  assertEqual("America/Los_Angeles", printStr.cstr());

  assertEqual(testingzonedb::kZoneIdUS_Pacific, tz.getZoneId());

  LocalDateTime ldt;
  OffsetDateTime dt;
  acetime_t epochSeconds;
  ZonedExtra ze;

  // just before spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(0*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PST"), ze.abbrev());

  // just after spring forward to DST
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 0);
  ze = tz.getZonedExtra(ldt);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeGap, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(0*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
  //
  dt = OffsetDateTime::forLocalDateTimeAndOffset(
      ldt, TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  ze = tz.getZonedExtra(epochSeconds);
  assertFalse(ze.isError());
  assertEqual(ZonedExtra::kTypeExact, ze.type());
  assertEqual(-8*60, ze.stdOffset().toMinutes());
  assertEqual(1*60, ze.dstOffset().toMinutes());
  assertEqual(-8*60, ze.reqStdOffset().toMinutes());
  assertEqual(1*60, ze.reqDstOffset().toMinutes());
  assertEqual(F("PDT"), ze.abbrev());
}

//---------------------------------------------------------------------------
// operator==() for kTypeManual, kTypeBasic and kTypeExtended.
//---------------------------------------------------------------------------

test(TimeZoneMoreTest, operatorEqualEqual_directZone) {
  BasicZoneProcessor basicZoneProcessor;
  ExtendedZoneProcessor extendedZoneProcessor;

  TimeZone manual = TimeZone::forHours(-8);
  TimeZone manual2 = TimeZone::forHours(-7);
  assertTrue(manual != manual2);

  TimeZone basic = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_Los_Angeles, &basicZoneProcessor);
  TimeZone basic2 = TimeZone::forZoneInfo(
      &testingzonedb::kZoneAmerica_New_York, &basicZoneProcessor);
  assertTrue(basic != basic2);

  TimeZone extended = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_Los_Angeles, &extendedZoneProcessor);
  TimeZone extended2 = TimeZone::forZoneInfo(
      &testingzonedbx::kZoneAmerica_New_York, &extendedZoneProcessor);
  assertTrue(extended != extended2);

  assertTrue(manual != basic);
  assertTrue(manual != extended);
  assertTrue(basic != extended);
}

//---------------------------------------------------------------------------
// TimeZoneData
//---------------------------------------------------------------------------

// We can use initializer lists, just like regular structs!
test(TimeZoneDataTest, array_initialization) {
  TimeZoneData zones[3] = {
    {}, // kTypeError
    {1, 2}, // kTypeManual
    {testingzonedb::kZoneIdAmerica_Los_Angeles}, // kTypeZoneId
  };
  assertTrue(TimeZoneData() == zones[0]);
  assertTrue(TimeZoneData(1, 2) == zones[1]);
  assertTrue(
      TimeZoneData(testingzonedb::kZoneIdAmerica_Los_Angeles) == zones[2]);
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
