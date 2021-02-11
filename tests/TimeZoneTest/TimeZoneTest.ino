#line 2 "TimeZoneTest.ino"

/*
 * The TimeZone, TimeZoneData, ManualZoneManager, BasicZoneManager and
 * ExtendedZoneManager classes are tightly interrelated, so we will test all of
 * those in this single test file.
 */

#include <AUnit.h>
#include <AceCommon.h> // PrintStr
#include <AceTime.h>

using aunit::TestRunner;
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

  assertNotEqual(TimeZone::kTypeManual, TimeZone::kTypeReserved);
  assertNotEqual(TimeZone::kTypeManual, BasicZoneProcessor::kTypeBasic);
  assertNotEqual(TimeZone::kTypeManual, ExtendedZoneProcessor::kTypeExtended);

  assertNotEqual(TimeZone::kTypeReserved, BasicZoneProcessor::kTypeBasic);
  assertNotEqual(TimeZone::kTypeReserved, ExtendedZoneProcessor::kTypeExtended);

  assertNotEqual(BasicZoneProcessor::kTypeBasic,
      ExtendedZoneProcessor::kTypeExtended);
}

//---------------------------------------------------------------------------
// TimeZone (kTypeError)
//---------------------------------------------------------------------------

test(TimeZoneTest, forError) {
  PrintStr<16> printStr;
  auto tz = TimeZone::forError();
  assertEqual(TimeZone::kTypeError, tz.getType());
  tz.printTo(printStr);
  assertEqual(F("<Error>"), printStr.getCstr());
}

//---------------------------------------------------------------------------
// TimeZone (kTypeManual)
//---------------------------------------------------------------------------

test(TimeZoneTest, manual_utc) {
  PrintStr<16> printStr;

  auto tz = TimeZone::forUtc();
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(0, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());
  assertTrue(tz.isUtc());
  assertEqual(F("UTC"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("UTC"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("UTC"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_no_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(0, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(0, tz.getDstOffset().toMinutes());
  assertEqual(F("STD"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("-08:00+00:00"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-08:00(STD)"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forTimeOffset_dst) {
  PrintStr<16> printStr;
  TimeZone tz = TimeZone::forTimeOffset(
      TimeOffset::forHours(-8),
      TimeOffset::forHours(1)
  );

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual(60, tz.getDeltaOffset(0).toMinutes());
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(60, tz.getDstOffset().toMinutes());
  assertEqual(F("DST"), tz.getAbbrev(0));

  tz.printTo(printStr);
  assertEqual(F("-08:00+01:00"), printStr.getCstr());
  printStr.flush();

  tz.printShortTo(printStr);
  assertEqual(F("-07:00(DST)"), printStr.getCstr());
  printStr.flush();
}

test(TimeZoneTest, forHours) {
  TimeZone tz = TimeZone::forHours(-8, 1);
  assertEqual(-8*60, tz.getStdOffset().toMinutes());
  assertEqual(1*60, tz.getDstOffset().toMinutes());
}

test(TimeZoneTest, forMinutes) {
  TimeZone tz = TimeZone::forMinutes(-120, 60);
  assertEqual(-120, tz.getStdOffset().toMinutes());
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

test(TimeZoneBasicTest, Los_Angeles) {
  OffsetDateTime dt;
  acetime_t epochSeconds;

  BasicZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  assertEqual(BasicZoneProcessor::kTypeBasic, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PST"), tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PDT"), tz.getAbbrev(epochSeconds));
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
  TimeZone losAngeles = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles, &basicZoneProcessor);
  TimeZone newYork = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_New_York, &basicZoneProcessor);

  assertEqual(zonedb::kZoneIdAmerica_Los_Angeles, losAngeles.getZoneId());
  assertEqual(zonedb::kZoneIdAmerica_New_York, newYork.getZoneId());

  // 2018-03-10 was still STD time, so both Los Angeles and New York should
  // return STD offset.
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 10, 0, 0, 0,
      TimeOffset::forHours(-8));
  acetime_t epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, losAngeles.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(-5*60, newYork.getUtcOffset(epochSeconds).toMinutes());

  assertEqual(0, losAngeles.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(0, newYork.getDeltaOffset(epochSeconds).toMinutes());

  assertEqual(F("PST"), losAngeles.getAbbrev(epochSeconds));
  assertEqual(F("EST"), newYork.getAbbrev(epochSeconds));

  PrintStr<32> printStr;
  losAngeles.printTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.getCstr());
  printStr.flush();
  newYork.printTo(printStr);
  assertEqual(F("America/New_York"), printStr.getCstr());
  printStr.flush();

  losAngeles.printShortTo(printStr);
  assertEqual(F("Los_Angeles"), printStr.getCstr());
  printStr.flush();
  newYork.printShortTo(printStr);
  assertEqual(F("New_York"), printStr.getCstr());
  printStr.flush();
}

//---------------------------------------------------------------------------
// TimeZone (ExtendedZoneProcessor::kTypeExtended)
//---------------------------------------------------------------------------

test(TimeZoneExtendedTest, Los_Angeles) {
  OffsetDateTime dt;
  acetime_t epochSeconds;

  ExtendedZoneProcessor zoneProcessor;
  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  assertEqual(ExtendedZoneProcessor::kTypeExtended, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(0, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PST"), tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual(1*60, tz.getDeltaOffset(epochSeconds).toMinutes());
  assertEqual(F("PDT"), tz.getAbbrev(epochSeconds));
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
      &zonedb::kZoneAmerica_Los_Angeles, &basicZoneProcessor);
  TimeZone basic2 = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_New_York, &basicZoneProcessor);
  assertTrue(basic != basic2);

  TimeZone extended = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles, &extendedZoneProcessor);
  TimeZone extended2 = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_New_York, &extendedZoneProcessor);
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
    {zonedb::kZoneIdAmerica_Los_Angeles}, // kTypeZoneId
  };
  assertTrue(TimeZoneData() == zones[0]);
  assertTrue(TimeZoneData(1, 2) == zones[1]);
  assertTrue(TimeZoneData(zonedb::kZoneIdAmerica_Los_Angeles) == zones[2]);
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
  TestRunner::run();
}
