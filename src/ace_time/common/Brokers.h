/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_H
#define ACE_TIME_BROKERS_H

#include "common.h"
#include "ZoneInfo.h"

/**
 * @file Brokers.h
 *
 * The classes provide a thin layer of indirection for accessing the
 * zoneinfo files stored in the zonedb/ and zonedbx/ directories. When
 * ACE_TIME_USE_PROGMEM_BASIC or ACE_TIME_USE_PROGMEM_EXTENDED are enabled, the
 * zoneinfo files are stored in flash memory (using the PROGMEM keyword), and
 * cannot be accessed directly on microcontrollers using the Harvard
 * architecture (e.g. AVR) where data and program live in 2 different address
 * spaces. The data in flash memory must be accessed using helper routines in
 * <pgmspace.h>. These classes abstract away this difference so that the code
 * BasicZoneSpecifier and ExtendedZoneSpecifier can be written to be (mostly)
 * agnostic to how the zoneinfo files are stored.
 *
 * When the ACE_TIME_USE_PROGMEM_BASIC and ACE_TIME_USE_PROGMEM_EXTENDED are
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

namespace ace_time {

namespace common {

/** Data broker for accessing ZoneRule in SRAM or PROGMEM. */
template <typename ZR>
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(const ZR* zoneRule):
        mZoneRule(zoneRule) {}

    ZoneRuleBroker():
        mZoneRule(nullptr) {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

    bool isNotNull() const { return mZoneRule != nullptr; }

    int8_t fromYearTiny() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->fromYearTiny;
    #endif
    }

    int8_t toYearTiny() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->toYearTiny;
    #endif
    }

    int8_t inMonth() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->inMonth;
    #endif
    }

    int8_t onDayOfWeek() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->onDayOfWeek;
    #endif
    }

    int8_t onDayOfMonth() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->onDayOfMonth;
    #endif
    }

    int8_t atTimeCode() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->atTimeCode;
    #endif
    }

    int8_t atTimeModifier() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->atTimeModifier;
    #endif
    }

    int8_t deltaCode() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->deltaCode;
    #endif
    }

    uint8_t letter() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneRule->letter;
    #endif
    }

  private:
    const ZR* mZoneRule;
};

/** Data broker for accessing ZonePolicy in SRAM or PROGMEM. */
template <typename ZP, typename ZR>
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(const ZP* zonePolicy):
        mZonePolicy(zonePolicy) {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = delete;

    bool isNull() const { return mZonePolicy == nullptr; }

    bool isNotNull() const { return mZonePolicy != nullptr; }

    uint8_t numRules() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZonePolicy->numRules;
    #endif
    }

    const ZoneRuleBroker<ZR> rule(uint8_t i) const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return ZoneRuleBroker<ZR>(&mZonePolicy->rules[i]);
    #endif
    }

  private:
    const ZP* const mZonePolicy;
};

/** Data broker for accessing ZoneEra in SRAM or PROGMEM. */
template <typename ZE, typename ZP, typename ZR>
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(const ZE* zoneEra):
        mZoneEra(zoneEra) {}

    ZoneEraBroker():
        mZoneEra(nullptr) {}

    // use default copy constructor
    ZoneEraBroker(const ZoneEraBroker&) = default;

    // use default assignment operator
    ZoneEraBroker& operator=(const ZoneEraBroker&) = default;

    const ZonePolicyBroker<ZP, ZR> zonePolicy() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return ZonePolicyBroker<ZP, ZR>(mZoneEra->zonePolicy);
    #endif
    }

    int8_t offsetCode() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneEra->offsetCode;
    #endif
    }

    int8_t untilYearTiny() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneEra->untilYearTiny;
    #endif
    }

    const char* format() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneEra->format;
    #endif
    }

  private:
    const ZE* mZoneEra;

};

/** Data broker for accessing ZoneInfo in SRAM or PROGMEM. */
template <typename ZI, typename ZE, typename ZP, typename ZR>
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZI* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // delete default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = delete;

    // delete default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = delete;

    const ZI* zoneInfo() const { return mZoneInfo; }

    int16_t startYear() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneInfo->zoneContext->startYear;
    #endif
    }

    int16_t untilYear() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneInfo->zoneContext->untilYear;
    #endif
    }

    uint8_t numEras() const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return mZoneInfo->numEras;
    #endif
    }

    const ZoneEraBroker<ZE, ZP, ZR> era(uint8_t i) const {
    #if ACE_TIME_USE_PROGMEM_BASIC
    #else
      return ZoneEraBroker<ZE, ZP, ZR>(&mZoneInfo->eras[i]);
    #endif
    }

  private:
    const ZI* const mZoneInfo;
};

}

namespace basic {

typedef common::ZoneRuleBroker<basic::ZoneRule> ZoneRuleBroker;
typedef common::ZonePolicyBroker<basic::ZonePolicy, basic::ZoneRule>
    ZonePolicyBroker;
typedef common::ZoneEraBroker<basic::ZoneEra, basic::ZonePolicy,
    basic::ZoneRule> ZoneEraBroker;
typedef common::ZoneInfoBroker<basic::ZoneInfo, basic::ZoneEra, basic::ZonePolicy,
    basic::ZoneRule> ZoneInfoBroker;

}

}

#endif
