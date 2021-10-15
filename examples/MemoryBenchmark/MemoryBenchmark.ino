/*
 * Determine the size of various components of the AceTime library.
 */

#include <stdint.h> // uint8_t
#include <Arduino.h>

// List of features of the AceTime library that we want to examine.
#define FEATURE_BASELINE 0
#define FEATURE_LOCAL_DATE_TIME 1
#define FEATURE_ZONED_DATE_TIME 2
#define FEATURE_MANUAL_ZONE_MANAGER 3
#define FEATURE_BASIC_TIME_ZONE 4
#define FEATURE_BASIC_TIME_ZONE2 5
#define FEATURE_BASIC_ZONE_MANAGER_ONE 6
#define FEATURE_BASIC_ZONE_MANAGER_ZONES 7
#define FEATURE_BASIC_ZONE_MANAGER_ZONES_AND_FAT_LINKS 8
#define FEATURE_BASIC_LINK_MANAGER 9
#define FEATURE_EXTENDED_TIME_ZONE 10
#define FEATURE_EXTENDED_TIME_ZONE2 11
#define FEATURE_EXTENDED_ZONE_MANAGER_ONE 12
#define FEATURE_EXTENDED_ZONE_MANAGER_ZONES 13
#define FEATURE_EXTENDED_ZONE_MANAGER_ZONES_AND_FAT_LINKS 14
#define FEATURE_EXTENDED_LINK_MANAGER 15

// Select one of the FEATURE_* parameter and compile. Then look at the flash
// and RAM usage, compared to FEATURE_BASELINE usage to determine how much
// flash and RAM is consumed by the selected feature.
// NOTE: This line is modified by a 'sed' script in collect.sh. Be careful
// when modifying its format.
#define FEATURE 0

#if FEATURE != FEATURE_BASELINE
  #include <AceTime.h>
  using namespace ace_time;
#endif

// Set this variable to prevent the compiler optimizer from removing the code
// being tested when it determines that it does nothing.
volatile int guard;

// Use this instead of a constant to prevent the compiler from calculating
// certain values (e.g. toEpochSeconds()) at compile-time.
volatile int16_t year = 2019;

#if FEATURE == FEATURE_BASIC_ZONE_MANAGER_ONE

static const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
};
static const uint16_t kBasicZoneRegistrySize =
    sizeof(kBasicZoneRegistry) / sizeof(basic::ZoneInfo*);

#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ONE

static const extended::ZoneInfo* const kExtendedZoneRegistry[]
    ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
};
static const uint16_t kExtendedZoneRegistrySize =
    sizeof(kExtendedZoneRegistry) / sizeof(extended::ZoneInfo*);

#endif

// TeensyDuino seems to pull in malloc() and free() when a class with virtual
// functions is used polymorphically. This causes the memory consumption of
// FEATURE_BASELINE (which normally has no classes defined, so does not include
// malloc() and free()) to be artificially small which throws off the memory
// consumption calculations for all subsequent features. Let's define a
// throw-away class and call its method for all FEATURES, including BASELINE.
#if defined(TEENSYDUINO)
  class FooClass {
    public:
      virtual void doit() {
        guard = 0;
      }
  };

  FooClass* foo;
#endif

void setup() {
#if defined(TEENSYDUINO)
  // Force Teensy to bring in malloc(), free() and other things for virtual
  // dispatch.
  foo = new FooClass();
#endif

#if FEATURE == FEATURE_BASELINE
  guard = 0;
#elif FEATURE == FEATURE_LOCAL_DATE_TIME
  auto dt = LocalDateTime::forComponents(year, 6, 17, 9, 18, 0);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_ZONED_DATE_TIME
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, TimeZone());
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_MANUAL_ZONE_MANAGER
  TimeZoneData tzd = { -8*60 /*stdMinutes*/, 60 /*dstMinutes*/ };
  ManualZoneManager manager;
  auto tz = manager.createForTimeZoneData(tzd);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE
  BasicZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
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
  auto dt1 = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ONE
  BasicZoneManager<1> manager(kBasicZoneRegistrySize, kBasicZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES
  BasicZoneManager<1> manager(
      zonedb::kZoneRegistrySize,
      zonedb::kZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_LINK_MANAGER
  BasicLinkManager manager(
    zonedb::kLinkRegistrySize,
    zonedb::kLinkRegistry);
  uint32_t zoneId = manager.zoneIdForLinkId(zonedb::kZoneIdUS_Pacific);
  guard ^= zoneId;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES_AND_FAT_LINKS
  BasicZoneManager<1> manager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry);
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE
  ExtendedZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
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
  auto dt1 = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ONE
  ExtendedZoneManager<1> manager(
      kExtendedZoneRegistrySize, kExtendedZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES
  ExtendedZoneManager<1> manager(
      zonedbx::kZoneRegistrySize,
      zonedbx::kZoneRegistry);
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES_AND_FAT_LINKS
  ExtendedZoneManager<1> manager(
      zonedbx::kZoneAndLinkRegistrySize,
      zonedbx::kZoneAndLinkRegistry);
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_LINK_MANAGER
  ExtendedLinkManager manager(
      zonedbx::kLinkRegistrySize,
      zonedbx::kLinkRegistry);
  uint32_t zoneId = manager.zoneIdForLinkId(zonedbx::kZoneIdUS_Pacific);
  guard ^= zoneId;

#else
  #error Unknown FEATURE
#endif
}

void loop() {
}
