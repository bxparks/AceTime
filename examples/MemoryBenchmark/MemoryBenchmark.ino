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
// NOTE: This line is modified by a 'sed' script in collect.sh. Be careful
// when modifying its format.
#define FEATURE 0

#if FEATURE != FEATURE_BASELINE
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

static const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_New_York,
};

static const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(kBasicZoneRegistry[0]);

static const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_New_York,
};

static const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(kExtendedZoneRegistry[0]);

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
  BasicZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE2
  // Same as FEATURE_BASIC_TIME_ZONE but with 2 zones
  BasicZoneProcessor processor1;
  BasicZoneProcessor processor2;
  auto tz1 = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor1);
  auto tz2 = TimeZone::forZoneInfo(&zonedb::kZoneEurope_Amsterdam,
      &processor2);
  auto dt1 = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE_ALL
  BasicZoneManager<1> manager(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE
  ExtendedZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE2
  // Same as FEATURE_EXTENDED_TIME_ZONE but with 2 zones
  ExtendedZoneProcessor processor1;
  ExtendedZoneProcessor processor2;
  auto tz1 = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &processor1);
  auto tz2 = TimeZone::forZoneInfo(&zonedbx::kZoneEurope_Amsterdam,
      &processor2);
  auto dt1 = ZonedDateTime::forComponents(2019, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE_ALL
  ExtendedZoneManager<1> manager(
      zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
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
  BasicZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forEpochSeconds(now, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#else
  #error Unknown FEATURE
#endif
}

void loop() {
}
