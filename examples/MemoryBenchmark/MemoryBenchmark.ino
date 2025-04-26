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
#define FEATURE_BASIC_ZONE_MANAGER_ZONES_AND_LINKS 8
#define FEATURE_BASIC_ZONE_SORTER_BY_NAME 9
#define FEATURE_BASIC_ZONE_SORTER_BY_OFFSET_AND_NAME 10
#define FEATURE_EXTENDED_TIME_ZONE 11
#define FEATURE_EXTENDED_TIME_ZONE2 12
#define FEATURE_EXTENDED_ZONE_MANAGER_ONE 13
#define FEATURE_EXTENDED_ZONE_MANAGER_ZONES 14
#define FEATURE_EXTENDED_ZONE_MANAGER_ZONES_AND_LINKS 15
#define FEATURE_EXTENDED_ZONE_SORTER_BY_NAME 16
#define FEATURE_EXTENDED_ZONE_SORTER_BY_OFFSET_AND_NAME 17
#define FEATURE_COMPLETE_TIME_ZONE 18
#define FEATURE_COMPLETE_TIME_ZONE2 19
#define FEATURE_COMPLETE_ZONE_MANAGER_ONE 20
#define FEATURE_COMPLETE_ZONE_MANAGER_ZONES 21
#define FEATURE_COMPLETE_ZONE_MANAGER_ZONES_AND_LINKS 22
#define FEATURE_COMPLETE_ZONE_SORTER_BY_NAME 23
#define FEATURE_COMPLETE_ZONE_SORTER_BY_OFFSET_AND_NAME 24

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

// Create the various objects that we want to measure as global variables so
// that their static memory consumption is detected. The previous version placed
// all these inside the setup() method, which creates the objects on the stack,
// which do not get detected as memory consumption, so don't show up in the
// *.txt files.
#if FEATURE == FEATURE_LOCAL_DATE_TIME
  auto dt = LocalDateTime::forComponents(year, 6, 17, 9, 18, 0);
#elif FEATURE == FEATURE_ZONED_DATE_TIME
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, TimeZone());
#elif FEATURE == FEATURE_MANUAL_ZONE_MANAGER
  ManualZoneManager manager;

#elif FEATURE == FEATURE_BASIC_TIME_ZONE
  BasicZoneProcessor processor;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE2
  // Same as FEATURE_BASIC_TIME_ZONE but with 2 zones
  BasicZoneProcessor processor1;
  BasicZoneProcessor processor2;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ONE
  static const basic::Info::ZoneInfo* const kBasicZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedb::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kBasicZoneRegistrySize =
      sizeof(kBasicZoneRegistry) / sizeof(basic::Info::ZoneInfo*);
  BasicZoneProcessorCache<1> zoneProcessorCache;
  BasicZoneManager manager(
      kBasicZoneRegistrySize,
      kBasicZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES
  BasicZoneProcessorCache<1> zoneProcessorCache;
  BasicZoneManager manager(
      zonedb::kZoneRegistrySize,
      zonedb::kZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES_AND_LINKS
  BasicZoneProcessorCache<1> zoneProcessorCache;
  BasicZoneManager manager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry,
    zoneProcessorCache);
#elif FEATURE == FEATURE_BASIC_ZONE_SORTER_BY_NAME
  // Construct the same BasicZoneManager as FEATURE_BASIC_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByName.
  static const basic::Info::ZoneInfo* const kBasicZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedb::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kBasicZoneRegistrySize =
      sizeof(kBasicZoneRegistry) / sizeof(basic::Info::ZoneInfo*);
  BasicZoneProcessorCache<1> zoneProcessorCache;
  BasicZoneManager manager(
      kBasicZoneRegistrySize,
      kBasicZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByName<BasicZoneManager> zoneSorter(manager);
#elif FEATURE == FEATURE_BASIC_ZONE_SORTER_BY_OFFSET_AND_NAME
  // Construct the same BasicZoneManager as FEATURE_BASIC_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByOffsetAndName.
  static const basic::Info::ZoneInfo* const kBasicZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedb::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kBasicZoneRegistrySize =
      sizeof(kBasicZoneRegistry) / sizeof(basic::Info::ZoneInfo*);
  BasicZoneProcessorCache<1> zoneProcessorCache;
  BasicZoneManager manager(
      kBasicZoneRegistrySize,
      kBasicZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByOffsetAndName<BasicZoneManager> zoneSorter(manager);

#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE
  ExtendedZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &processor);
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE2
  // Same as FEATURE_EXTENDED_TIME_ZONE but with 2 zones
  ExtendedZoneProcessor processor1;
  ExtendedZoneProcessor processor2;
  auto tz1 = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &processor1);
  auto tz2 = TimeZone::forZoneInfo(&zonedbx::kZoneEurope_Amsterdam,
      &processor2);
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ONE
  static const extended::Info::ZoneInfo* const kExtendedZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbx::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kExtendedZoneRegistrySize =
      sizeof(kExtendedZoneRegistry) / sizeof(extended::Info::ZoneInfo*);
  ExtendedZoneProcessorCache<1> zoneProcessorCache;
  ExtendedZoneManager manager(
      kExtendedZoneRegistrySize,
      kExtendedZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES
  ExtendedZoneProcessorCache<1> zoneProcessorCache;
  ExtendedZoneManager manager(
      zonedbx::kZoneRegistrySize,
      zonedbx::kZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES_AND_LINKS
  ExtendedZoneProcessorCache<1> zoneProcessorCache;
  ExtendedZoneManager manager(
      zonedbx::kZoneAndLinkRegistrySize,
      zonedbx::kZoneAndLinkRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_EXTENDED_ZONE_SORTER_BY_NAME
  // Construct the same ExtendedZoneManager as FEATURE_EXTENDED_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByName.
  static const extended::Info::ZoneInfo* const kExtendedZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbx::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kExtendedZoneRegistrySize =
      sizeof(kExtendedZoneRegistry) / sizeof(extended::Info::ZoneInfo*);
  ExtendedZoneProcessorCache<1> zoneProcessorCache;
  ExtendedZoneManager manager(
      kExtendedZoneRegistrySize,
      kExtendedZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByName<ExtendedZoneManager> zoneSorter(manager);
#elif FEATURE == FEATURE_EXTENDED_ZONE_SORTER_BY_OFFSET_AND_NAME
  // Construct the same ExtendedZoneManager as FEATURE_EXTENDED_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByOffsetAndName.
  static const extended::Info::ZoneInfo* const kExtendedZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbx::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kExtendedZoneRegistrySize =
      sizeof(kExtendedZoneRegistry) / sizeof(extended::Info::ZoneInfo*);
  ExtendedZoneProcessorCache<1> zoneProcessorCache;
  ExtendedZoneManager manager(
      kExtendedZoneRegistrySize,
      kExtendedZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByOffsetAndName<ExtendedZoneManager> zoneSorter(manager);

#elif FEATURE == FEATURE_COMPLETE_TIME_ZONE
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  CompleteZoneProcessor processor;
  auto tz = TimeZone::forZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles,
      &processor);
#elif FEATURE == FEATURE_COMPLETE_TIME_ZONE2
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  // Same as FEATURE_COMPLETE_TIME_ZONE but with 2 zones
  CompleteZoneProcessor processor1;
  CompleteZoneProcessor processor2;
  auto tz1 = TimeZone::forZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles,
      &processor1);
  auto tz2 = TimeZone::forZoneInfo(&zonedbc::kZoneEurope_Amsterdam,
      &processor2);
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ONE
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  static const complete::Info::ZoneInfo* const kCompleteZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbc::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kCompleteZoneRegistrySize =
      sizeof(kCompleteZoneRegistry) / sizeof(complete::Info::ZoneInfo*);
  CompleteZoneProcessorCache<1> zoneProcessorCache;
  CompleteZoneManager manager(
      kCompleteZoneRegistrySize,
      kCompleteZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ZONES
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  CompleteZoneProcessorCache<1> zoneProcessorCache;
  CompleteZoneManager manager(
      zonedbc::kZoneRegistrySize,
      zonedbc::kZoneRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ZONES_AND_LINKS
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  CompleteZoneProcessorCache<1> zoneProcessorCache;
  CompleteZoneManager manager(
      zonedbc::kZoneAndLinkRegistrySize,
      zonedbc::kZoneAndLinkRegistry,
      zoneProcessorCache);
#elif FEATURE == FEATURE_COMPLETE_ZONE_SORTER_BY_NAME
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  // Construct the same CompleteZoneManager as FEATURE_COMPLETE_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByName.
  static const complete::Info::ZoneInfo* const kCompleteZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbc::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kCompleteZoneRegistrySize =
      sizeof(kCompleteZoneRegistry) / sizeof(complete::Info::ZoneInfo*);
  CompleteZoneProcessorCache<1> zoneProcessorCache;
  CompleteZoneManager manager(
      kCompleteZoneRegistrySize,
      kCompleteZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByName<CompleteZoneManager> zoneSorter(manager);
#elif FEATURE == FEATURE_COMPLETE_ZONE_SORTER_BY_OFFSET_AND_NAME
  #if defined(ARDUINO_ARCH_AVR)
    #error Unsupported FEATURE on this platform
  #endif
  // Construct the same CompleteZoneManager as FEATURE_COMPLETE_TIME_ZONE, then
  // subtract its memory consumption numbers to isolate just the
  // ZoneSorterByOffsetAndName.
  static const complete::Info::ZoneInfo* const kCompleteZoneRegistry[]
      ACE_TIME_PROGMEM = {
    &zonedbc::kZoneAmerica_Los_Angeles,
  };
  static const uint16_t kCompleteZoneRegistrySize =
      sizeof(kCompleteZoneRegistry) / sizeof(complete::Info::ZoneInfo*);
  CompleteZoneProcessorCache<1> zoneProcessorCache;
  CompleteZoneManager manager(
      kCompleteZoneRegistrySize,
      kCompleteZoneRegistry,
      zoneProcessorCache);
  ZoneSorterByOffsetAndName<CompleteZoneManager> zoneSorter(manager);

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
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_ZONED_DATE_TIME
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_MANUAL_ZONE_MANAGER
  TimeZoneData tzd = { -8*60 /*stdMinutes*/, 60 /*dstMinutes*/ };
  auto tz = manager.createForTimeZoneData(tzd);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;

#elif FEATURE == FEATURE_BASIC_TIME_ZONE
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_TIME_ZONE2
  // Same as FEATURE_BASIC_TIME_ZONE but with 2 zones
  auto tz1 = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &processor1);
  auto tz2 = TimeZone::forZoneInfo(&zonedb::kZoneEurope_Amsterdam,
      &processor2);
  auto dt1 = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ONE
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_MANAGER_ZONES_AND_LINKS
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_BASIC_ZONE_SORTER_BY_NAME
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];
#elif FEATURE == FEATURE_BASIC_ZONE_SORTER_BY_OFFSET_AND_NAME
  auto tz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];

#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_TIME_ZONE2
  auto dt1 = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ONE
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_MANAGER_ZONES_AND_LINKS
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_EXTENDED_ZONE_SORTER_BY_NAME
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];
#elif FEATURE == FEATURE_EXTENDED_ZONE_SORTER_BY_OFFSET_AND_NAME
  auto tz = manager.createForZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];

#elif FEATURE == FEATURE_COMPLETE_TIME_ZONE
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_COMPLETE_TIME_ZONE2
  auto dt1 = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz1);
  auto dt2 = dt1.convertToTimeZone(tz2);
  acetime_t epochSeconds = dt2.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ONE
  auto tz = manager.createForZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ZONES
  auto tz = manager.createForZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_COMPLETE_ZONE_MANAGER_ZONES_AND_LINKS
  auto tz = manager.createForZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
#elif FEATURE == FEATURE_COMPLETE_ZONE_SORTER_BY_NAME
  auto tz = manager.createForZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];
#elif FEATURE == FEATURE_COMPLETE_ZONE_SORTER_BY_OFFSET_AND_NAME
  auto tz = manager.createForZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles);
  auto dt = ZonedDateTime::forComponents(year, 6, 17, 9, 18, 0, tz);
  acetime_t epochSeconds = dt.toEpochSeconds();
  guard ^= epochSeconds;
  uint16_t indexes[2] = {0, 1};
  zoneSorter.sortIndexes(indexes, 2);
  guard ^= indexes[0];

#else
  #error Unknown FEATURE
#endif
}

void loop() {
}
