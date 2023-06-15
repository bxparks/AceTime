/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_PROCESSOR_H
#define ACE_TIME_BASIC_ZONE_PROCESSOR_H

#include <stdint.h>
#include <AceCommon.h> // strncpy_T()
#include "../zoneinfo/infos.h"
#include "../zoneinfo/brokers.h"
#include "common/common.h" // kAbbrevSize
#include "common/logging.h"
#include "TimeOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneProcessor.h"

#ifndef ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG
#define ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG 0
#endif

class BasicZoneProcessorTest_priorYearOfRule;
class BasicZoneProcessorTest_compareRulesBeforeYear;
class BasicZoneProcessorTest_findLatestPriorRule;
class BasicZoneProcessorTest_findZoneEra;
class BasicZoneProcessorTest_init_primitives;
class BasicZoneProcessorTest_initForLocalDate;
class BasicZoneProcessorTest_setZoneKey;
class BasicZoneProcessorTest_createAbbreviation;
class BasicZoneProcessorTest_calcRuleOffsetMinutes;

class Print;

namespace ace_time {
namespace basic {

/**
 * Data structure that defines the start of a specific UTC offset as described
 * by the matching ZoneEra and its ZoneRule for a given year. If the ZoneEra
 * does not have a ZoneRule, then the Transition is defined by the start date
 * of the ZoneEra.
 *
 * The 'era' and 'rule' variables' intermediate values calculated during the
 * init() phase. They are used to calculate the 'year', 'startEpochSeconds',
 * 'offsetSeconds', 'deltaSeconds', and 'abbrev' parameters which are used
 * during findMatch() lookup. This separation helps in moving the ZoneInfo and
 * ZonePolicy data structures into PROGMEM.
 *
 * Ordering of fields optimized along 4-byte boundaries to help 32-bit
 * processors without making the program size bigger for 8-bit processors.
 *
 * @tparam ZIB type of ZoneInfoBroker
 * @tparam ZEB type of ZoneEraBroker
 * @tparam ZPB type of ZonePolicyBroker
 * @tparam ZRB type of ZoneRuleBroker
 */
template <typename ZIB, typename ZEB, typename ZPB, typename ZRB>
struct TransitionTemplate {
  /** The ZoneEra that matched the given year. NonNullable.
   *
   * This field is used only during the init() phase, not during the
   * findMatch() phase.
   */
  ZEB era;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-' or 'hh:mm'. The fixed DST offset placed
   * in deltaOffset. Two examples of such a timezone isEurope/Istanbul and
   * America/Argentina/San_Luis.
   *
   * This field is used only during the init() phase, not during the
   * findMatch() phase.
   */
  ZRB rule;

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /**
   * The total effective UTC offset minutes at the start of transition,
   * *including* DST offset. (Maybe rename this effectiveOffsetMinutes?) The
   * DST offset is stored at deltaMinutes.
   */
  int16_t offsetMinutes;

  /** The deltaMinutes from "standard time" at the start of transition */
  int16_t deltaMinutes;

  /** Year of the Transition. */
  int16_t year;

  /**
   * Month of the transition. Copied from ZoneRule.inMonth() if it exists or
   * set to 1 if ZoneRule is null (indicating that the ZoneEra represents a
   * fixed offset for the entire year).
   */
  uint8_t month;

  /**
   * The calculated effective time zone abbreviation, e.g. "PST" or "PDT".
   * When the Transition is initially created using createTransition(),
   * abbrev[0] is set to ZoneRule.letter (to avoid potentially another lookup
   * in PROGMEM). That 'letter' is used later in the init() to generate
   * the correct abbreviation which will replace the 'letter' in here.
   */
  char abbrev[internal::kAbbrevSize];

  /** Used only for debugging. */
  void log() const {
    if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
      logging::printf("(%d/%d)", year, month);
      if (sizeof(acetime_t) == sizeof(int)) {
        logging::printf("; stEps: %d", startEpochSeconds);
      } else {
        logging::printf("; stEps: %ld", startEpochSeconds);
      }
      logging::printf("; offMin: %d", offsetMinutes);
      logging::printf("; abbrev: %s", abbrev);
      if (! rule.isNull()) {
        logging::printf("; r.fromYear: %d", rule.fromYear());
        logging::printf("; r.toYear: %d", rule.toYear());
        logging::printf("; r.inMonth: %d", rule.inMonth());
        logging::printf("; r.onDayOfMonth: %d", rule.onDayOfMonth());
      }
      logging::printf("\n");
    }
  }
};

/** Compare two (year, month) pairs and return (-1, 0, 1). */
inline int8_t compareYearMonth(int16_t aYear, uint8_t aMonth,
    int16_t bYear, uint8_t bMonth) {
  if (aYear < bYear) return -1;
  if (aYear > bYear) return 1;
  if (aMonth < bMonth) return -1;
  if (aMonth > bMonth) return 1;
  return 0;
}

} // namespace basic

/**
 * An implementation of ZoneProcessor that supports a subset of the zones
 * containing in the TZ Database. The supported list of zones, as well as the
 * list of unsupported zones, are is listed in the zonedb/zone_info.h header
 * file. The constructor expects a pointer to one of the ZoneInfo structures
 * declared in the zone_infos.h file.
 *
 * The internal ZoneRule and ZoneEra records that match the year of the given
 * epochSeconds are cached for performance. The expectation is that repeated
 * calls to the various methods will have epochSeconds which do not vary too
 * greatly and will occur in the same year.
 *
 * The Rule records are transition points which look like this:
 * @verbatim
 * Rule  NAME  FROM    TO  TYPE    IN     ON        AT      SAVE    LETTER/S
 * @endverbatim
 *
 * Each record is represented by basic::ZoneRule and the entire collection is
 * represented by basic::ZonePolicy.
 *
 * The Zone records define the region which follows a specific set of Rules
 * for certain time periods (given by UNTIL below):
 * @verbatim
 * Zone NAME              GMTOFF    RULES FORMAT  [UNTIL]
 * @endverbatim
 *
 * Each record is represented by basic::ZoneEra and the entire collection is
 * represented by basic::ZoneInfo.
 *
 * This class assumes that the various components of ZoneInfo, ZoneEra, and
 * ZonePolicy, ZoneRule have a number of limitations and constraints which
 * simplify the implementation of this class. The tzcompiler.py script will
 * remove zones which do not meet these constraints when generating the structs
 * defined by zonedb/zone_infos.h. The constraints are at least the following
 * (see AceTimeTools/transformer.py for the authoratative algorithm):
 *
 *  * ZoneInfo UNTIL field must contain only the full year;
 *    cannot contain month, day, or time components
 *  * ZoneInfo untilTimeSuffix can contain only 'w' (not 's' or 'u')
 *  * ZonePolicy can contain only 1 ZoneRule in a single month
 *  * ZoneRule AT time cannot occur on Jan 1
 *  * ZoneRule atTimeSuffix can be any of ('w', 's', and 'u')
 *  * ZoneRule LETTER must contain only a single letter (not "WAT" or "CST")
 *
 * Even with these limitations, zonedb/zone_info.h shows that 270 out of a
 * total of 387 zones are supported by BasicZoneProcessor (as of v0.8).
 *
 * Not thread-safe.
 *
 * @tparam ZIS type of ZoneInfoStore, needed for implementations that require
 *    more complex brokers, and allows this template class to be independent
 *    of the exact type of the zone primary key
 * @tparam ZIB type of ZoneInfoBroker
 * @tparam ZEB type of ZoneEraBroker
 * @tparam ZPB type of ZonePolicyBroker
 * @tparam ZRB type of ZoneRuleBroker
 */
template <typename ZIS, typename ZIB, typename ZEB, typename ZPB, typename ZRB>
class BasicZoneProcessorTemplate: public ZoneProcessor {
  public:
    /** Exposed only for testing purposes. */
    typedef basic::TransitionTemplate<ZIB, ZEB, ZPB, ZRB> Transition;

    bool isLink() const override {
      return ! mZoneInfoBroker.targetInfo().isNull();
    }

    uint32_t getZoneId() const override {
      return mZoneInfoBroker.zoneId();
    }

    /**
     * @copydoc ZoneProcessor::findByLocalDateTime()
     *
     * The Transitions calculated by BasicZoneProcessor contain only the
     * epochSeconds when each transition occurs. They do not contain the local
     * date/time components of the transition. This design reduces the amount
     * of memory required by BasicZoneProcessor, but this means that the
     * information needed to implement this method correctly does not exist.
     *
     * The implementation is somewhat of a hack:
     *
     * 0) Use the localDateTime to extract the offset, *assuming* that the
     * localDatetime is UTC. This will get us within 12-14h of the correct
     * UTC offset.
     * 1) Use (localDateTime, offset0) to determine offset1.
     * 2) Use (localdateTime, offset1) to determine offset2.
     * 3) Finally, check if offset1 and offset2 are equal. If they are
     * we reached equilibrium so we can just return (localDateTime, offset1).
     * If they are not equal, then we have a cycle because the localDateTime
     * occurred in a DST gap (STD->DST transition) or overlap (DST->STD
     * transition). We arbitrarily pick the offset of the *later* epochSeconds
     * since that seems to match closely to what most people would expect to
     * happen in a gap or overlap (e.g. In the gap of 2am->3am, a 2:30am would
     * get shifted to 3:30am.)
     *
     * This algorithm will detect a FindResult::kTypeGap, but it will not be
     * able to distinguish between a kTypeExact and kTypeOverlap.
     */
    FindResult findByLocalDateTime(
        const LocalDateTime& ldt) const override {
      FindResult result;
      bool success = initForLocalDate(ldt.localDate());
      if (!success) return result;

      // 0) Use the UTC epochSeconds to get intial guess of offset.
      acetime_t epochSeconds0 = ldt.toEpochSeconds();
      auto result0 = findByEpochSeconds(epochSeconds0);
      if (result0.type == FindResult::kTypeNotFound) return result;
      auto offset0 = TimeOffset::forSeconds(
          result0.reqStdOffsetSeconds + result0.reqDstOffsetSeconds);

      // 1) Use offset0 to get the next epochSeconds and offset.
      auto odt = OffsetDateTime::forLocalDateTimeAndOffset(ldt, offset0);
      acetime_t epochSeconds1 = odt.toEpochSeconds();
      auto result1 = findByEpochSeconds(epochSeconds1);
      if (result1.type == FindResult::kTypeNotFound) return result;
      auto offset1 = TimeOffset::forSeconds(
          result1.reqStdOffsetSeconds + result1.reqDstOffsetSeconds);

      // 2) Use offset1 to get the next epochSeconds and offset.
      odt = OffsetDateTime::forLocalDateTimeAndOffset(ldt, offset1);
      acetime_t epochSeconds2 = odt.toEpochSeconds();
      auto result2 = findByEpochSeconds(epochSeconds2);
      if (result2.type == FindResult::kTypeNotFound) return result;
      auto offset2 = TimeOffset::forSeconds(
          result2.reqStdOffsetSeconds + result2.reqDstOffsetSeconds);

      // If offset1 and offset2 are equal, then we have an equilibrium
      // and odt(1) must equal odt(2).
      if (offset1 == offset2) {
        // I think this happens for kTypeExact or kTypeOverlap, but the current
        // algorithm cannot distinguish between the two, so let's pretend that
        // it is kTypeExact. Pick either of result1 or result2.
        result = result1;
      } else {
        // If the offsets don't match, then I think we have a kTypeGap.
        // Pick the stdOffset and dstOffset that generate the later epochSeconds
        // (the earlier transition), but convert into the LocalDateTime of the
        // earlier epochSeconds (the later transition).
        if (epochSeconds1 > epochSeconds2) {
          result = result1;
        } else {
          result = result2;
        }
        result.type = FindResult::kTypeGap;
      }

      return result;
    }

    FindResult findByEpochSeconds(acetime_t epochSeconds) const override {
      FindResult result;
      const Transition* transition = getTransition(epochSeconds);
      if (!transition) return result;

      result.dstOffsetSeconds = transition->deltaMinutes * kSecPerMin;
      result.stdOffsetSeconds = (transition->offsetMinutes
          - transition->deltaMinutes) * kSecPerMin;
      result.reqStdOffsetSeconds = result.stdOffsetSeconds;
      result.reqDstOffsetSeconds = result.dstOffsetSeconds;
      result.type = FindResult::kTypeExact;
      result.abbrev = transition->abbrev;

      return result;
    }

    void printNameTo(Print& printer) const override {
      mZoneInfoBroker.printNameTo(printer);
    }

    void printShortNameTo(Print& printer) const override {
      mZoneInfoBroker.printShortNameTo(printer);
    }

    void printTargetNameTo(Print& printer) const override {
      if (isLink()) {
        mZoneInfoBroker.targetInfo().printNameTo(printer);
      }
    }

    void setZoneKey(uintptr_t zoneKey) override {
      if (! mZoneInfoStore) return;
      if (mZoneInfoBroker.equals(zoneKey)) return;

      mZoneInfoBroker = mZoneInfoStore->createZoneInfoBroker(zoneKey);
      mYear = LocalDate::kInvalidYear;
      mNumTransitions = 0;
    }

    bool equalsZoneKey(uintptr_t zoneKey) const override {
      return mZoneInfoBroker.equals(zoneKey);
    }

    /** Used only for debugging. */
    void log() const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("BasicZoneProcessor:\n");
        logging::printf("  mEpochYear: %d\n", mEpochYear);
        logging::printf("  mYear: %d\n", mYear);
        logging::printf("  mNumTransitions: %d\n", mNumTransitions);
        for (int i = 0; i < mNumTransitions; i++) {
          logging::printf("  mT[%d]=", i);
          mTransitions[i].log();
        }
      }
    }

    /**
     * Set the zone info store at runtime. This is an advanced usage where the
     * custom subclass of ExtendedZoneProcessorTemplate does not know its zone
     * info store at compile time, so it must be set at runtime through this
     * method.
     */
    void setZoneInfoStore(const ZIS* zoneInfoStore) {
      mZoneInfoStore = zoneInfoStore;
    }

  protected:

    /**
     * Constructor. When first initialized inside a cache, the zoneInfoStore may
     * be set to nullptr, and the zoneKey should be ignored.
     *
     * @param type indentifier for the specific subclass of ZoneProcessor (e.g.
     *    Basic versus Extended) mostly used for debugging
     * @param zoneInfoStore pointer to a ZoneInfoStore that creates a ZIB
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    explicit BasicZoneProcessorTemplate(
        uint8_t type,
        const ZIS* zoneInfoStore /*nullable*/,
        uintptr_t zoneKey
    ) :
        ZoneProcessor(type),
        mZoneInfoStore(zoneInfoStore)
    {
      setZoneKey(zoneKey);
    }

  private:
    friend class ::BasicZoneProcessorTest_priorYearOfRule;
    friend class ::BasicZoneProcessorTest_compareRulesBeforeYear;
    friend class ::BasicZoneProcessorTest_findLatestPriorRule;
    friend class ::BasicZoneProcessorTest_findZoneEra;
    friend class ::BasicZoneProcessorTest_init_primitives;
    friend class ::BasicZoneProcessorTest_initForLocalDate;
    friend class ::BasicZoneProcessorTest_setZoneKey;
    friend class ::BasicZoneProcessorTest_createAbbreviation;
    friend class ::BasicZoneProcessorTest_calcRuleOffsetMinutes;

    /**
     * Maximum size of Transition cache across supported zones. This number (5)
     * is derived from the following:
     *
     *    * 1 transition prior to the current year
     *    * 1 transition at the start of the current year if the zone
     *      switches to a new ZoneEra (e.g. into a new ZonePolicy)
     *    * 2 DST transitions (spring and autumn)
     *    * 1 transition at start of the next year
     */
    static const uint8_t kMaxCacheEntries = 5;

    /**
     * The smallest Transition.startEpochSeconds which represents -Infinity.
     * Can't use INT32_MIN because that is used internally to indicate
     * "invalid".
     */
    static const acetime_t kMinEpochSeconds = INT32_MIN + 1;

    // Disable copy constructor and assignment operator.
    BasicZoneProcessorTemplate(const BasicZoneProcessorTemplate&) = delete;
    BasicZoneProcessorTemplate& operator=(const BasicZoneProcessorTemplate&) =
        delete;

    bool equals(const ZoneProcessor& other) const override {
      return mZoneInfoBroker.equals(
          ((const BasicZoneProcessorTemplate&) other).mZoneInfoBroker);
    }

    /** Return the Transition at the given epochSeconds. */
    const Transition* getTransition(acetime_t epochSeconds) const {
      bool success = initForEpochSeconds(epochSeconds);
      return (success) ? findMatch(epochSeconds) : nullptr;
    }

    /**
     * Initialize the transition cache, keyed by the "current" year. The
     * current year is not always the year determined by the UTC time of
     * the epoch seconds. If the UTC date is 1/1 (Jan 1), the "current" year
     * is set to be the previous year as explained below.
     *
     * There are some countries that decided to make a time zone change at on
     * 12/31 (e.g. Asia/Dhaka), which means that determining the correct DST
     * offset on 1/1 requires the Transitions from the previous year. To
     * support these zones, if the UTC date is 1/1, then we force the
     * transition cache to be generated using the *previous* year. This
     * workaround will fail for zones which have DST transitions on 1/1.
     * Therefore, the zone_info.h generator (AceTimeTools/tzcompiler.py) removes
     * all zones which have time zone transitions on 1/1 from the list of
     * supported zones.
     *
     * The high level algorithm for determining the DST transitions is as
     * follows:
     *
     *  1. Find the last ZoneRule that was active just before the current year.
     *  2. Find the ZoneRules which are active in the current year.
     *  3. Calculate the Transitions given the above ZoneRules.
     *  4. Calculate the zone abbreviations (e.g. "PDT" or "BST") for each
     *  Transition.
     *
     * Returns success status: true if successful, false if an error occurred
     * (e.g. out of bounds).
     */
    bool initForLocalDate(const LocalDate& ld) const {
      int16_t year = ld.year();
      if (ld.month() == 1 && ld.day() == 1) {
        year--;
      }
      // Restrict to [1,9999], even though LocalDate should be able to handle
      // [0,10000].
      if (year <= LocalDate::kMinYear || LocalDate::kMaxYear <= year) {
        return false;
      }

      if (isFilled(year)) return true;
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("initForLocalDate(): %d (new year %d)\n",
            ld.year(), year);
      }

      mYear = year;
      mEpochYear = Epoch::currentEpochYear();
      mNumTransitions = 0; // clear cache

      ZEB priorEra = addTransitionPriorToYear(year);
      ZEB currentEra = addTransitionsForYear(year, priorEra);
      addTransitionAfterYear(year, currentEra);
      calcTransitions();
      calcAbbreviations();

      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        log();
      }

      return true;
    }

    /**
     * Initialize the transition cache, by converting the epochSeconds to
     * year-month-day in UTC, then calling initForLocalDate() with the 'year'
     * component.
     */
    bool initForEpochSeconds(acetime_t epochSeconds) const {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      return initForLocalDate(ld);
    }

    /**
     * Add the last matching rule just prior to the given year. This determines
     * the offset at the beginning of the current year.
     *
     * @return the ZoneEra of the previous year
     */
    ZEB addTransitionPriorToYear(int16_t year) const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addTransitionPriorToYear(): %d\n", year);
      }

      const ZEB era = findZoneEra(mZoneInfoBroker, year - 1);

      // If the prior ZoneEra has a ZonePolicy), then find the latest rule
      // within the ZoneEra. Otherwise, add a Transition using a rule==nullptr.
      ZRB latest = findLatestPriorRule(era.zonePolicy(), year);
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addTransitionsPriorToYear(): adding latest prior ");
        if (latest.isNull()) {
          logging::printf("ZR(null)\n");
        } else {
          logging::printf("ZR[%d,%d]\n", latest.fromYear(), latest.toYear());
        }
      }
      addTransition(year - 1, 0 /*month*/, era, latest);

      return era;
    }

    /**
     * Find the latest rule in the ZonePolicy in effective before the given
     * year. Assume that there are no more than 1 rule per month.
     * Return null ZoneRule if ZonePoicy is null.
     */
    static ZRB findLatestPriorRule(const ZPB& zonePolicy, int16_t year) {
      ZRB latest;
      if (zonePolicy.isNull()) return latest;

      uint8_t numRules = zonePolicy.numRules();
      for (uint8_t i = 0; i < numRules; i++) {
        const ZRB rule = zonePolicy.rule(i);
        // Check if rule is effective prior to the given year
        if (rule.fromYear() < year) {
          if ((latest.isNull()) ||
              compareRulesBeforeYear(year, rule, latest) > 0) {
            latest = rule;
          }
        }
      }

      return latest;
    }

    /** Compare two ZoneRules which are valid *prior* to the given year. */
    static int8_t compareRulesBeforeYear(
        int16_t year, const ZRB& a, const ZRB& b) {
      return basic::compareYearMonth(
          priorYearOfRule(year, a), a.inMonth(),
          priorYearOfRule(year, b), b.inMonth());
    }

    /**
     * Return the largest effective year of the rule *prior* to given year. It
     * is assumed that the caller has already verified that
     * rule.fromYear() < year, so we only need to check 2 cases:
     *
     *    * If [from,to]<year, return (to).
     *    * Else we know [from<year<=to], so return (year-1).
     */
    static int16_t priorYearOfRule(int16_t year, const ZRB& rule) {
      if (rule.toYear() < year) {
        return rule.toYear();
      }
      return year - 1;
    }

    /**
     * Add all matching transitions from the current year.
     * @return the ZoneEra of the current year.
     */
    ZEB addTransitionsForYear(int16_t year, const ZEB& priorEra) const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addTransitionsForYear(): %d\n", year);
      }

      const ZEB era = findZoneEra(mZoneInfoBroker, year);

      // If the ZonePolicy has no rules, then add a Transition which takes
      // effect at the start time of the current year.
      const ZPB zonePolicy = era.zonePolicy();
      if (zonePolicy.isNull()) {
        if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
          logging::printf("addTransitionsForYear(): adding ZE.untilY=%d\n",
              era.untilYear());
        }
        addTransition(year, 0 /*month*/, era, ZRB());
        return era;
      }

      if (! era.equals(priorEra)) {
        // The ZoneEra has changed, so we need to find the Rule in effect at
        // the start of the current year of the current ZoneEra. This may be a
        // rule far in the past, but shift the rule forward to {year, 1, 1}.
        ZRB latestPrior = findLatestPriorRule(era.zonePolicy(), year);
        if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
              "addTransitionsForYear(): adding latest prior ");
          if (latestPrior.isNull()) {
            logging::printf("ZR(null)\n");
          } else {
            logging::printf("ZR[%d,%d]\n",
              latestPrior.fromYear(), latestPrior.toYear());
          }
        }
        addTransition(year, 1 /*month*/, era, latestPrior);
      }

      // Find all directly matching transitions (i.e. the [from, to] overlap
      // with the current year) and add them to mTransitions, in sorted order
      // according to the ZoneRule::inMonth field.
      uint8_t numRules = zonePolicy.numRules();
      for (uint8_t i = 0; i < numRules; i++) {
        const ZRB rule = zonePolicy.rule(i);
        if ((rule.fromYear() <= year) && (year <= rule.toYear())) {
          if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
            logging::printf(
                "addTransitionsForYear(): adding rule ");
            if (rule.isNull()) {
              logging::printf("ZR(null)\n");
            } else {
              logging::printf("ZR[%d,%d]\n", rule.fromYear(), rule.toYear());
            }
          }
          addTransition(year, 0 /*month*/, era, rule);
        }
      }

      return era;
    }

    /** Add the rule just after the current year if there exists one. */
    void addTransitionAfterYear(int16_t year, const ZEB& currentEra) const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addTransitionAfterYear(): %d\n", year);
      }

      const ZEB eraAfter = findZoneEra(mZoneInfoBroker, year + 1);

      // If the current era is the same as the following year, then we'll just
      // assume that the latest ZoneRule carries over to Jan 1st of the next
      // year. tzcompiler.py guarantees no ZoneRule occurs on Jan 1st.
      if (currentEra.equals(eraAfter)) {
        return;
      }

      // If the ZoneEra did change, find the latest transition prior to
      // {year + 1, 1, 1}, then shift that Transition to Jan 1st of the
      // following year.
      ZRB latest = findLatestPriorRule(eraAfter.zonePolicy(), year + 1);
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf(
            "addTransitionsAfterYear(): adding latest prior ");
        if (latest.isNull()) {
          logging::printf("ZR(null)\n");
        } else {
          logging::printf("ZR[%d,%d]\n", latest.fromYear(), latest.toYear());
        }
      }
      addTransition(year + 1, 1 /*month*/, eraAfter, latest);
    }

    /**
     * Add the Transition(era, rule) to the mTransitions cache, in sorted order
     * according to the 'ZoneRule::inMonth' field. This assumes that there are
     * no more than one transition per month, so tzcompiler.py removes
     * ZonePolicies which have multiple transitions in one month (e.g. Egypt,
     * Palestine, Spain, Tunisia).
     *
     * Essentially, this method is doing an Insertion Sort of the Transition
     * elements. Even through it is O(N^2), for small number of Transition
     * elements, this is faster than the O(N log(N)) sorting algorithms. The
     * nice property of this Insertion Sort is that if the ZoneRules are
     * already sorted (they are mostly sorted), then the loop terminates early
     * and the total sort time is O(N).
     *
     * @param year create the transition for this year (era, rule)
     * @param month create the transition at this month. If set to 0, infer the
     *    month from the rule using the expression ((rule) ? rule.inMonth() :
     *    1).
     * @param era the ZoneEra which defined this transition, used to extract
     *    the offsetSeconds() and deltaSeconds()
     * @param rule the ZoneRule which defined this transition, used to
     *    extract deltaSeconds(), letter()
     */
    void addTransition(int16_t year, uint8_t month, const ZEB& era,
          const ZRB& rule) const {

      // If a zone needs more transitions than kMaxCacheEntries, the check below
      // will cause the DST transition information to be inaccurate, and it is
      // highly likely that this situation would be caught in the
      // AceTimeValidation tests. Since these integration tests pass, I feel
      // confident that those zones which need more than kMaxCacheEntries are
      // already filtered out by tzcompiler.py.
      //
      // Ideally, the tzcompiler.py script would explicitly remove those zones
      // which need more than kMaxCacheEntries Transitions. But this would
      // require a Python version of the BasicZoneProcessor, and unfortunately,
      // zone_processor.py implements only the ExtendedZoneProcessor algorithm
      // An early version of zone_processor.py may have implemented something
      // close to BasicZoneProcessor, and it may be available in the git
      // history. But it seems like too much work right now to try to dig that
      // out, just to implement the explicit check for kMaxCacheEntries. It
      // would mean maintaining another version of zone_processor.py.
      if (mNumTransitions >= kMaxCacheEntries) return;

      // Insert new element at the end of the list.
      // NOTE: It is probably tempting to pass a pointer (or reference) to
      // mTransitions[mNumTransitions] into createTransition(), instead of
      // returning it by value. However, MemoryBenchmark shows that directly
      // updating the Transition through the pointer increases flash memory
      // consumption by ~110 bytes on AVR processors. It seems that creating a
      // local copy of Transition on the stack, filling it, and then copying it
      // by value takes fewer instructions.
      mTransitions[mNumTransitions] = createTransition(year, month, era, rule);
      mNumTransitions++;

      // perform an insertion sort based on ZoneRule.inMonth()
      for (uint8_t i = mNumTransitions - 1; i > 0; i--) {
        Transition& left = mTransitions[i - 1];
        Transition& right = mTransitions[i];
        // assume only 1 rule per month
        if (basic::compareYearMonth(left.year, left.month,
            right.year, right.month) > 0) {
          Transition tmp = left;
          left = right;
          right = tmp;
        }
      }
    }

    /**
     * Create a Transition with the 'deltaSeconds' and 'offsetSeconds' filled
     * in so that subsequent processing does not need to retrieve those again
     * (potentially from PROGMEM).
     */
    static Transition createTransition(int16_t year, uint8_t month,
        const ZEB& era, const ZRB& rule) {

      Transition transition;
      int16_t deltaMinutes;
      uint8_t mon;
      if (rule.isNull()) {
        mon = 1; // RULES is either '-' or 'hh:mm' so takes effect in Jan
        deltaMinutes = era.deltaSeconds() / kSecPerMin;
        transition.abbrev[0] = '\0';
      } else {
        mon = rule.inMonth();
        deltaMinutes = rule.deltaSeconds() / kSecPerMin;
        ace_common::strncpy_T(
            transition.abbrev, rule.letter(), internal::kAbbrevSize - 1);
        transition.abbrev[internal::kAbbrevSize - 1] = '\0';
      }
      // Clobber the month if specified.
      if (month != 0) {
        mon = month;
      }
      int16_t offsetMinutes = era.offsetSeconds() / kSecPerMin + deltaMinutes;

      transition.era = era;
      transition.rule = rule;
      transition.startEpochSeconds = 0;
      transition.offsetMinutes = offsetMinutes;
      transition.deltaMinutes = deltaMinutes;
      transition.year = year;
      transition.month = mon;
      return transition;
    }

    /**
     * Find the ZoneEra which applies to the given year. The era will satisfy
     * (year < ZoneEra.untilYear).
     */
    static ZEB findZoneEra(const ZIB& info, int16_t year) {
      for (uint8_t i = 0; i < info.numEras(); i++) {
        const ZEB era = info.era(i);
        if (year < era.untilYear()) return era;
      }
      // Return the last ZoneEra if we run off the end.
      return info.era(info.numEras() - 1);
    }

    /**
     * Calculate the startEpochSeconds of each Transition. (previous, this also
     * calculated the offsetSeconds and deltaSeconds as well, but it turned out
     * that they could be calculated early in createTransition()). The start
     * time of a given transition is defined as the "wall clock", which means
     * that it is defined in terms of the *previous* Transition.
     */
    void calcTransitions() const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("calcTransitions():\n");
      }

      // Set the initial startEpochSeconds to be -Infinity
      Transition* prevTransition = &mTransitions[0];
      prevTransition->startEpochSeconds = kMinEpochSeconds;

      for (uint8_t i = 1; i < mNumTransitions; i++) {
        Transition& transition = mTransitions[i];
        const int16_t year = transition.year;

        if (transition.rule.isNull()) {
          // If the transition is simple (has no named rule), then the
          // ZoneEra applies for the entire year (since BasicZoneProcessor
          // supports only whole year in the UNTIL field). The whole year UNTIL
          // field has an implied 'w' suffix on 00:00, we don't need to call
          // calcRuleOffsetMinutes() with a 'w', we can just use the previous
          // transition's offset to calculate the startDateTime of this
          // transition.
          //
          // Also, when transition.rule == nullptr, the mNumTransitions should
          // be 1, since only a single transition is added by
          // addTransitionsForYear().
          const int16_t prevOffsetMinutes = prevTransition->offsetMinutes;
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, 1, 1, 0, 0, 0,
              TimeOffset::forMinutes(prevOffsetMinutes));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();
        } else {
          // In this case, the transition points to a named ZonePolicy, which
          // means that there could be multiple ZoneRules associated with the
          // given year. For each transition, determine the startEpochSeconds,
          // and the effective offset code.

          // Determine the start date of the rule.
          const internal::MonthDay monthDay = internal::calcStartDayOfMonth(
              year, transition.month, transition.rule.onDayOfWeek(),
              transition.rule.onDayOfMonth());

          // Determine the offset of the 'atTimeSuffix'. The 'w' suffix
          // requires the offset of the previous transition.
          const int16_t prevOffsetMinutes = calcRuleOffsetMinutes(
              prevTransition->offsetMinutes,
              transition.era.offsetSeconds() / kSecPerMin,
              transition.rule.atTimeSuffix());

          // startDateTime
          const uint16_t minutes = transition.rule.atTimeSeconds() / 60;
          const uint8_t atHour = minutes / 60;
          const uint8_t atMinute = minutes % 60;
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, monthDay.month, monthDay.day,
              atHour, atMinute, 0 /*second*/,
              TimeOffset::forMinutes(prevOffsetMinutes));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();
        }

        prevTransition = &transition;
      }
    }

    /**
     * Determine the offset of the 'atTimeSuffix'. If 'w', then we must use the
     * offset of the *previous* zone rule. If 's', use the current base offset
     * (which does not contain the extra DST offset). If 'u', 'g', 'z', then
     * use 0 offset.
     */
    static int16_t calcRuleOffsetMinutes(int16_t prevEffectiveOffsetMinutes,
        int16_t currentBaseOffsetMinutes, uint8_t atSuffix) {
      if (atSuffix == basic::ZoneContext::kSuffixW) {
        return prevEffectiveOffsetMinutes;
      } else if (atSuffix == basic::ZoneContext::kSuffixS) {
        return currentBaseOffsetMinutes;
      } else { // 'u', 'g' or 'z'
        return 0;
      }
    }

    /** Determine the time zone abbreviations. */
    void calcAbbreviations() const {
      if (ACE_TIME_BASIC_ZONE_PROCESSOR_DEBUG) {
        logging::printf("calcAbbreviations():\n");
      }

      for (uint8_t i = 0; i < mNumTransitions; i++) {
        calcAbbreviation(&mTransitions[i]);
      }
    }

    /** Calculate the time zone abbreviation of the current transition. */
    static void calcAbbreviation(Transition* transition) {
      internal::createAbbreviation(
          transition->abbrev,
          internal::kAbbrevSize,
          transition->era.format(),
          transition->deltaMinutes,
          transition->abbrev);
    }

    /** Search the cache and find closest Transition. */
    const Transition* findMatch(acetime_t epochSeconds) const {
      const Transition* closestMatch = nullptr;
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        const Transition* m = &mTransitions[i];
        if (closestMatch == nullptr || m->startEpochSeconds <= epochSeconds) {
          closestMatch = m;
        }
      }
      return closestMatch;
    }

  private:
    static const int32_t kSecPerMin = 60;

    const ZIS* mZoneInfoStore; // nullable
    ZIB mZoneInfoBroker;

    mutable uint8_t mNumTransitions = 0;
    mutable Transition mTransitions[kMaxCacheEntries];
};

/**
 * A specific implementation of BasicZoneProcessorTemplate that uses
 * ZoneXxxBrokers which read from zonedb files in PROGMEM flash memory.
 */
class BasicZoneProcessor: public BasicZoneProcessorTemplate<
    basic::ZoneInfoStore,
    basic::ZoneInfoBroker,
    basic::ZoneEraBroker,
    basic::ZonePolicyBroker,
    basic::ZoneRuleBroker> {

  public:
    /** Unique TimeZone type identifier for BasicZoneProcessor. */
    static const uint8_t kTypeBasic = 3;

    explicit BasicZoneProcessor(const basic::ZoneInfo* zoneInfo = nullptr)
      : BasicZoneProcessorTemplate<
          basic::ZoneInfoStore,
          basic::ZoneInfoBroker,
          basic::ZoneEraBroker,
          basic::ZonePolicyBroker,
          basic::ZoneRuleBroker>(
              kTypeBasic, &mZoneInfoStore, (uintptr_t) zoneInfo)
    {}

  private:
    basic::ZoneInfoStore mZoneInfoStore;
};

} // namespace ace_time

#endif
