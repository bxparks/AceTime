/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_H
#define ACE_TIME_BROKERS_H

/**
 * @file Brokers.h
 *
 * These classes provide a thin layer of indirection for accessing the
 * zoneinfo files stored in the zonedb/ and zonedbx/ directories.
 *
 * When ACE_TIME_USE_PROGMEM is enabled, the zoneinfo files are stored in flash
 * memory (using the PROGMEM keyword), and cannot be accessed directly on
 * microcontrollers using the Harvard architecture (e.g. AVR) where data and
 * program live in 2 different address spaces. The data in flash memory must be
 * accessed using helper routines in <pgmspace.h>. These classes abstract away
 * this difference so that the code BasicZoneProcessor and ExtendedZoneProcessor
 * can be written to be (mostly) agnostic to how the zoneinfo files are stored.
 *
 * When ACE_TIME_USE_PROGMEM is disabled, the compiler will optimize away this
 * entire abstraction layer, so the resulting machine code is no bigger than
 * (and in most cases, identical to) accessing the zoneinfo files directly.
 *
 * The abstraction layer is thin enough that the code in BasicZoneProcessor and
 * ExtendedZoneProcessor did not change very much. It was mostly a mechanical
 * source code replacement of direct zoneinfo access to using these data
 * brokers.
 *
 * The brokers in the basic:: and extended:: namespaces are identical in code.
 * The purpose for having separate class hierarchies is to provide compile-time
 * assurance that the BasicZoneProcessor and ExtendedZoneProcessor are given the
 * correct zoneinfo files from the appropriate zonedb database.
 */

#include <stdint.h> // uintptr_t, uint32_t, etc
#include <AceCommon.h> // KString
#include "../common/compat.h" // ACE_TIME_USE_PROGMEM
#include "BrokerCommon.h"
#include "ZoneContext.h"
#include "ZoneInfo.h"

class __FlashStringHelper;
class Print;

namespace ace_time {
namespace internal {

/**
 * Data broker for accessing ZoneRule.
 *
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZR>
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(
        const internal::ZoneContext* zoneContext = nullptr,
        const ZR* zoneRule = nullptr)
        : mZoneContext(zoneContext)
        , mZoneRule(zoneRule)
    {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    int16_t fromYear() const {
      return pgm_read_word(&mZoneRule->fromYear);
    }

    int16_t toYear() const {
      return pgm_read_word(&mZoneRule->toYear);
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
      return internal::toDeltaMinutes(pgm_read_byte(&mZoneRule->deltaCode));
    }

    const char* letter() const {
      uint8_t index = pgm_read_byte(&mZoneRule->letterIndex);
      return mZoneContext->letters[index];
    }

  #else

    int16_t fromYear() const { return mZoneRule->fromYear; }

    int16_t toYear() const { return mZoneRule->toYear; }

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

    int16_t deltaMinutes() const {
      return internal::toDeltaMinutes(mZoneRule->deltaCode);
    }

    const char* letter() const {
      uint8_t index = mZoneRule->letterIndex;
      return mZoneContext->letters[index];
    }

  #endif

  private:
    const internal::ZoneContext* mZoneContext;
    const ZR* mZoneRule;
};

/**
 * Data broker for accessing ZonePolicy.
 *
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZP, typename ZR>
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(
        const internal::ZoneContext* zoneContext,
        const ZP* zonePolicy)
        : mZoneContext(zoneContext)
        , mZonePolicy(zonePolicy)
    {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const ZoneRuleBroker<ZR> rule(uint8_t i) const {
      const ZR* rules = (const ZR*) pgm_read_ptr(&mZonePolicy->rules);
      return ZoneRuleBroker<ZR>(mZoneContext, &rules[i]);
    }

  #else

    uint8_t numRules() const { return mZonePolicy->numRules; }

    const ZoneRuleBroker<ZR> rule(uint8_t i) const {
      return ZoneRuleBroker<ZR>(mZoneContext, &mZonePolicy->rules[i]);
    }

  #endif

  private:
    const internal::ZoneContext* mZoneContext;
    const ZP* mZonePolicy;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing ZoneEra.
 *
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZE, typename ZP, typename ZR>
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(
        const internal::ZoneContext* zoneContext = nullptr,
        const ZE* zoneEra = nullptr)
        : mZoneContext(zoneContext)
        , mZoneEra(zoneEra)
    {}

    // use default copy constructor
    ZoneEraBroker(const ZoneEraBroker&) = default;

    // use default assignment operator
    ZoneEraBroker& operator=(const ZoneEraBroker&) = default;

    bool isNull() const { return mZoneEra == nullptr; }

    bool equals(const ZoneEraBroker& other) const {
      return mZoneEra == other.mZoneEra;
    }

  #if ACE_TIME_USE_PROGMEM

    const ZonePolicyBroker<ZP, ZR> zonePolicy() const {
      return ZonePolicyBroker<ZP, ZR>(
          mZoneContext,
          (const ZP*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int16_t offsetMinutes() const {
      return internal::toOffsetMinutes(
        pgm_read_byte(&mZoneEra->offsetCode),
        pgm_read_byte(&mZoneEra->deltaCode));
    }

    int16_t deltaMinutes() const {
      return internal::toDeltaMinutes(pgm_read_byte(&mZoneEra->deltaCode));
    }

    const char* format() const {
      return (const char*) pgm_read_ptr(&mZoneEra->format);
    }

    int16_t untilYear() const {
      return pgm_read_word(&mZoneEra->untilYear);
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

    const ZonePolicyBroker<ZP> zonePolicy() const {
      return ZonePolicyBroker<ZP>(mZoneContext, mZoneEra->zonePolicy);
    }

    int16_t offsetMinutes() const {
      return internal::toOffsetMinutes(
          mZoneEra->offsetCode, mZoneEra->deltaCode);
    }

    int16_t deltaMinutes() const {
      return internal::toDeltaMinutes(mZoneEra->deltaCode);
    }

    const char* format() const { return mZoneEra->format; }

    int16_t untilYear() const { return mZoneEra->untilYear; }

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
    const internal::ZoneContext* mZoneContext;
    const ZE* mZoneEra;
};

/**
 * Data broker for accessing ZoneInfo.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZI, typename ZE, typename ZP, typename ZR>
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZI* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = default;

    // use default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = default;

    /**
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    bool equals(uintptr_t zoneKey) const {
      return mZoneInfo == (const ZI*) zoneKey;
    }

    bool equals(const ZoneInfoBroker& zoneInfoBroker) const {
      return mZoneInfo == zoneInfoBroker.mZoneInfo;
    }

    bool isNull() const { return mZoneInfo == nullptr; }

  #if ACE_TIME_USE_PROGMEM

    const internal::ZoneContext* zoneContext() const {
      return (const internal::ZoneContext*)
          pgm_read_ptr(&mZoneInfo->zoneContext);
    }

    const __FlashStringHelper* name() const {
      return FPSTR(pgm_read_ptr(&mZoneInfo->name));
    }

    uint32_t zoneId() const {
      return pgm_read_dword(&mZoneInfo->zoneId);
    }

    uint8_t numEras() const {
      return pgm_read_byte(&mZoneInfo->numEras);
    }

    const ZoneEraBroker<ZE, ZP, ZR> era(uint8_t i) const {
      auto eras = (const ZE*) pgm_read_ptr(&mZoneInfo->eras);
      return ZoneEraBroker<ZE, ZP, ZR>(zoneContext(), &eras[i]);
    }

    bool isLink() const {
      return mZoneInfo->targetInfo != nullptr;
    }

    ZoneInfoBroker targetInfo() const {
      return ZoneInfoBroker(
          (const ZI*) pgm_read_ptr(&mZoneInfo->targetInfo));
    }

  #else

    const internal::ZoneContext* zoneContext() const {
      return mZoneInfo->zoneContext;
    }

    const char* name() const { return mZoneInfo->name; }

    uint32_t zoneId() const { return mZoneInfo->zoneId; }

    uint8_t numEras() const { return mZoneInfo->numEras; }

    const ZoneEraBroker era(uint8_t i) const {
      return ZoneEraBroker(zoneContext(), &mZoneInfo->eras[i]);
    }

    const ZoneInfoBroker targetInfo() const {
      return ZoneInfoBroker(mZoneInfo->targetInfo);
    }

  #endif

    /** Print a human-readable identifier (e.g. "America/Los_Angeles"). */
    void printNameTo(Print& printer) const;

    /**
     * Print a short human-readable identifier (e.g. "Los Angeles").
     * Any underscore in the short name is replaced with a space.
     */
    void printShortNameTo(Print& printer) const;

  private:
    const ZI* mZoneInfo;
};


template <typename ZI, typename ZE, typename ZP, typename ZR>
void ZoneInfoBroker<ZI, ZE, ZP, ZR>::printNameTo(Print& printer) const {
  const ZoneContext* zc = zoneContext();
  ace_common::KString kname(name(), zc->fragments, zc->numFragments);
  kname.printTo(printer);
}

template <typename ZI, typename ZE, typename ZP, typename ZR>
void ZoneInfoBroker<ZI, ZE, ZP, ZR>::printShortNameTo(Print& printer) const {
  ace_common::printReplaceCharTo(printer, findShortName(name()), '_', ' ');
}

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo or extended::ZoneInfo)
 */
template <typename ZI>
class ZoneRegistryBroker {
  public:
    ZoneRegistryBroker(const ZI* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // use default copy constructor
    ZoneRegistryBroker(const ZoneRegistryBroker&) = default;

    // use default assignment operator
    ZoneRegistryBroker& operator=(const ZoneRegistryBroker&) = default;

  #if ACE_TIME_USE_PROGMEM

    const ZI* zoneInfo(uint16_t i) const {
      return (const ZI*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  #else

    const ZI* zoneInfo(uint16_t i) const {
      return mZoneRegistry[i];
    }

  #endif

  private:
    const ZI* const* mZoneRegistry;
};

//-----------------------------------------------------------------------------

/**
 * A factory that creates an ZoneInfoBroker.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZI, typename ZE, typename ZP, typename ZR>
class BrokerFactory {
  public:
    /**
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    ZoneInfoBroker<ZI, ZE, ZP, ZR>
    createZoneInfoBroker(uintptr_t zoneKey) const {
      return ZoneInfoBroker<ZI, ZE, ZP, ZR>((const ZI*) zoneKey);
    }
};

} // internal

//-----------------------------------------------------------------------------

namespace basic {

/** Data broker for accessing ZoneRule. */
using ZoneRuleBroker = internal::ZoneRuleBroker<ZoneRule>;

/** Data broker for accessing ZonePolicy. */
using ZonePolicyBroker = internal::ZonePolicyBroker<ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneEra. */
using ZoneEraBroker = internal::ZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneInfo. */
using ZoneInfoBroker = internal::ZoneInfoBroker<
    ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
using ZoneRegistryBroker = internal::ZoneRegistryBroker<ZoneInfo>;

using BrokerFactory = internal::BrokerFactory<
    ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

} // basic

//-----------------------------------------------------------------------------

namespace extended {

/** Data broker for accessing ZoneRule. */
using ZoneRuleBroker = internal::ZoneRuleBroker<ZoneRule>;

/** Data broker for accessing ZonePolicy. */
using ZonePolicyBroker = internal::ZonePolicyBroker<ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneEra. */
using ZoneEraBroker = internal::ZoneEraBroker<ZoneEra, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneInfo. */
using ZoneInfoBroker = internal::ZoneInfoBroker<
    ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
using ZoneRegistryBroker = internal::ZoneRegistryBroker<ZoneInfo>;

using BrokerFactory = internal::BrokerFactory<
    ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

} // extended

} // ace_time

#endif
