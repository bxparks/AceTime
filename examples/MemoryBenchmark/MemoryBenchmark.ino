/*
 * Determine the size of various components of the AceTime library.
 */

// List of features of the AceTime library that we want to examine.
#define FEATURE_BASELINE 0
#define FEATURE_LOCAL_DATE_TIME 1
#define FEATURE_ZONED_DATE_TIME 2
#define FEATURE_BASIC_TIME_ZONE 3
#define FEATURE_BASIC_TIME_ZONE2 4
#define FEATURE_BASIC_TIME_ZONE_ALL 5
#define FEATURE_EXTENDED_TIME_ZONE 6
#define FEATURE_EXTENDED_TIME_ZONE2 7
#define FEATURE_EXTENDED_TIME_ZONE_ALL 8
#define FEATURE_SYSTEM_CLOCK 9
#define FEATURE_SYSTEM_CLOCK_AND_BASIC_TIME_ZONE 10

// Select one of the FEATURE_* parameter and compile. Then look at the flash
// and RAM usage, compared to FEATURE_BASELINE usage to determine how much
// flash and RAM is consumed by the selected feature.
#define FEATURE 0

#if FEATURE != FEATURE_BASELINE
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
#endif

// Set this variable to prevent the compiler optimizer from removing the code
// being tested when it determines that it does nothing.
volatile uint8_t guard;

void setup() {
#if FEATURE == FEATURE_BASELINE
  guard = 0;
#elif FEATURE == FEATURE_LOCAL_DATE_TIME
  auto dt = LocalDateTime::forComponents(2019, 6, 17, 9, 18, 0);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_ZONED_DATE_TIME
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, TimeZone());
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE
  BasicZoneSpecifier zspec(&zonedb::kZoneAmerica_Los_Angeles);
  auto tz = TimeZone::forZoneSpecifier(&zspec);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE2
  // Same as FEATURE_BASIC_TIME_ZONE but with 2 zones
  BasicZoneSpecifier zspec1(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneSpecifier zspec2(&zonedb::kZoneEurope_Amsterdam);
  auto tz1 = TimeZone::forZoneSpecifier(&zspec1);
  auto tz2 = TimeZone::forZoneSpecifier(&zspec2);
  auto dt1 = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE_ALL
  BasicZoneManager manager(zonedb::kZoneRegistry, zonedb::kZoneRegistrySize);
  const basic::ZoneInfo* zoneInfo =
      manager.getZoneInfo("America/Los_Angeles");
  BasicZoneSpecifier zspec(zoneInfo);
  auto tz = TimeZone::forZoneSpecifier(&zspec);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE
  ExtendedZoneSpecifier zspec(&zonedbx::kZoneAmerica_Los_Angeles);
  auto tz = TimeZone::forZoneSpecifier(&zspec);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE2
  // Same as FEATURE_EXTENDED_TIME_ZONE but with 2 zones
  ExtendedZoneSpecifier zspec1(&zonedbx::kZoneAmerica_Los_Angeles);
  ExtendedZoneSpecifier zspec2(&zonedbx::kZoneEurope_Amsterdam);
  auto tz1 = TimeZone::forZoneSpecifier(&zspec1);
  auto tz2 = TimeZone::forZoneSpecifier(&zspec2);
  auto dt1 = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE_ALL
  ExtendedZoneManager manager(zonedbx::kZoneRegistry,
      zonedbx::kZoneRegistrySize);
  const extended::ZoneInfo* zoneInfo =
      manager.getZoneInfo("America/Los_Angeles");
  ExtendedZoneSpecifier zspec(zoneInfo);
  auto tz = TimeZone::forZoneSpecifier(&zspec);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_SYSTEM_CLOCK
  DS3231TimeKeeper dsTimeKeeper;
  SystemClock systemClock(&dsTimeKeeper, &dsTimeKeeper);
  systemClock.setup();
  acetime_t now = systemClock.getNow();
  guard ^= now;
#elif FEATURE == FEATURE_SYSTEM_CLOCK_AND_BASIC_TIME_ZONE
  DS3231TimeKeeper dsTimeKeeper;
  SystemClock systemClock(&dsTimeKeeper, &dsTimeKeeper);
  systemClock.setup();
  acetime_t now = systemClock.getNow();
  BasicZoneSpecifier zspec(&zonedb::kZoneAmerica_Los_Angeles);
  auto tz = TimeZone::forZoneSpecifier(&zspec);
  auto dt = ZonedDateTime::forEpochSeconds(now, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#else
  #error Unknown FEATURE
#endif
}

void loop() {
}
