#line 2 "LocalDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// LocalDate
// --------------------------------------------------------------------------

test(localDateAccessors) {
  LocalDate ld = LocalDate::forComponents(1, 2, 3);
  assertEqual(1, ld.year());
  assertEqual(2, ld.month());
  assertEqual(3, ld.day());
}

test(localDateSetError) {
  LocalDate ld = LocalDate().setError();
  assertTrue(ld.isError());
}

test(localDateForDateString) {
  LocalDate ld;
  ld = LocalDate::forDateString("2000-01-01");
  assertTrue(ld == LocalDate::forComponents(0, 1, 1));

  ld = LocalDate::forDateString("2099-02-28");
  assertTrue(ld == LocalDate::forComponents(99, 2, 28));
}

test(localDateDayOfWeek) {
  // year 2000 (leap year due to every 400 rule)
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 1, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 2, 29).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 3, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 3, 31).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 4, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 4, 30).dayOfWeek());

  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 5, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 5, 31).dayOfWeek());

  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 6, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 6, 30).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 7, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 7, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 8, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 8, 31).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 9, 1).dayOfWeek());
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 9, 30).dayOfWeek());

  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 10, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 10, 31).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 11, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 11, 30).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 12, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 12, 31).dayOfWeek());

  // year 2001
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(1, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(1, 1, 31).dayOfWeek());

  // year 2004 (leap year)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(4, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(4, 2, 29).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(4, 3, 1).dayOfWeek());

  // year 2099
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(99, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(99, 12, 31).dayOfWeek());

  // year 2100 (not leap year due to every 100 rule)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(100, 2, 28).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(100, 3, 1).dayOfWeek());
}

test(localDateToAndFromEpochDays) {
  LocalDate ld;
  
  ld = LocalDate::forComponents(0, 1, 1);
  assertEqual((uint32_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  ld = LocalDate::forComponents(0, 2, 29);
  assertEqual((uint32_t) 59, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(59));

  ld = LocalDate::forComponents(18, 1, 1);
  assertEqual((uint32_t) 6575, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(6575));

  ld = LocalDate::forComponents(49, 12, 31);
  assertEqual((uint32_t) 18262, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(18262));

  ld = LocalDate::forComponents(99, 12, 31);
  assertEqual((uint32_t) 36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(36524));
}

test(localDateCompareTo) {
  LocalDate a, b;

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(1, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertTrue(a != b);
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
