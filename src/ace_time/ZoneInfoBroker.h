/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_INFO_BROKER_H
#define ACE_TIME_ZONE_INFO_BROKER_H

#include "common/ZoneInfo.h"

namespace ace_time {

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

    int8_t fromYearTiny() const { return mZoneRule->fromYearTiny; }

    int8_t toYearTiny() const { return mZoneRule->toYearTiny; }

    int8_t inMonth() const { return mZoneRule->inMonth; }

    int8_t onDayOfWeek() const { return mZoneRule->onDayOfWeek; }

    int8_t onDayOfMonth() const { return mZoneRule->onDayOfMonth; }

    int8_t atTimeCode() const { return mZoneRule->atTimeCode; }

    int8_t atTimeModifier() const { return mZoneRule->atTimeModifier; }

    int8_t deltaCode() const { return mZoneRule->deltaCode; }

    uint8_t letter() const { return mZoneRule->letter; }

  private:
    const ZR* mZoneRule;
};

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

    uint8_t numRules() const { return mZonePolicy->numRules; }

    const ZoneRuleBroker<ZR> rule(uint8_t i) const {
      return ZoneRuleBroker<ZR>(&mZonePolicy->rules[i]);
    }

  private:
    const ZP* const mZonePolicy;
};

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
      return ZonePolicyBroker<ZP, ZR>(mZoneEra->zonePolicy);
    }

    int8_t offsetCode() const { return mZoneEra->offsetCode; }

    int8_t untilYearTiny() const { return mZoneEra->untilYearTiny; }

    const char* format() const { return mZoneEra->format; }

  private:
    const ZE* mZoneEra;

};

template <typename ZI, typename ZE, typename ZP, typename ZR>
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZI* zoneInfo):
        mZoneInfo(zoneInfo) {}

    // delete default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = delete;

    // delete default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = delete;

    int16_t startYear() const {
      return mZoneInfo->zoneContext->startYear;
    }

    int16_t untilYear() const {
      return mZoneInfo->zoneContext->untilYear;
    }

    uint8_t numEras() const {
      return mZoneInfo->numEras;
    }

    const ZoneEraBroker<ZE, ZP, ZR> era(uint8_t i) const {
      return ZoneEraBroker<ZE, ZP, ZR>(&mZoneInfo->eras[i]);
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
