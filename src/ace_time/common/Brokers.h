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
 * ACE_TIME_USE_BASIC_PROGMEM or ACE_TIME_USE_EXTENDED_PROGMEM are enabled, the
 * zoneinfo files are stored in flash memory (using the PROGMEM keyword), and
 * cannot be accessed directly on microcontrollers using the Harvard
 * architecture (e.g. AVR) where data and program live in 2 different address
 * spaces. The data in flash memory must be accessed using helper routines in
 * <pgmspace.h>. These classes abstract away this difference so that the code
 * BasicZoneSpecifier and ExtendedZoneSpecifier can be written to be (mostly)
 * agnostic to how the zoneinfo files are stored.
 *
 * When the ACE_TIME_USE_BASIC_PROGMEM and ACE_TIME_USE_EXTENDED_PROGMEM are
 * disabled, the compiler will optimize away this entire abstraction layer, so
 * the resulting machine code is no bigger than (and in most cases, identifical
 * to) accessing the zoneinfo files directly.
 *
 * The abstraction layer is thin enough that the code in BasicZoneSpecifier and
 * ExtendedZoneSpecifier did not change very much. It was mostly a mechanical
 * source code replacement of direct zoneinfo access to using these data
 * brokers.
 *
 * The core broker classes live in the common:: namespace and are templatized
 * so that they can be used for both basic::Zone* classes and the
 * extended::Zone* classes. Specific template instantiations are created in the
 * basic:: and extended:: namespaces so that they can be used by the
 * BasicZoneSpecifier and ExtendedZoneSpecifier respectively.
 */

#include "flash.h"
#include "ZoneInfo.h"

namespace ace_time {

namespace common {

//----------------------------------------------------------------------------
// Direct data brokers for reading from SRAM
//----------------------------------------------------------------------------

/** Data broker for accessing ZoneRule in SRAM. */
template <typename ZR>
class DirectZoneRuleBroker {
  public:
    explicit DirectZoneRuleBroker(const ZR* zoneRule):
        mZoneRule(zoneRule) {}

    DirectZoneRuleBroker():
        mZoneRule(nullptr) {}

    // use the default copy constructor
    DirectZoneRuleBroker(const DirectZoneRuleBroker&) = default;

    // use the default assignment operator
    DirectZoneRuleBroker& operator=(const DirectZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

    bool isNotNull() const { return mZoneRule != nullptr; }

    int8_t fromYearTiny() const { return mZoneRule->fromYearTiny; }

    int8_t toYearTiny() const { return mZoneRule->toYearTiny; }

    int8_t inMonth() const { return mZoneRule->inMonth; }

    int8_t onDayOfWeek() const { return mZoneRule->onDayOfWeek; }

    int8_t onDayOfMonth() const { return mZoneRule->onDayOfMonth; }

    uint8_t atTimeCode() const { return mZoneRule->atTimeCode; }

    uint8_t atTimeModifier() const { return mZoneRule->atTimeModifier; }

    int8_t deltaCode() const { return mZoneRule->deltaCode; }

    uint8_t letter() const { return mZoneRule->letter; }

  private:
    const ZR* mZoneRule;
};

/** Data broker for accessing ZonePolicy in SRAM. */
template <typename ZP, typename ZR>
class DirectZonePolicyBroker {
  public:
    explicit DirectZonePolicyBroker(const ZP* zonePolicy):
        mZonePolicy(zonePolicy) {}

    // use default copy constructor
    DirectZonePolicyBroker(const DirectZonePolicyBroker&) = default;

    // use default assignment operator
    DirectZonePolicyBroker& operator=(const DirectZonePolicyBroker&) = delete;

    bool isNull() const { return mZonePolicy == nullptr; }

    bool isNotNull() const { return mZonePolicy != nullptr; }

    uint8_t numRules() const { return mZonePolicy->numRules; }

    const DirectZoneRuleBroker<ZR> rule(uint8_t i) const {
      return DirectZoneRuleBroker<ZR>(&mZonePolicy->rules[i]);
    }

    uint8_t numLetters() const { return mZonePolicy->numLetters; }

    const char* letter(uint8_t i) const {
      return mZonePolicy->letters[i];
    }

  private:
    const ZP* const mZonePolicy;
};

/** Data broker for accessing ZoneEra in SRAM. */
template <typename ZE, typename ZP, typename ZR>
class DirectZoneEraBroker {
  public:
    explicit DirectZoneEraBroker(const ZE* zoneEra):
        mZoneEra(zoneEra) {}

    DirectZoneEraBroker():
        mZoneEra(nullptr) {}

    // use default copy constructor
    DirectZoneEraBroker(const DirectZoneEraBroker&) = default;

    // use default assignment operator
    DirectZoneEraBroker& operator=(const DirectZoneEraBroker&) = default;

    const ZE* zoneEra() const { return mZoneEra; }

    bool isNull() const { return mZoneEra == nullptr; }

    bool isNotNull() const { return mZoneEra != nullptr; }

    int8_t offsetCode() const { return mZoneEra->offsetCode; }

    const DirectZonePolicyBroker<ZP, ZR> zonePolicy() const {
      return DirectZonePolicyBroker<ZP, ZR>(mZoneEra->zonePolicy);
    }

    int8_t deltaCode() const { return mZoneEra->deltaCode; }

    const char* format() const { return mZoneEra->format; }

    int8_t untilYearTiny() const { return mZoneEra->untilYearTiny; }

    uint8_t untilMonth() const { return mZoneEra->untilMonth; }

    uint8_t untilDay() const { return mZoneEra->untilDay; }

    uint8_t untilTimeCode() const { return mZoneEra->untilTimeCode; }

    uint8_t untilTimeModifier() const { return mZoneEra->untilTimeModifier; }

  private:
    const ZE* mZoneEra;

};

/** Data broker for accessing ZoneInfo in SRAM. */
template <typename ZI, typename ZE, typename ZP, typename ZR>
class DirectZoneInfoBroker {
  public:
    explicit DirectZoneInfoBroker(const ZI* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    DirectZoneInfoBroker(const DirectZoneInfoBroker&) = default;

    // use default assignment operator
    DirectZoneInfoBroker& operator=(const DirectZoneInfoBroker&) = default;

    const ZI* zoneInfo() const { return mZoneInfo; }

    const char* name() const { return mZoneInfo->name; }

    int16_t startYear() const { return mZoneInfo->zoneContext->startYear; }

    int16_t untilYear() const { return mZoneInfo->zoneContext->untilYear; }

    uint8_t numEras() const { return mZoneInfo->numEras; }

    const DirectZoneEraBroker<ZE, ZP, ZR> era(uint8_t i) const {
      return DirectZoneEraBroker<ZE, ZP, ZR>(&mZoneInfo->eras[i]);
    }

  private:
    const ZI* mZoneInfo;
};

/**
 * Data broker for accessing the ZoneRegistry in SRAM. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
template <typename ZI>
class DirectZoneRegistryBroker {
  public:
    DirectZoneRegistryBroker(const ZI* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // delete default copy constructor
    DirectZoneRegistryBroker(const DirectZoneRegistryBroker&) = delete;

    // delete default assignment operator
    DirectZoneRegistryBroker& operator=(const DirectZoneRegistryBroker&) =
        delete;

    const ZI* zoneInfo(uint16_t i) const {
      return mZoneRegistry[i];
    }

  private:
    const ZI* const* const mZoneRegistry;
};

//----------------------------------------------------------------------------
// Data brokers for reading from PROGMEM.
//----------------------------------------------------------------------------

/** Data broker for accessing ZoneRule in PROGMEM. */
template <typename ZR>
class FlashZoneRuleBroker {
  public:
    explicit FlashZoneRuleBroker(const ZR* zoneRule):
        mZoneRule(zoneRule) {}

    FlashZoneRuleBroker():
        mZoneRule(nullptr) {}

    // use the default copy constructor
    FlashZoneRuleBroker(const FlashZoneRuleBroker&) = default;

    // use the default assignment operator
    FlashZoneRuleBroker& operator=(const FlashZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

    bool isNotNull() const { return mZoneRule != nullptr; }

    int8_t fromYearTiny() const {
      return pgm_read_byte(&mZoneRule->fromYearTiny);
    }

    int8_t toYearTiny() const {
      return pgm_read_byte(&mZoneRule->toYearTiny);
    }

    int8_t inMonth() const {
      return pgm_read_byte(&mZoneRule->inMonth);
    }

    int8_t onDayOfWeek() const {
      return pgm_read_byte(&mZoneRule->onDayOfWeek);
    }

    int8_t onDayOfMonth() const {
      return pgm_read_byte(&mZoneRule->onDayOfMonth);
    }

    uint8_t atTimeCode() const {
      return pgm_read_byte(&mZoneRule->atTimeCode);
    }

    uint8_t atTimeModifier() const {
      return pgm_read_byte(&mZoneRule->atTimeModifier);
    }

    int8_t deltaCode() const {
      return pgm_read_byte(&mZoneRule->deltaCode);
    }

    uint8_t letter() const {
      return pgm_read_byte(&mZoneRule->letter);
    }

  private:
    const ZR* mZoneRule;
};

/** Data broker for accessing ZonePolicy in PROGMEM. */
template <typename ZP, typename ZR>
class FlashZonePolicyBroker {
  public:
    explicit FlashZonePolicyBroker(const ZP* zonePolicy):
        mZonePolicy(zonePolicy) {}

    // use default copy constructor
    FlashZonePolicyBroker(const FlashZonePolicyBroker&) = default;

    // use default assignment operator
    FlashZonePolicyBroker& operator=(const FlashZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

    bool isNotNull() const { return mZonePolicy != nullptr; }

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const FlashZoneRuleBroker<ZR> rule(uint8_t i) const {
      const ZR* rules = (const ZR*) pgm_read_ptr(&mZonePolicy->rules);
      return FlashZoneRuleBroker<ZR>(&rules[i]);
    }

    uint8_t numLetters() const {
      return pgm_read_byte(&mZonePolicy->numLetters);
    }

    const char* letter(uint8_t i) const {
      const char* const* letters = (const char* const*)
          pgm_read_ptr(&mZonePolicy->letters);
      return (const char*) pgm_read_ptr(&letters[i]);
    }

  private:
    const ZP* const mZonePolicy;
};

/** Data broker for accessing ZoneEra in PROGMEM. */
template <typename ZE, typename ZP, typename ZR>
class FlashZoneEraBroker {
  public:
    explicit FlashZoneEraBroker(const ZE* zoneEra):
        mZoneEra(zoneEra) {}

    FlashZoneEraBroker():
        mZoneEra(nullptr) {}

    // use default copy constructor
    FlashZoneEraBroker(const FlashZoneEraBroker&) = default;

    // use default assignment operator
    FlashZoneEraBroker& operator=(const FlashZoneEraBroker&) = default;

    const ZE* zoneEra() const { return mZoneEra; }

    bool isNull() const { return mZoneEra == nullptr; }

    bool isNotNull() const { return mZoneEra != nullptr; }

    int8_t offsetCode() const {
      return pgm_read_byte(&mZoneEra->offsetCode);
    }

    const FlashZonePolicyBroker<ZP, ZR> zonePolicy() const {
      return FlashZonePolicyBroker<ZP, ZR>(
          (const ZP*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int8_t deltaCode() const {
      return pgm_read_byte(&mZoneEra->deltaCode);
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

    uint8_t untilTimeCode() const {
      return pgm_read_byte(&mZoneEra->untilTimeCode);
    }

    uint8_t untilTimeModifier() const {
      return pgm_read_byte(&mZoneEra->untilTimeModifier);
    }

  private:
    const ZE* mZoneEra;

};

/** Data broker for accessing ZoneInfo in PROGMEM. */
template <typename ZI, typename ZE, typename ZP, typename ZR>
class FlashZoneInfoBroker {
  public:
    explicit FlashZoneInfoBroker(const ZI* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    FlashZoneInfoBroker(const FlashZoneInfoBroker&) = default;

    // use default assignment operator
    FlashZoneInfoBroker& operator=(const FlashZoneInfoBroker&) = default;

    const ZI* zoneInfo() const { return mZoneInfo; }

    const char* name() const {
      return (const char*) pgm_read_ptr(&mZoneInfo->name);
    }

    int16_t startYear() const {
      const common::ZoneContext* zoneContext = (const common::ZoneContext*)
          pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->startYear;
    }

    int16_t untilYear() const {
      const common::ZoneContext* zoneContext = (const common::ZoneContext*)
          pgm_read_ptr(&mZoneInfo->zoneContext);
      return zoneContext->untilYear;
    }

    uint8_t numEras() const {
      return pgm_read_byte(&mZoneInfo->numEras);
    }

    const FlashZoneEraBroker<ZE, ZP, ZR> era(uint8_t i) const {
      const ZE* eras = (const ZE*) pgm_read_ptr(&mZoneInfo->eras);
      return FlashZoneEraBroker<ZE, ZP, ZR>(&eras[i]);
    }

  private:
    const ZI* mZoneInfo;
};

/**
 * Data broker for accessing the ZoneRegistry in PROGMEM. The ZoneRegistry is
 * an array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
template <typename ZI>
class FlashZoneRegistryBroker {
  public:
    explicit FlashZoneRegistryBroker(const ZI* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // use default copy constructor
    FlashZoneRegistryBroker(const FlashZoneRegistryBroker&) = default;

    // use default assignment operator
    FlashZoneRegistryBroker& operator=(const FlashZoneRegistryBroker&) =
        default;

    const ZI* zoneInfo(uint16_t i) const {
      return (const ZI*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  private:
    const ZI* const* mZoneRegistry;
};

}

//----------------------------------------------------------------------------

namespace basic {

#if ACE_TIME_USE_BASIC_PROGMEM
typedef common::FlashZoneRuleBroker<ZoneRule> ZoneRuleBroker;
typedef common::FlashZonePolicyBroker<ZonePolicy, ZoneRule> ZonePolicyBroker;
typedef common::FlashZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule> ZoneEraBroker;
typedef common::FlashZoneInfoBroker<ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>
    ZoneInfoBroker;
typedef common::FlashZoneRegistryBroker<ZoneInfo> ZoneRegistryBroker;
#else
typedef common::DirectZoneRuleBroker<ZoneRule> ZoneRuleBroker;
typedef common::DirectZonePolicyBroker<ZonePolicy, ZoneRule> ZonePolicyBroker;
typedef common::DirectZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule>
    ZoneEraBroker;
typedef common::DirectZoneInfoBroker<ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>
    ZoneInfoBroker;
typedef common::DirectZoneRegistryBroker<ZoneInfo> ZoneRegistryBroker;
#endif

}

namespace extended {

#if ACE_TIME_USE_EXTENDED_PROGMEM
typedef common::FlashZoneRuleBroker<ZoneRule> ZoneRuleBroker;
typedef common::FlashZonePolicyBroker<ZonePolicy, ZoneRule> ZonePolicyBroker;
typedef common::FlashZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule> ZoneEraBroker;
typedef common::FlashZoneInfoBroker<ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>
    ZoneInfoBroker;
typedef common::FlashZoneRegistryBroker<ZoneInfo> ZoneRegistryBroker;
#else
typedef common::DirectZoneRuleBroker<ZoneRule> ZoneRuleBroker;
typedef common::DirectZonePolicyBroker<ZonePolicy, ZoneRule> ZonePolicyBroker;
typedef common::DirectZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule>
    ZoneEraBroker;
typedef common::DirectZoneInfoBroker<ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>
    ZoneInfoBroker;
typedef common::DirectZoneRegistryBroker<ZoneInfo> ZoneRegistryBroker;
#endif

}

}

#endif
