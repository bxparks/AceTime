#line 2 "PlainDateTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>

using namespace ace_time;
using ace_time::basic::Info;

//---------------------------------------------------------------------------
// PlainDate
//---------------------------------------------------------------------------

test(PlainDateTest, year_limits) {
  assertLess(PlainDate::kInvalidYear, PlainDate::kMinYear);
  assertLess(PlainDate::kInvalidYear, Info::ZoneContext::kMinYear);
  assertLess(PlainDate::kInvalidYear, Info::ZoneContext::kMinYear);

  assertLess(PlainDate::kMinYear, PlainDate::kMaxYear);
  assertLess(PlainDate::kMinYear, PlainDate::kMaxYear);

  assertLess(Info::ZoneContext::kMinYear, Info::ZoneContext::kMaxYear);
  assertLess(Info::ZoneContext::kMinYear, Info::ZoneContext::kMaxYear);

  assertLess(Info::ZoneContext::kMaxYear, Info::ZoneContext::kMaxUntilYear);
  assertLess(Info::ZoneContext::kMaxYear, Info::ZoneContext::kMaxUntilYear);

  // PlainDate limits must be within the Info::ZoneContext limits
  assertMore(PlainDate::kMinYear, Info::ZoneContext::kMinYear);
  assertLess(PlainDate::kMaxYear, Info::ZoneContext::kMaxYear);
}

test(PlainDateTest, year_range) {
  // Not valid
  assertTrue(PlainDate::forComponents(INT16_MIN, 1, 1).isError());
  assertTrue(PlainDate::forComponents(-1, 1, 1).isError());

  // kMinYear allowed
  assertFalse(PlainDate::forComponents(0, 1, 1).isError());

  // first valid year allowed
  assertFalse(PlainDate::forComponents(1, 1, 1).isError());

  // largest valid FROM or TO year ("max"), allowed
  assertFalse(PlainDate::forComponents(9999, 1, 1).isError());

  // largest valid UNTIL year ("-"), allowed, kMaxYear
  assertFalse(PlainDate::forComponents(10000, 1, 1).isError());

  // Not valid
  assertTrue(PlainDate::forComponents(PlainDate::kMaxYear + 1, 1, 1).isError());
  assertTrue(PlainDate::forComponents(INT16_MAX, 1, 1).isError());
}

test(PlainDateTest, month_range) {
  assertTrue(PlainDate::forComponents(2000, 0, 1).isError());
  assertTrue(PlainDate::forComponents(2000, 13, 1).isError());
}

test(PlainDateTest, day_range) {
  assertTrue(PlainDate::forComponents(2000, 1, 0).isError());
  assertTrue(PlainDate::forComponents(2000, 1, 32).isError());
}

test(PlainDateTest, accessors) {
  PlainDate pd = PlainDate::forComponents(2001, 2, 3);
  assertEqual((int16_t) 2001, pd.year());
  assertEqual(2, pd.month());
  assertEqual(3, pd.day());
}

// Verify that toEpochDays()/forEpochDays() and
// toEpochSeconds()/forEpochSeconds() support round trip conversions when when
// isError()==true.
test(PlainDateTest, forError_roundTrip) {
  PlainDate pd;

  pd = PlainDate::forError();
  assertTrue(pd.isError());
  assertEqual(PlainDate::kInvalidEpochDays, pd.toEpochDays());
  assertEqual(PlainDate::kInvalidEpochSeconds, pd.toEpochSeconds());

  pd = PlainDate::forEpochDays(PlainDate::kInvalidEpochDays);
  assertTrue(pd.isError());

  pd = PlainDate::forEpochSeconds(PlainDate::kInvalidEpochSeconds);
  assertTrue(pd.isError());
}

test(PlainDateTest, dayOfWeek) {
  // year 1900 (not leap year due to every 100 rule)
  assertEqual(PlainDate::kWednesday,
      PlainDate::forComponents(1900, 2, 28).dayOfWeek());
  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(1900, 3, 1).dayOfWeek());

  // year 1999 was not a leap year
  assertEqual(PlainDate::kFriday,
      PlainDate::forComponents(1999, 1, 1).dayOfWeek());
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(1999, 1, 31).dayOfWeek());

  // year 2000 (leap year due to every 400 rule)
  assertEqual(PlainDate::kSaturday,
      PlainDate::forComponents(2000, 1, 1).dayOfWeek());
  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2000, 1, 31).dayOfWeek());

  assertEqual(PlainDate::kTuesday,
      PlainDate::forComponents(2000, 2, 1).dayOfWeek());
  assertEqual(PlainDate::kTuesday,
      PlainDate::forComponents(2000, 2, 29).dayOfWeek());

  assertEqual(PlainDate::kWednesday,
      PlainDate::forComponents(2000, 3, 1).dayOfWeek());
  assertEqual(PlainDate::kFriday,
      PlainDate::forComponents(2000, 3, 31).dayOfWeek());

  assertEqual(PlainDate::kSaturday,
      PlainDate::forComponents(2000, 4, 1).dayOfWeek());
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2000, 4, 30).dayOfWeek());

  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2000, 5, 1).dayOfWeek());
  assertEqual(PlainDate::kWednesday,
      PlainDate::forComponents(2000, 5, 31).dayOfWeek());

  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(2000, 6, 1).dayOfWeek());
  assertEqual(PlainDate::kFriday,
      PlainDate::forComponents(2000, 6, 30).dayOfWeek());

  assertEqual(PlainDate::kSaturday,
      PlainDate::forComponents(2000, 7, 1).dayOfWeek());
  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2000, 7, 31).dayOfWeek());

  assertEqual(PlainDate::kTuesday,
      PlainDate::forComponents(2000, 8, 1).dayOfWeek());
  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(2000, 8, 31).dayOfWeek());

  assertEqual(PlainDate::kFriday,
      PlainDate::forComponents(2000, 9, 1).dayOfWeek());
  assertEqual(PlainDate::kSaturday,
      PlainDate::forComponents(2000, 9, 30).dayOfWeek());

  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2000, 10, 1).dayOfWeek());
  assertEqual(PlainDate::kTuesday,
      PlainDate::forComponents(2000, 10, 31).dayOfWeek());

  assertEqual(PlainDate::kWednesday,
      PlainDate::forComponents(2000, 11, 1).dayOfWeek());
  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(2000, 11, 30).dayOfWeek());

  assertEqual(PlainDate::kFriday,
      PlainDate::forComponents(2000, 12, 1).dayOfWeek());
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2000, 12, 31).dayOfWeek());

  // year 2001
  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2001, 1, 1).dayOfWeek());
  assertEqual(PlainDate::kWednesday,
      PlainDate::forComponents(2001, 1, 31).dayOfWeek());

  // year 2004 (leap year)
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2004, 2, 1).dayOfWeek());
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2004, 2, 29).dayOfWeek());
  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2004, 3, 1).dayOfWeek());

  // year 2099
  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(2099, 1, 1).dayOfWeek());
  assertEqual(PlainDate::kThursday,
      PlainDate::forComponents(2099, 12, 31).dayOfWeek());

  // year 2100 (not leap year due to every 100 rule)
  assertEqual(PlainDate::kSunday,
      PlainDate::forComponents(2100, 2, 28).dayOfWeek());
  assertEqual(PlainDate::kMonday,
      PlainDate::forComponents(2100, 3, 1).dayOfWeek());
}

test(PlainDateTest, toAndFromEpochDays) {
  // Change current epoch year to 2050, making the epoch 2100-01-01T00:00:00.
  testing::EpochYearContext context(2000);

  PlainDate pd;

  // Smallest PlainDate in our 16-bit implementation is 0001-01-01
  pd = PlainDate::forComponents(1, 1, 1);
  int32_t epoch1 = (int32_t) ((0 - 2000) / 400) * 146097 + 366;
  assertEqual(epoch1, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(epoch1));

  // Smallest PlainDate in an 8-bit implementation
  pd = PlainDate::forComponents(1873, 1, 1);
  assertEqual((int32_t) -46385, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(-46385));

  pd = PlainDate::forComponents(1900, 1, 1);
  assertEqual((int32_t) -36524, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(-36524));

  // Smallest date using int32_t seconds from AceTime epoch
  pd = PlainDate::forComponents(1931, 12, 14);
  assertEqual((int32_t) -24855, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(-24855));

  // AceTime v1 hardcoded epoch
  pd = PlainDate::forComponents(2000, 1, 1);
  assertEqual((int32_t) 0, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(0));

  pd = PlainDate::forComponents(2000, 2, 29);
  assertEqual((int32_t) 59, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(59));

  pd = PlainDate::forComponents(2018, 1, 1);
  assertEqual((int32_t) 6575, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(6575));

  // AceTime v2 default epoch
  pd = PlainDate::forComponents(2050, 1, 1);
  assertEqual((int32_t) 18263, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(18263));

  // Largest date using int32_t seconds from AceTime epoch
  pd = PlainDate::forComponents(2068, 1, 19);
  assertEqual((int32_t) 24855, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(24855));

  // Largest PlainDate in an 8-bit implementation
  pd = PlainDate::forComponents(2127, 12, 31);
  assertEqual((int32_t) 46750, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(46750));

  // Largest PlainDate in our 16-bit implementation is 9999-12-31
  pd = PlainDate::forComponents(9999, 12, 31);
  const int32_t epochDays9999 = (int32_t) ((10000 - 2000) / 400) * 146097 - 1;
  assertEqual(epochDays9999, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(epochDays9999));
}

// Change currentEpochYear to a different value.
test(PlainDateTest, toAndFromEpochDays_epoch2050) {
  // Change current epoch year to 2050, making the epoch 2100-01-01T00:00:00.
  testing::EpochYearContext context(2050);

  // Verify lower and upper valid year limits.
  assertEqual(Epoch::epochValidYearLower(), 2000);
  assertEqual(Epoch::epochValidYearUpper(), 2100);

  // Verify that 2050-01-01 returns epoch days of 0
  PlainDate pd = PlainDate::forComponents(2050, 1, 1);
  assertEqual((int32_t) 0, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(0));

  // Verify the smallest PlainDate. The smallest valid epochseconds is
  // (INT32_MIN+1) because INT32_MIN is a sentinel for an Error condition.
  // The complicated expression below is an integer division of a negative
  // number that truncates towards -Infinity.
  pd = PlainDate::forComponents(1981, 12, 13);
  int32_t smallestEpochDays = (PlainDate::kMinEpochSeconds + 1) / 86400 - 1;
  assertEqual(smallestEpochDays, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(smallestEpochDays));

  // Verify the largest PlainDate.
  pd = PlainDate::forComponents(2118, 1, 20);
  int32_t largestEpochDays = PlainDate::kMaxEpochSeconds / 86400;
  assertEqual(largestEpochDays, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(largestEpochDays));
}

// Change currentEpochYear to a different value.
test(PlainDateTest, toAndFromEpochDays_epoch2100) {
  // Change current epoch year to 2100, so the epoch becomes
  // 2100-01-01T00:00:00.
  testing::EpochYearContext context(2100);

  // Verify lower and upper valid year limits.
  assertEqual(Epoch::epochValidYearLower(), 2050);
  assertEqual(Epoch::epochValidYearUpper(), 2150);

  // Verify that 2100-01-01 returns epoch days of 0
  PlainDate pd = PlainDate::forComponents(2100, 1, 1);
  assertEqual((int32_t) 0, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(0));

  // Verify the smallest PlainDate. The smallest valid epochseconds is
  // (INT32_MIN+1) because INT32_MIN is a sentinel for an Error condition.
  // The complicated expression below is an integer division of a negative
  // number that truncates towards -Infinity.
  pd = PlainDate::forComponents(2031, 12, 13);
  int32_t smallestEpochDays = (PlainDate::kMinEpochSeconds + 1) / 86400 - 1;
  assertEqual(smallestEpochDays, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(smallestEpochDays));

  // Verify the largest PlainDate.
  pd = PlainDate::forComponents(2168, 1, 20);
  int32_t largestEpochDays = PlainDate::kMaxEpochSeconds / 86400;
  assertEqual(largestEpochDays, pd.toEpochDays());
  assertTrue(pd == PlainDate::forEpochDays(largestEpochDays));
}

// Same as toAndFromEpochDays, shifted 30 years
test(PlainDateTest, toAndFromUnixDays) {
  PlainDate pd;

  // Smallest PlainDate in an 8-bit implementation
  pd = PlainDate::forComponents(1873, 1, 1);
  assertEqual((int32_t) -35428, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(-35428));

  // Smallest date using int32_t from Unix epoch
  pd = PlainDate::forComponents(1901, 12, 14);
  assertEqual((int32_t) -24855, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(-24855));

  // Unix Epoch
  pd = PlainDate::forComponents(1970, 1, 1);
  assertEqual((int32_t) 0, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(0));

  // 1970 is not a leap year, whereas 2000 is a leap year
  pd = PlainDate::forComponents(1970, 2, 28);
  assertEqual((int32_t) 58, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(58));

  pd = PlainDate::forComponents(1988, 1, 1);
  assertEqual((int32_t) 6574, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(6574));

  // Largest date using int32_t from Unix epoch
  pd = PlainDate::forComponents(2038, 1, 19);
  assertEqual((int32_t) 24855, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(24855));

  // Largest PlainDate in an 8-bit implementation
  pd = PlainDate::forComponents(2127, 12, 31);
  assertEqual((int32_t) 57707, pd.toUnixDays());
  assertTrue(pd == PlainDate::forUnixDays(57707));
}

test(PlainDateTest, toAndFromEpochSeconds) {
  testing::EpochYearContext context(2000);
  PlainDate pd;

  // Smallest date with an int32_t seconds from AceTime Epoch is 1931-12-13
  // 20:45:52. The forEpochSeconds() will correctly truncate the partial day
  // *down* towards the to the nearest whole day.
  pd = PlainDate::forComponents(1931, 12, 13);
  assertTrue(pd == PlainDate::forEpochSeconds(INT32_MIN + 1));

  // The smallest whole day that can be represented with an int32_t seconds from
  // AceTime Epoch is 1931-12-14.
  pd = PlainDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -24855 * 86400, pd.toEpochSeconds());
  assertTrue(pd == PlainDate::forEpochSeconds((acetime_t) -24855 * 86400));

  pd = PlainDate::forComponents(2000, 1, 1);
  assertEqual((acetime_t) 0, pd.toEpochSeconds());
  assertTrue(pd == PlainDate::forEpochSeconds(0));

  pd = PlainDate::forComponents(2000, 2, 29);
  assertEqual((acetime_t) 59 * 86400, pd.toEpochSeconds());
  assertTrue(pd == PlainDate::forEpochSeconds((acetime_t) 59 * 86400 + 1));

  pd = PlainDate::forComponents(2018, 1, 1);
  assertEqual((acetime_t) 6575 * 86400, pd.toEpochSeconds());
  assertTrue(pd == PlainDate::forEpochSeconds((acetime_t) 6575 * 86400 + 2));

  // Largest date possible using AceTime Epoch Seconds is 2068-01-19 03:14:07.
  pd = PlainDate::forComponents(2068, 1, 19);
  assertEqual((acetime_t) 24855 * 86400, pd.toEpochSeconds());
  assertTrue(pd == PlainDate::forEpochSeconds(
      (acetime_t) 24855 * 86400 + 11647));
}

test(PlainDateTest, toAndFromUnixSeconds64) {
  testing::EpochYearContext context(2000);
  PlainDate pd;

  // Verify error sentinel.
  pd = PlainDate::forUnixSeconds64(PlainDate::kInvalidUnixSeconds64);
  assertTrue(pd.isError());

  // Verify that 64-bit unixSeconds allows dates beyond 32-bit limit.
  // 1770 to 1970 is 200 years == 200 * 365 + (200/4) leap years - 2 (1800, 1900
  // are not leap) == 73048 days
  pd = PlainDate::forComponents(1770, 1, 1);
  assertEqual((int64_t) -73048 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) -73048 * 86400));

  // The smallest whole day that can be represented with an int32_t from AceTime
  // epoch is 1931-12-14, can't do better with unixSeconds since it uses
  // the Acetime seconds internally.
  pd = PlainDate::forComponents(1931, 12, 14);
  assertEqual((int64_t) -13898 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) -13898 * 86400));

  pd = PlainDate::forComponents(1970, 1, 1);
  assertEqual((int64_t) 0, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64(0));

  // one second after should make no difference due to truncation
  pd = PlainDate::forComponents(1970, 1, 1);
  assertEqual((int64_t) 0, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64(0));

  pd = PlainDate::forComponents(1970, 2, 28);
  assertEqual((int64_t) 58 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) 58 * 86400));

  pd = PlainDate::forComponents(1988, 1, 1);
  assertEqual((int64_t) 6574 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) 6574 * 86400));

  // Largest date possible using Unix Seconds is 2038-01-19 03:14:07.
  pd = PlainDate::forComponents(2038, 1, 19);
  assertEqual((int64_t) 24855 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) INT32_MAX));

  // One day after the largest 32-bit dates should work in 64-bits.
  pd = PlainDate::forComponents(2038, 1, 20);
  assertEqual((int64_t) 24856 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) 24856 * 86400));

  // Verify that year 2068 works just fine with 64-bit unix seconds.
  pd = PlainDate::forComponents(2068, 1, 19);
  assertEqual((int64_t) 35812 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) 35812 * 86400));

  // Verify that year 2170 works just fine with 64-bit unix seconds.
  // 200 years = 73049 days, instead of 73048 days, because 2000 was a leap
  // year.
  pd = PlainDate::forComponents(2170, 1, 1);
  assertEqual((int64_t) 73049 * 86400, pd.toUnixSeconds64());
  assertTrue(pd == PlainDate::forUnixSeconds64((int64_t) 73049 * 86400 + 2));
}

test(PlainDateTest, compareTo) {
  PlainDate a, b;

  a = PlainDate::forComponents(2000, 1, 1);
  b = PlainDate::forComponents(2000, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = PlainDate::forComponents(2000, 1, 1);
  b = PlainDate::forComponents(2000, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = PlainDate::forComponents(2000, 1, 1);
  b = PlainDate::forComponents(2000, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = PlainDate::forComponents(2000, 1, 1);
  b = PlainDate::forComponents(2001, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(PlainDateTest, forDateString) {
  PlainDate pd;
  pd = PlainDate::forDateString("2000-01-01");
  assertTrue(pd == PlainDate::forComponents(2000, 1, 1));

  pd = PlainDate::forDateString("2099-02-28");
  assertTrue(pd == PlainDate::forComponents(2099, 2, 28));

  pd = PlainDate::forDateString("2127-12-31");
  assertTrue(pd == PlainDate::forComponents(2127, 12, 31));
}

test(PlainDateTest, forDateString_invalid) {
  PlainDate pd = PlainDate::forDateString("2000-01");
  assertTrue(pd.isError());
}

test(PlainDateTest, isLeapYear) {
  assertFalse(PlainDate::isLeapYear(1900));
  assertTrue(PlainDate::isLeapYear(2000));
  assertFalse(PlainDate::isLeapYear(2001));
  assertTrue(PlainDate::isLeapYear(2004));
  assertFalse(PlainDate::isLeapYear(2100));
}

test(PlainDateTest, daysInMonth) {
  assertEqual(31, PlainDate::daysInMonth(2000, 1));
  assertEqual(29, PlainDate::daysInMonth(2000, 2));
  assertEqual(31, PlainDate::daysInMonth(2000, 3));
  assertEqual(30, PlainDate::daysInMonth(2000, 4));
  assertEqual(31, PlainDate::daysInMonth(2000, 5));
  assertEqual(30, PlainDate::daysInMonth(2000, 6));
  assertEqual(31, PlainDate::daysInMonth(2000, 7));
  assertEqual(31, PlainDate::daysInMonth(2000, 8));
  assertEqual(30, PlainDate::daysInMonth(2000, 9));
  assertEqual(31, PlainDate::daysInMonth(2000, 10));
  assertEqual(30, PlainDate::daysInMonth(2000, 11));
  assertEqual(31, PlainDate::daysInMonth(2000, 12));

  assertEqual(28, PlainDate::daysInMonth(2001, 2));
  assertEqual(29, PlainDate::daysInMonth(2004, 2));
  assertEqual(28, PlainDate::daysInMonth(2100, 2));
}

test(PlainDateTest, daysUntil) {
  PlainDate today = PlainDate::forComponents(2000, 12, 25);
  assertEqual(0, today.daysUntil(12, 25));

  today = PlainDate::forComponents(2000, 12, 24);
  assertEqual(1, today.daysUntil(12, 25));

  // 2001 is a normal year, so 364 days until next Christmas
  today = PlainDate::forComponents(2000, 12, 26);
  assertEqual(364, today.daysUntil(12, 25));

  // 2004 is a leap year so 365 days until next Christmas
  today = PlainDate::forComponents(2003, 12, 26);
  assertEqual(365, today.daysUntil(12, 25));
}

//---------------------------------------------------------------------------

test(PlainDateTest, incrementOneDay) {
  PlainDate pd;

  pd = PlainDate::forComponents(2000, 2, 28);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 2, 29));

  pd = PlainDate::forComponents(2000, 2, 29);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 3, 1));

  pd = PlainDate::forComponents(2000, 3, 31);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 4, 1));

  pd = PlainDate::forComponents(2000, 12, 31);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2001, 1, 1));

  pd = PlainDate::forComponents(2001, 2, 28);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2001, 3, 1));

  pd = PlainDate::forComponents(2004, 2, 28);
  plain_date_mutation::incrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2004, 2, 29));
}

test(PlainDateTest, decrementOneDay) {
  PlainDate pd;

  pd = PlainDate::forComponents(2004, 2, 29);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2004, 2, 28));

  pd = PlainDate::forComponents(2001, 3, 1);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2001, 2, 28));

  pd = PlainDate::forComponents(2001, 1, 1);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 12, 31));

  pd = PlainDate::forComponents(2000, 4, 1);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 3, 31));

  pd = PlainDate::forComponents(2000, 3, 1);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 2, 29));

  pd = PlainDate::forComponents(2000, 2, 29);
  plain_date_mutation::decrementOneDay(pd);
  assertTrue(pd == PlainDate::forComponents(2000, 2, 28));
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
