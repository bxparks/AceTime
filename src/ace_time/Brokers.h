/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_H
#define ACE_TIME_BROKERS_H

#include "common/common.h"
#include "common/ZoneInfo.h"

namespace ace_time {

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

typedef ZoneRuleBroker<basic::ZoneRule> BasicZoneRuleBroker;
typedef ZonePolicyBroker<basic::ZonePolicy, basic::ZoneRule> \
    BasicZonePolicyBroker;
typedef ZoneEraBroker<basic::ZoneEra, basic::ZonePolicy, basic::ZoneRule> \
    BasicZoneEraBroker;
typedef ZoneInfoBroker<basic::ZoneInfo, basic::ZoneEra, basic::ZonePolicy,
    basic::ZoneRule> BasicZoneInfoBroker;

}

#endif
