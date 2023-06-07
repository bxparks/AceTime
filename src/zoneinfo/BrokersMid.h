/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_MID_H
#define ACE_TIME_BROKERS_MID_H

/**
 * @file BrokersMid.h
 *
 * These classes provide a thin layer of indirection for accessing the
 * data structures defined by `ZoneInfoMid.h`.
 *
 * The zoneinfo files are stored in flash memory (using the PROGMEM keyword),
 * and cannot be accessed directly on microcontrollers using the Harvard
 * architecture (e.g. AVR) where data and program live in 2 different address
 * spaces. The data in flash memory must be accessed using helper routines in
 * <pgmspace.h>. These classes abstract away this difference so that the code
 * BasicZoneProcessor and ExtendedZoneProcessor can be written to be (mostly)
 * agnostic to how the zoneinfo files are stored.
 */

#include <stdint.h> // uintptr_t, uint32_t, etc
#include <AceCommon.h> // KString
#include "compat.h" // ACE_TIME_USE_PROGMEM
#include "BrokerCommon.h"
#include "ZoneInfoMid.h"

class __FlashStringHelper;
class Print;

namespace ace_time {
namespace zoneinfomid {

//-----------------------------------------------------------------------------

/**
 * Convert the `deltaCode` in the ZoneInfo or the ZoneRule struct to the actual
 * deltaMinutes. The lower 4-bits stores minutes in units of 15-minutes, shifted
 * by 1h, so can represent the interval [-01:00 to 02:45].
 *
 * @code
 * deltaMinutes = deltaCode * 15m - 1h
 * @endcode
 */
inline int16_t toDeltaMinutes(uint8_t deltaCode) {
  return ((int16_t)(deltaCode & 0x0f) - 4) * 15;
}

/**
 * Convert the `offsetCode` and `deltaCode` into a signed 16-bit integer that
 * represents the UTCOFF of the ZoneEra in minutes. The `offsetCode` is rounded
 * towards -infinity in 15-minute multiples. The upper 4-bits of `deltaCode`
 * holds the (unsigned) remainder in one-minute increments.
 */
inline int16_t toOffsetMinutes(int8_t offsetCode, uint8_t deltaCode) {
  return (offsetCode * 15) + (((uint8_t)deltaCode & 0xf0) >> 4);
}


/**
 * Convert (code, modifier) fields representing the UNTIL time in ZoneInfo or AT
 * time in ZoneRule in one minute resolution. The `code` parameter holds the AT
 * or UNTIL time in minutes component in units of 15 minutes. The lower 4-bits
 * of `modifier` holds the remainder minutes.
 */
inline uint16_t timeCodeToMinutes(uint8_t code, uint8_t modifier) {
  return code * (uint16_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU. Used for Zone.UNTIL and
 * Rule.AT  fields.
 */
inline uint8_t toSuffix(uint8_t modifier) {
  return modifier & 0xf0;
}

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing a ZoneContext. Currently, this assumes that the
 * ZoneContext is in RAM. TODO: Move ZoneContext to PROGMEM for consistency.
 *
 * @tparam ZC the specific ZoneContext class
 */
template <typename ZC>
class ZoneContextBroker {
  public:
    explicit ZoneContextBroker(const ZC* zoneContext = nullptr)
        : mZoneContext(zoneContext)
    {}

    // use the default copy constructor
    ZoneContextBroker(const ZoneContextBroker&) = default;

    // use the default assignment operator
    ZoneContextBroker& operator=(const ZoneContextBroker&) = default;

    bool isNull() const { return mZoneContext == nullptr; }

    const ZC* raw() const { return mZoneContext; }

    int16_t startYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->startYear);
    }

    int16_t untilYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->untilYear);
    }

    int16_t baseYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->baseYear);
    }

    int16_t maxTransitions() const {
      return (int16_t) pgm_read_word(&mZoneContext->maxTransitions);
    }

    const __FlashStringHelper* tzVersion() const {
      return (const __FlashStringHelper*)
          pgm_read_ptr(&mZoneContext->tzVersion);
    }

    uint8_t numFragments() const {
      return (uint8_t) pgm_read_byte(&mZoneContext->numFragments);
    }

    uint8_t numLetters() const {
      return (uint8_t) pgm_read_byte(&mZoneContext->numLetters);
    }

    const char* const* fragments() const {
      return (const char* const*) pgm_read_ptr(&mZoneContext->fragments);
    }

    const __FlashStringHelper* letter(uint8_t i) const {
      const char * const* letters = (const char* const*)
          pgm_read_ptr(&mZoneContext->letters);
      const char* letter = (const char*) pgm_read_ptr(letters + i);
      return (const __FlashStringHelper*) letter;
    }

  private:
    const ZC* mZoneContext;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing ZoneRule.
 *
 * @tparam ZC ZoneContext type
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZC, typename ZR>
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(
        const ZC* zoneContext = nullptr,
        const ZR* zoneRule = nullptr)
        : mZoneContext(zoneContext)
        , mZoneRule(zoneRule)
    {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

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

    uint32_t atTimeSeconds() const {
      return 60 * timeCodeToMinutes(
          pgm_read_byte(&mZoneRule->atTimeCode),
          pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    uint8_t atTimeSuffix() const {
      return toSuffix(pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    int32_t deltaSeconds() const {
      return 60 * toDeltaMinutes(pgm_read_byte(&mZoneRule->deltaCode));
    }

    const __FlashStringHelper* letter() const {
      uint8_t index = pgm_read_byte(&mZoneRule->letterIndex);
      return ZoneContextBroker<ZC>(mZoneContext).letter(index);
    }

  private:
    const ZC* mZoneContext;
    const ZR* mZoneRule;
};

/**
 * Data broker for accessing ZonePolicy.
 *
 * @tparam ZC ZoneContext type
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZC, typename ZP, typename ZR>
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(
        const ZC* zoneContext,
        const ZP* zonePolicy)
        : mZoneContext(zoneContext)
        , mZonePolicy(zonePolicy)
    {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const ZoneRuleBroker<ZC, ZR> rule(uint8_t i) const {
      const ZR* rules = (const ZR*) pgm_read_ptr(&mZonePolicy->rules);
      return ZoneRuleBroker<ZC, ZR>(mZoneContext, &rules[i]);
    }

  private:
    const ZC* mZoneContext;
    const ZP* mZonePolicy;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing ZoneEra.
 *
 * @tparam ZC ZoneContext
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZC, typename ZE, typename ZP, typename ZR>
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(
        const ZC* zoneContext = nullptr,
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

    const ZonePolicyBroker<ZC, ZP, ZR> zonePolicy() const {
      return ZonePolicyBroker<ZC, ZP, ZR>(
          mZoneContext,
          (const ZP*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int32_t offsetSeconds() const {
      return 60 * toOffsetMinutes(
        pgm_read_byte(&mZoneEra->offsetCode),
        pgm_read_byte(&mZoneEra->deltaCode));
    }

    int32_t deltaSeconds() const {
      return 60 * toDeltaMinutes(pgm_read_byte(&mZoneEra->deltaCode));
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

    uint32_t untilTimeSeconds() const {
      return 60 * timeCodeToMinutes(
        pgm_read_byte(&mZoneEra->untilTimeCode),
        pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

    uint8_t untilTimeSuffix() const {
      return toSuffix(pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

  private:
    const ZC* mZoneContext;
    const ZE* mZoneEra;
};

/**
 * Data broker for accessing ZoneInfo.
 *
 * @tparam ZC ZoneContext type
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZC, typename ZI, typename ZE, typename ZP, typename ZR>
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

    const ZoneContextBroker<ZC> zoneContext() const {
      const ZC* context = (const ZC*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return ZoneContextBroker<ZC>(context);
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

    const ZoneEraBroker<ZC, ZE, ZP, ZR> era(uint8_t i) const {
      auto eras = (const ZE*) pgm_read_ptr(&mZoneInfo->eras);
      return ZoneEraBroker<ZC, ZE, ZP, ZR>(zoneContext().raw(), &eras[i]);
    }

    bool isLink() const {
      return mZoneInfo->targetInfo != nullptr;
    }

    ZoneInfoBroker targetInfo() const {
      return ZoneInfoBroker(
          (const ZI*) pgm_read_ptr(&mZoneInfo->targetInfo));
    }

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


template <typename ZC, typename ZI, typename ZE, typename ZP, typename ZR>
void ZoneInfoBroker<ZC, ZI, ZE, ZP, ZR>::printNameTo(Print& printer) const {
  ZoneContextBroker<ZC> zc = zoneContext();
  ace_common::KString kname(name(), zc.fragments(), zc.numFragments());
  kname.printTo(printer);
}

template <typename ZC, typename ZI, typename ZE, typename ZP, typename ZR>
void ZoneInfoBroker<ZC, ZI, ZE, ZP, ZR>::printShortNameTo(Print& printer)
    const {
  ace_common::printReplaceCharTo(
      printer, internal::findShortName(name()), '_', ' ');
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

    const ZI* zoneInfo(uint16_t i) const {
      return (const ZI*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  private:
    const ZI* const* mZoneRegistry;
};

//-----------------------------------------------------------------------------

/**
 * A storage object that creates an ZoneInfoBroker from a key that identifies
 * the ZoneInfo. The key can be a pointer to flash memory, or an integer into
 * a list stored in a file.
 *
 * @tparam ZC ZoneContext type
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZE ZoneEra type (e.g. basic::ZoneEra or extended::ZoneEra)
 * @tparam ZP ZonePolicy type (e.g. basic::ZonePolicy or extended::ZonePolicy)
 * @tparam ZR ZoneRule type (e.g. basic::ZoneRule or extended::ZoneRule)
 */
template <typename ZC, typename ZI, typename ZE, typename ZP, typename ZR>
class ZoneInfoStore {
  public:
    /**
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    ZoneInfoBroker<ZC, ZI, ZE, ZP, ZR>
    createZoneInfoBroker(uintptr_t zoneKey) const {
      return ZoneInfoBroker<ZC, ZI, ZE, ZP, ZR>((const ZI*) zoneKey);
    }
};

} // zoneinfomid
} // ace_time

#endif
