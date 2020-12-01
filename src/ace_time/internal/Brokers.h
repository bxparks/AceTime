/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_H
#define ACE_TIME_BROKERS_H

/**
 * @file Brokers.h
 *
 * The classes provide a thin layer of indirection for accessing the
 * zoneinfo files stored in the zonedb/ and zonedbx/ directories. When
 * ACE_TIME_USE_PROGMEM or ACE_TIME_USE_PROGMEM are enabled, the
 * zoneinfo files are stored in flash memory (using the PROGMEM keyword), and
 * cannot be accessed directly on microcontrollers using the Harvard
 * architecture (e.g. AVR) where data and program live in 2 different address
 * spaces. The data in flash memory must be accessed using helper routines in
 * <pgmspace.h>. These classes abstract away this difference so that the code
 * BasicZoneProcessor and ExtendedZoneProcessor can be written to be (mostly)
 * agnostic to how the zoneinfo files are stored.
 *
 * When ACE_TIME_USE_PROGMEM are disabled, the compiler will optimize away this
 * entire abstraction layer, so the resulting machine code is no bigger than
 * (and in most cases, identifical to) accessing the zoneinfo files directly.
 *
 * The abstraction layer is thin enough that the code in BasicZoneProcessor and
 * ExtendedZoneProcessor did not change very much. It was mostly a mechanical
 * source code replacement of direct zoneinfo access to using these data
 * brokers.
 *
 * The helper functions live in the internal:: namespace. The classes are
 * somewhat duplicated between the 'basic' and 'extended' namespaces. They used
 * to be identical so that they could be templatized. But supporting one-minute
 * resolution for 'extended' meant that the implementations diverged, so I had
 * to manual duplicate the classes.
 */

#include "../common/compat.h"
#include "ZoneInfo.h"

namespace ace_time {

namespace internal {

/** Convert (timeCode, timeModifier) fields in zoneinfo to minutes. */
inline uint16_t timeCodeToMinutes(uint8_t code, uint8_t modifier) {
  return code * (uint16_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU.
 */
inline uint8_t toSuffix(uint8_t modifier) {
  return modifier & 0xf0;
}

} // internal

//------------------------------------------------------------------------

namespace basic {

/** Data broker for accessing ZoneRule. */
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(const ZoneRule* zoneRule):
        mZoneRule(zoneRule) {}

    ZoneRuleBroker():
        mZoneRule(nullptr) {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    int8_t fromYearTiny() const {
      return pgm_read_byte(&mZoneRule->fromYearTiny);
    }

    int8_t toYearTiny() const {
      return pgm_read_byte(&mZoneRule->toYearTiny);
    }

    uint8_t inMonth() const {
      return pgm_read_byte(&mZoneRule->inMonth);
    }

    int8_t onDayOfWeek() const {
      return pgm_read_byte(&mZoneRule->onDayOfWeek);
    }

    int8_t onDayOfMonth() const {
      return pgm_read_byte(&mZoneRule->onDayOfMonth);
    }

    uint16_t atTimeMinutes() const {
      return internal::timeCodeToMinutes(
          pgm_read_byte(&mZoneRule->atTimeCode),
          pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    uint8_t atTimeSuffix() const {
      return internal::toSuffix(pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    int16_t deltaMinutes() const {
      return 15 * (int8_t) pgm_read_byte(&mZoneRule->deltaCode);
    }

    uint8_t letter() const {
      return pgm_read_byte(&mZoneRule->letter);
    }

  #else

    int8_t fromYearTiny() const { return mZoneRule->fromYearTiny; }

    int8_t toYearTiny() const { return mZoneRule->toYearTiny; }

    uint8_t inMonth() const { return mZoneRule->inMonth; }

    uint8_t onDayOfWeek() const { return mZoneRule->onDayOfWeek; }

    int8_t onDayOfMonth() const { return mZoneRule->onDayOfMonth; }

    uint16_t atTimeMinutes() const {
      return internal::timeCodeToMinutes(
          mZoneRule->atTimeCode, mZoneRule->atTimeModifier);
    }

    uint8_t atTimeSuffix() const {
      return internal::toSuffix(mZoneRule->atTimeModifier);
    }

    int16_t deltaMinutes() const { return 15 * mZoneRule->deltaCode; }

    uint8_t letter() const { return mZoneRule->letter; }

  #endif

  private:
    const ZoneRule* mZoneRule;
};

/** Data broker for accessing ZonePolicy. */
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(const ZonePolicy* zonePolicy):
        mZonePolicy(zonePolicy) {}

    ZonePolicyBroker():
        mZonePolicy(nullptr) {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const ZoneRuleBroker rule(uint8_t i) const {
      const ZoneRule* rules =
          (const ZoneRule*) pgm_read_ptr(&mZonePolicy->rules);
      return ZoneRuleBroker(&rules[i]);
    }

    uint8_t numLetters() const {
      return pgm_read_byte(&mZonePolicy->numLetters);
    }

    const char* letter(uint8_t i) const {
      const char* const* letters = (const char* const*)
          pgm_read_ptr(&mZonePolicy->letters);
      return (const char*) pgm_read_ptr(&letters[i]);
    }

  #else

    uint8_t numRules() const { return mZonePolicy->numRules; }

    const ZoneRuleBroker rule(uint8_t i) const {
      return ZoneRuleBroker(&mZonePolicy->rules[i]);
    }

    uint8_t numLetters() const { return mZonePolicy->numLetters; }

    const char* letter(uint8_t i) const {
      return mZonePolicy->letters[i];
    }

  #endif

  private:
    const ZonePolicy* mZonePolicy;
};

/** Data broker for accessing ZoneEra. */
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(const ZoneEra* zoneEra):
        mZoneEra(zoneEra) {}

    ZoneEraBroker():
        mZoneEra(nullptr) {}

    // use default copy constructor
    ZoneEraBroker(const ZoneEraBroker&) = default;

    // use default assignment operator
    ZoneEraBroker& operator=(const ZoneEraBroker&) = default;

    const ZoneEra* zoneEra() const { return mZoneEra; }

    bool isNull() const { return mZoneEra == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    int16_t offsetMinutes() const {
      return 15 * (int8_t) pgm_read_byte(&mZoneEra->offsetCode);
    }

    const ZonePolicyBroker zonePolicy() const {
      return ZonePolicyBroker(
          (const ZonePolicy*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int16_t deltaMinutes() const {
      return 15 * (int8_t) pgm_read_byte(&mZoneEra->deltaCode);
    }

    const char* format() const {
      return (const char*) pgm_read_ptr(&mZoneEra->format);
    }

    int8_t untilYearTiny() const {
      return pgm_read_byte(&mZoneEra->untilYearTiny);
    }

    uint8_t untilMonth() const {
      return pgm_read_byte(&mZoneEra->untilMonth);
    }

    uint8_t untilDay() const {
      return pgm_read_byte(&mZoneEra->untilDay);
    }

    uint16_t untilTimeMinutes() const {
      return internal::timeCodeToMinutes(
        pgm_read_byte(&mZoneEra->untilTimeCode),
        pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

    uint8_t untilTimeSuffix() const {
      return internal::toSuffix(pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

  #else

    int16_t offsetMinutes() const { return 15 * mZoneEra->offsetCode; }

    const ZonePolicyBroker zonePolicy() const {
      return ZonePolicyBroker(mZoneEra->zonePolicy);
    }

    int16_t deltaMinutes() const { return 15 * mZoneEra->deltaCode; }

    const char* format() const { return mZoneEra->format; }

    int8_t untilYearTiny() const { return mZoneEra->untilYearTiny; }

    uint8_t untilMonth() const { return mZoneEra->untilMonth; }

    uint8_t untilDay() const { return mZoneEra->untilDay; }

    uint16_t untilTimeMinutes() const {
      return internal::timeCodeToMinutes(
          mZoneEra->untilTimeCode, mZoneEra->untilTimeModifier);
    }

    uint8_t untilTimeSuffix() const {
      return internal::toSuffix(mZoneEra->untilTimeModifier);
    }

  #endif

  private:
    const ZoneEra* mZoneEra;
};

/** Data broker for accessing ZoneInfo. */
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = default;

    // use default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = default;

    const ZoneInfo* zoneInfo() const { return mZoneInfo; }

  #if ACE_TIME_USE_PROGMEM

    const char* name() const {
      return (const char*) pgm_read_ptr(&mZoneInfo->name);
    }

    uint32_t zoneId() const {
      return pgm_read_dword(&mZoneInfo->zoneId);
    }

    int16_t startYear() const {
      const ZoneContext* zoneContext =
          (const ZoneContext*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->startYear;
    }

    int16_t untilYear() const {
      const ZoneContext* zoneContext =
          (const ZoneContext*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->untilYear;
    }

    uint8_t numEras() const {
      return pgm_read_byte(&mZoneInfo->numEras);
    }

    const ZoneEraBroker era(uint8_t i) const {
      const ZoneEra* eras = (const ZoneEra*) pgm_read_ptr(&mZoneInfo->eras);
      return ZoneEraBroker(&eras[i]);
    }

  #else

    const char* name() const { return mZoneInfo->name; }

    uint32_t zoneId() const { return mZoneInfo->zoneId; }

    int16_t startYear() const { return mZoneInfo->zoneContext->startYear; }

    int16_t untilYear() const { return mZoneInfo->zoneContext->untilYear; }

    uint8_t numEras() const { return mZoneInfo->numEras; }

    const ZoneEraBroker era(uint8_t i) const {
      return ZoneEraBroker(&mZoneInfo->eras[i]);
    }

  #endif

  private:
    const ZoneInfo* mZoneInfo;
};

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
class ZoneRegistryBroker {
  public:
    ZoneRegistryBroker(const ZoneInfo* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // use default copy constructor
    ZoneRegistryBroker(const ZoneRegistryBroker&) = default;

    // use default assignment operator
    ZoneRegistryBroker& operator=(const ZoneRegistryBroker&) = default;

  #if ACE_TIME_USE_PROGMEM

    const ZoneInfo* zoneInfo(uint16_t i) const {
      return (const ZoneInfo*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  #else

    const ZoneInfo* zoneInfo(uint16_t i) const {
      return mZoneRegistry[i];
    }

  #endif

  private:
    const ZoneInfo* const* mZoneRegistry;
};

} // basic

//------------------------------------------------------------------------

namespace extended {

/**
 * Convert the `deltaCode` in the ZoneInfo struct to the actual
 * deltaMinutes. The lower 4-bits can store -01:00 to 02:45.
 *
 * @code
 * deltaMinutes = deltaCode * 15m - 1h
 * @endcode
 */
inline int16_t toDeltaMinutes(int8_t deltaCode) {
  return ((int8_t)((uint8_t)deltaCode & 0x0f) - 4) * 15;
}

/**
 * Convert the `offsetCode` and `deltaCode` into a signed 8-bit integer. The
 * `offsetCode` holds the upper 15-minute multiples. The upper 4-bits of
 * `deltaCode` holds the one-minute resolution, as an unsigned offset.
 */
inline int16_t toOffsetMinutes(int8_t offsetCode, int8_t deltaCode) {
  return (offsetCode * 15) + (((uint8_t)deltaCode & 0xf0) >> 4);
}

/** Data broker for accessing ZoneRule. */
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(const ZoneRule* zoneRule):
        mZoneRule(zoneRule) {}

    ZoneRuleBroker():
        mZoneRule(nullptr) {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    int8_t fromYearTiny() const {
      return pgm_read_byte(&mZoneRule->fromYearTiny);
    }

    int8_t toYearTiny() const {
      return pgm_read_byte(&mZoneRule->toYearTiny);
    }

    uint8_t inMonth() const {
      return pgm_read_byte(&mZoneRule->inMonth);
    }

    uint8_t onDayOfWeek() const {
      return pgm_read_byte(&mZoneRule->onDayOfWeek);
    }

    int8_t onDayOfMonth() const {
      return pgm_read_byte(&mZoneRule->onDayOfMonth);
    }

    uint16_t atTimeMinutes() const {
      return internal::timeCodeToMinutes(
          pgm_read_byte(&mZoneRule->atTimeCode),
          pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    uint8_t atTimeSuffix() const {
      return internal::toSuffix(pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    int16_t deltaMinutes() const {
      return toDeltaMinutes(pgm_read_byte(&mZoneRule->deltaCode));
    }

    uint8_t letter() const {
      return pgm_read_byte(&mZoneRule->letter);
    }

  #else

    int8_t fromYearTiny() const { return mZoneRule->fromYearTiny; }

    int8_t toYearTiny() const { return mZoneRule->toYearTiny; }

    uint8_t inMonth() const { return mZoneRule->inMonth; }

    int8_t onDayOfWeek() const { return mZoneRule->onDayOfWeek; }

    int8_t onDayOfMonth() const { return mZoneRule->onDayOfMonth; }

    uint16_t atTimeMinutes() const {
      return internal::timeCodeToMinutes(
          mZoneRule->atTimeCode, mZoneRule->atTimeModifier);
    }

    uint8_t atTimeSuffix() const {
      return internal::toSuffix(mZoneRule->atTimeModifier);
    }

    int16_t deltaMinutes() const {
      return toDeltaMinutes(mZoneRule->deltaCode);
    }

    uint8_t letter() const { return mZoneRule->letter; }

  #endif

  private:
    const ZoneRule* mZoneRule;
};

/** Data broker for accessing ZonePolicy. */
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(const ZonePolicy* zonePolicy):
        mZonePolicy(zonePolicy) {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const ZoneRuleBroker rule(uint8_t i) const {
      const ZoneRule* rules =
          (const ZoneRule*) pgm_read_ptr(&mZonePolicy->rules);
      return ZoneRuleBroker(&rules[i]);
    }

    uint8_t numLetters() const {
      return pgm_read_byte(&mZonePolicy->numLetters);
    }

    const char* letter(uint8_t i) const {
      const char* const* letters = (const char* const*)
          pgm_read_ptr(&mZonePolicy->letters);
      return (const char*) pgm_read_ptr(&letters[i]);
    }

  #else

    uint8_t numRules() const { return mZonePolicy->numRules; }

    const ZoneRuleBroker rule(uint8_t i) const {
      return ZoneRuleBroker(&mZonePolicy->rules[i]);
    }

    uint8_t numLetters() const { return mZonePolicy->numLetters; }

    const char* letter(uint8_t i) const {
      return mZonePolicy->letters[i];
    }

  #endif

  private:
    const ZonePolicy* mZonePolicy;
};

/** Data broker for accessing ZoneEra. */
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(const ZoneEra* zoneEra):
        mZoneEra(zoneEra) {}

    ZoneEraBroker():
        mZoneEra(nullptr) {}

    // use default copy constructor
    ZoneEraBroker(const ZoneEraBroker&) = default;

    // use default assignment operator
    ZoneEraBroker& operator=(const ZoneEraBroker&) = default;

    const ZoneEra* zoneEra() const { return mZoneEra; }

    bool isNull() const { return mZoneEra == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    const ZonePolicyBroker zonePolicy() const {
      return ZonePolicyBroker(
          (const ZonePolicy*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int16_t offsetMinutes() const {
      return toOffsetMinutes(
        pgm_read_byte(&mZoneEra->offsetCode),
        pgm_read_byte(&mZoneEra->deltaCode));
    }

    int16_t deltaMinutes() const {
      return toDeltaMinutes(pgm_read_byte(&mZoneEra->deltaCode));
    }

    const char* format() const {
      return (const char*) pgm_read_ptr(&mZoneEra->format);
    }

    int8_t untilYearTiny() const {
      return pgm_read_byte(&mZoneEra->untilYearTiny);
    }

    uint8_t untilMonth() const {
      return pgm_read_byte(&mZoneEra->untilMonth);
    }

    uint8_t untilDay() const {
      return pgm_read_byte(&mZoneEra->untilDay);
    }

    uint16_t untilTimeMinutes() const {
      return internal::timeCodeToMinutes(
        pgm_read_byte(&mZoneEra->untilTimeCode),
        pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

    uint8_t untilTimeSuffix() const {
      return internal::toSuffix(pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

  #else

    const ZonePolicyBroker zonePolicy() const {
      return ZonePolicyBroker(mZoneEra->zonePolicy);
    }

    int16_t offsetMinutes() const {
      return toOffsetMinutes(mZoneEra->offsetCode, mZoneEra->deltaCode);
    }

    int16_t deltaMinutes() const {
      return toDeltaMinutes(mZoneEra->deltaCode);
    }

    const char* format() const { return mZoneEra->format; }

    int8_t untilYearTiny() const { return mZoneEra->untilYearTiny; }

    uint8_t untilMonth() const { return mZoneEra->untilMonth; }

    uint8_t untilDay() const { return mZoneEra->untilDay; }

    uint16_t untilTimeMinutes() const {
      return internal::timeCodeToMinutes(
          mZoneEra->untilTimeCode, mZoneEra->untilTimeModifier);
    }

    uint8_t untilTimeSuffix() const {
      return internal::toSuffix(mZoneEra->untilTimeModifier);
    }

  #endif

  private:
    const ZoneEra* mZoneEra;

};

/** Data broker for accessing ZoneInfo. */
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = default;

    // use default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = default;

    const ZoneInfo* zoneInfo() const { return mZoneInfo; }

  #if ACE_TIME_USE_PROGMEM

    const char* name() const {
      return (const char*) pgm_read_ptr(&mZoneInfo->name);
    }

    uint32_t zoneId() const {
      return pgm_read_dword(&mZoneInfo->zoneId);
    }

    int16_t startYear() const {
      const ZoneContext* zoneContext =
          (const ZoneContext*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->startYear;
    }

    int16_t untilYear() const {
      const ZoneContext* zoneContext =
          (const ZoneContext*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->untilYear;
    }

    uint8_t numEras() const {
      return pgm_read_byte(&mZoneInfo->numEras);
    }

    const ZoneEraBroker era(uint8_t i) const {
      const ZoneEra* eras = (const ZoneEra*) pgm_read_ptr(&mZoneInfo->eras);
      return ZoneEraBroker(&eras[i]);
    }

  #else

    const char* name() const { return mZoneInfo->name; }

    uint32_t zoneId() const { return mZoneInfo->zoneId; }

    int16_t startYear() const { return mZoneInfo->zoneContext->startYear; }

    int16_t untilYear() const { return mZoneInfo->zoneContext->untilYear; }

    uint8_t numEras() const { return mZoneInfo->numEras; }

    const ZoneEraBroker era(uint8_t i) const {
      return ZoneEraBroker(&mZoneInfo->eras[i]);
    }

  #endif

  private:
    const ZoneInfo* mZoneInfo;
};

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
class ZoneRegistryBroker {
  public:
    ZoneRegistryBroker(const ZoneInfo* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // use default copy constructor
    ZoneRegistryBroker(const ZoneRegistryBroker&) = default;

    // use default assignment operator
    ZoneRegistryBroker& operator=(const ZoneRegistryBroker&) = default;

  #if ACE_TIME_USE_PROGMEM

    const ZoneInfo* zoneInfo(uint16_t i) const {
      return (const ZoneInfo*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  #else

    const ZoneInfo* zoneInfo(uint16_t i) const {
      return mZoneRegistry[i];
    }

  #endif

  private:
    const ZoneInfo* const* mZoneRegistry;
};

} // extended

}

#endif
