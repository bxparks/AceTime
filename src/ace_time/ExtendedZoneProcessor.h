/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_H
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_H

#include <stdint.h> // uintptr_t
#include <AceCommon.h> // copyReplaceString()
#include "../zoneinfo/infos.h"
#include "../zoneinfo/brokers.h"
#include "common/common.h" // kAbbrevSize
#include "common/logging.h"
#include "LocalDate.h"
#include "ZoneProcessor.h"
#include "Transition.h"

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG 0
#endif

class ExtendedZoneProcessorTest_compareEraToYearMonth;
class ExtendedZoneProcessorTest_compareEraToYearMonth2;
class ExtendedZoneProcessorTest_createMatchingEra;
class ExtendedZoneProcessorTest_findMatches_simple;
class ExtendedZoneProcessorTest_findMatches_named;
class ExtendedZoneProcessorTest_findCandidateTransitions;
class ExtendedZoneProcessorTest_createTransitionsFromNamedMatch;
class ExtendedZoneProcessorTest_getTransitionTime;
class ExtendedZoneProcessorTest_createTransitionForYear;
class ExtendedZoneProcessorTest_calcInteriorYears;
class ExtendedZoneProcessorTest_getMostRecentPriorYear;
class ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
class ExtendedZoneProcessorTest_compareTransitionToMatch;
class ExtendedZoneProcessorTest_processTransitionCompareStatus;
class ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
class ExtendedZoneProcessorTest_setZoneKey;
class ExtendedTransitionValidation;
class CompleteTransitionValidation;

class Print;

namespace ace_time {

namespace extended {

/** A simple tuple to represent a year/month pair. */
struct YearMonthTuple {
  int16_t year;
  uint8_t month;
};

}

/**
 * An implementation of ZoneProcessor that supports for *all* zones defined by
 * the TZ Database. The supported zones are defined in the zonedbx/zone_infos.h
 * header file. The constructor expects a pointer to one of the ZoneInfo
 * structures declared in the zonedbx/zone_infos.h file. The zone_processor.py
 * file is the initial Python implementation of this class, which got
 * translated into C++.
 *
 * The underlying zoneinfo files (extended::ZoneInfo, etc) store the UTC and DST
 * offsets of a timezone as a single signed byte in 15-minute increments. This
 * is sufficient to accurate describe all time zones from the year 2000 until
 * 2100. The AT and UNTIL transition times are stored using a 1-minute
 * resolution, which correctly handles the 5 timezones whose DST transition
 * times occur at 00:01. Those zones are:
 *
 *    - America/Goose_Bay
 *    - America/Moncton
 *    - America/St_Johns
 *    - Asia/Gaza
 *    - Asia/Hebron
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
class ExtendedZoneProcessorTemplate: public ZoneProcessor {
  public:
    /**
     * Max number of Transitions required for all Zones supported by this class.
     * This includes the most recent prior Transition. The max transitions for
     * each Zone is given by the kZoneBufSize{zoneName} constant in the
     * generated `zonedb[x]/zone_infos.h` file. The maximum over all zones is
     * given in the 'MaxBufSize' comment in the `zone_infos.h` file. Currently
     * that overall maximum is 7, which has been verified by various tests (e.g.
     * HinnantExtendedTest, DateUtilExtendedTest, JavaExtendedTest, and
     * AcetzExtendedTest) in the AceTimeValidation project. We set this to one
     * more than 7 for safety.
     */
    static const uint8_t kMaxTransitions = 8;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionTemplate<ZEB, ZPB, ZRB> Transition;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionForSecondsTemplate<ZEB, ZPB, ZRB>
        TransitionForSeconds;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionForDateTimeTemplate<ZEB, ZPB, ZRB>
        TransitionForDateTime;

    /** Exposed only for testing purposes. */
    typedef extended::MatchingEraTemplate<ZEB> MatchingEra;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionStorageTemplate<kMaxTransitions, ZEB, ZPB, ZRB>
        TransitionStorage;

    bool isLink() const override {
      return ! mZoneInfoBroker.targetInfo().isNull();
    }

    uint32_t getZoneId() const override {
      return mZoneInfoBroker.zoneId();
    }

    FindResult findByLocalDateTime(const LocalDateTime& ldt) const override {
      FindResult result;

      bool success = initForYear(ldt.year());
      if (! success) {
        return result;
      }

      // Find the Transition(s) in the gap or overlap.
      TransitionForDateTime transitionForDateTime =
          mTransitionStorage.findTransitionForDateTime(ldt);

      // Extract the target Transition, depending on the requested ldt.fold
      // and the result.num.
      const Transition* transition;
      if (transitionForDateTime.num == 1) {
        transition = transitionForDateTime.curr;
        result.type = FindResult::kTypeExact;
        result.reqStdOffsetSeconds = transition->offsetSeconds;
        result.reqDstOffsetSeconds = transition->deltaSeconds;
      } else { // num = 0 or 2
        if (transitionForDateTime.prev == nullptr
            || transitionForDateTime.curr == nullptr) {
          // ldt was far past or far future
          transition = nullptr;
          result.type = FindResult::kTypeNotFound;
        } else { // gap or overlap
          if (transitionForDateTime.num == 0) { // num==0, Gap
            result.type = FindResult::kTypeGap;
            if (ldt.fold() == 0) {
              // ldt wants to use the 'prev' transition to convert to
              // epochSeconds.
              result.reqStdOffsetSeconds =
                  transitionForDateTime.prev->offsetSeconds;
              result.reqDstOffsetSeconds =
                  transitionForDateTime.prev->deltaSeconds;
              // But after normalization, it will be shifted into the curr
              // transition, so select 'curr' as the target transition.
              transition = transitionForDateTime.curr;
            } else {
              // ldt wants to use the 'curr' transition to convert to
              // epochSeconds.
              result.reqStdOffsetSeconds =
                  transitionForDateTime.curr->offsetSeconds;
              result.reqDstOffsetSeconds =
                  transitionForDateTime.curr->deltaSeconds;
              // But after normalization, it will be shifted into the prev
              // transition, so select 'prev' as the target transition.
              transition = transitionForDateTime.prev;
            }
          } else { // num==2, Overlap
            transition = (ldt.fold() == 0)
                ? transitionForDateTime.prev
                : transitionForDateTime.curr;
            result.type = FindResult::kTypeOverlap;
            result.reqStdOffsetSeconds = transition->offsetSeconds;
            result.reqDstOffsetSeconds = transition->deltaSeconds;
            result.fold = ldt.fold();
          }
        }
      }

      if (! transition) {
        return result;
      }

      result.stdOffsetSeconds = transition->offsetSeconds;
      result.dstOffsetSeconds = transition->deltaSeconds;
      result.abbrev = transition->abbrev;

      return result;
    }

    /**
     * @copydoc ZoneProcessor::findByEpochSeconds(acetime_t)
     *
     * This implementation calculates the `OffsetDateTime.fold()` parameter
     * correctly, and indicates whether the localized datetime is before the
     * overlap (fold==0) or after the overlap (fold==1). During a gap, there is
     * no ambiguity when searching on epochSeconds so fold will always be 0.
     */
    FindResult findByEpochSeconds(acetime_t epochSeconds) const override {
      FindResult result;
      bool success = initForEpochSeconds(epochSeconds);
      if (!success) return result;

      TransitionForSeconds transitionForSeconds =
          mTransitionStorage.findTransitionForSeconds(epochSeconds);
      const Transition* transition = transitionForSeconds.curr;
      if (!transition) return result;

      result.stdOffsetSeconds = transition->offsetSeconds;
      result.dstOffsetSeconds = transition->deltaSeconds;
      result.reqStdOffsetSeconds = transition->offsetSeconds;
      result.reqDstOffsetSeconds = transition->deltaSeconds;
      result.abbrev = transition->abbrev;
      result.fold = transitionForSeconds.fold;
      if (transitionForSeconds.num == 2) {
        result.type = FindResult::kTypeOverlap;
      } else {
        result.type = FindResult::kTypeExact;
      }
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

    /** Used only for debugging. */
    void log() const {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("ExtendedZoneProcessor:\n");
        logging::printf("  mEpochYear: %d\n", mEpochYear);
        logging::printf("  mYear: %d\n", mYear);
        logging::printf("  mNumMatches: %d\n", mNumMatches);
        for (int i = 0; i < mNumMatches; i++) {
          logging::printf("  Match %d: ", i);
          mMatches[i].log();
          logging::printf("\n");
        }
        mTransitionStorage.log();
      }
    }

    /** Reset the TransitionStorage high water mark. For debugging. */
    void resetTransitionAllocSize() {
      mTransitionStorage.resetAllocSize();
    }

    /** Get the largest allocation size of TransitionStorage. For debugging. */
    uint8_t getTransitionAllocSize() const {
      return mTransitionStorage.getAllocSize();
    }

    void setZoneKey(uintptr_t zoneKey) override {
      if (! mZoneInfoStore) return;
      if (mZoneInfoBroker.equals(zoneKey)) return;

      mZoneInfoBroker = mZoneInfoStore->createZoneInfoBroker(zoneKey);
      mYear = LocalDate::kInvalidYear;
      mNumMatches = 0;
      resetTransitionAllocSize(); // clear the alloc size for new zone
    }

    bool equalsZoneKey(uintptr_t zoneKey) const override {
      return mZoneInfoBroker.equals(zoneKey);
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

    /**
     * Initialize using the epochSeconds.  The epochSeconds is converted to
     * the LocalDate for UTC time, and the year is used to call initForYear().
     * Exposed for debugging.
     */
    bool initForEpochSeconds(acetime_t epochSeconds) const {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      return initForYear(ld.year());
    }

    /**
     * Initialize the zone rules cache, keyed by the "current" year.
     * Returns success status: true if successful, false if an error occurred.
     * Exposed for debugging.
     */
    bool initForYear(int16_t year) const {
      // Restrict to [1,9999] even though LocalDate should be able to handle
      // [0,10000].
      if (year <= LocalDate::kMinYear || LocalDate::kMaxYear <= year) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
              "initForYear(): Year %d outside range [%d, %d]\n",
              year, LocalDate::kMinYear + 1, LocalDate::kMaxYear - 1);
        }
        return false;
      }

      if (isFilled(year)) return true;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("initForYear(): %d\n", year);
      }
      mYear = year;
      mEpochYear = Epoch::currentEpochYear();
      mNumMatches = 0; // clear cache
      mTransitionStorage.init();

      // Fill transitions over a 14-month window straddling the given year.
      extended::YearMonthTuple startYm = { (int16_t) (year - 1), 12 };
      extended::YearMonthTuple untilYm =  { (int16_t) (year + 1), 2 };

      // Step 1. The equivalent steps for the Python version are in the
      // acetimepy project, under zone_processor.ZoneProcessor.init_for_year().
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("==== Step 1: findMatches()\n");
      }
      mNumMatches = findMatches(mZoneInfoBroker, startYm, untilYm, mMatches,
          kMaxMatches);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 2
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("==== Step 2: createTransitions()\n");
      }
      createTransitions(mTransitionStorage, mMatches, mNumMatches);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 3
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("==== Step 3: fixTransitionTimes()\n");
      }
      Transition** begin = mTransitionStorage.getActivePoolBegin();
      Transition** end = mTransitionStorage.getActivePoolEnd();
      fixTransitionTimes(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 4
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("==== Step 4: generateStartUntilTimes()\n");
      }
      generateStartUntilTimes(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 5
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("==== Step 5: calcAbbreviations()\n");
      }
      calcAbbreviations(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      return true;
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
    explicit ExtendedZoneProcessorTemplate(
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
    friend class ::ExtendedZoneProcessorTest_compareEraToYearMonth;
    friend class ::ExtendedZoneProcessorTest_compareEraToYearMonth2;
    friend class ::ExtendedZoneProcessorTest_createMatchingEra;
    friend class ::ExtendedZoneProcessorTest_findMatches_simple;
    friend class ::ExtendedZoneProcessorTest_findMatches_named;
    friend class ::ExtendedZoneProcessorTest_findCandidateTransitions;
    friend class ::ExtendedZoneProcessorTest_createTransitionsFromNamedMatch;
    friend class ::ExtendedZoneProcessorTest_getTransitionTime;
    friend class ::ExtendedZoneProcessorTest_createTransitionForYear;
    friend class ::ExtendedZoneProcessorTest_calcInteriorYears;
    friend class ::ExtendedZoneProcessorTest_getMostRecentPriorYear;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatch;
    friend class ::ExtendedZoneProcessorTest_processTransitionCompareStatus;
    friend class ::ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
    friend class ::ExtendedZoneProcessorTest_setZoneKey;
    friend class ::ExtendedTransitionValidation;
    friend class ::CompleteTransitionValidation;

    // Disable copy constructor and assignment operator.
    ExtendedZoneProcessorTemplate(
        const ExtendedZoneProcessorTemplate&) = delete;
    ExtendedZoneProcessorTemplate& operator=(
        const ExtendedZoneProcessorTemplate&) = delete;

    /**
     * Number of Extended Matches. We look at the 3 years straddling the current
     * year, plus the most recent prior year, so that makes 4.
     */
    static const uint8_t kMaxMatches = 4;

    /**
     * Maximum number of interior years. For a viewing window of 14 months,
     * this will be 4. (Verify: I think this can be changed to 3.)
     */
    static const uint8_t kMaxInteriorYears = 4;

    bool equals(const ZoneProcessor& other) const override {
      return mZoneInfoBroker.equals(
          ((const ExtendedZoneProcessorTemplate&) other).mZoneInfoBroker);
    }

    /**
     * Find the ZoneEras which overlap [startYm, untilYm), ignoring day, time
     * and timeSuffix. The start and until fields of the ZoneEra are
     * truncated at the low and high end by startYm and untilYm, respectively.
     * Each matching ZoneEra is wrapped inside a MatchingEra object, placed in
     * the 'matches' array, and the number of matches is returned.
     */
    static uint8_t findMatches(
        const ZIB& zoneInfo,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm,
        MatchingEra* matches,
        uint8_t maxMatches
    ) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findMatches()\n");
      }
      uint8_t iMatch = 0;
      MatchingEra* prevMatch = nullptr;
      for (uint8_t iEra = 0; iEra < zoneInfo.numEras(); iEra++) {
        const ZEB era = zoneInfo.era(iEra);
        if (eraOverlapsInterval(prevMatch, era, startYm, untilYm)) {
          if (iMatch < maxMatches) {
            matches[iMatch] = createMatchingEra(
                prevMatch, era, startYm, untilYm);
            prevMatch = &matches[iMatch];
            iMatch++;
          }
        }
      }
      return iMatch;
    }

    /**
     * Determines if `era` overlaps the interval defined by `[startYm,
     * untilYm)`.
     *
     * The start date of the current era is defined by the UNTIL fields of the
     * previous era. The interval of current era is `[prev.until, era.until)`.
     * This function determines if the two intervals overlap.
     *
     * @code
     *         start          until
     *           [              )
     * -------------)[--------------)[-----------------
     *          prev.until       era.until
     *
     * @endcode
     *
     * The 2 intervals overlap if:
     *
     * @code
     * (prev.until < until) && (era.until > start)
     * @endcode
     *
     * If `prev == nil`, then `prev.until` is assigned to be `-infinity`, so the
     * `era` extends back to earliest possible time.
     */
    static bool eraOverlapsInterval(
        const MatchingEra* prevMatch,
        const ZEB& era,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm) {
      return (prevMatch == nullptr || compareEraToYearMonth(
              prevMatch->era, untilYm.year, untilYm.month) < 0)
          && compareEraToYearMonth(era, startYm.year, startYm.month) > 0;
    }

    /** Return (1, 0, -1) depending on how era compares to (year, month). */
    static int8_t compareEraToYearMonth(const ZEB& era,
        int16_t year, uint8_t month) {
      if (era.untilYear() < year) return -1;
      if (era.untilYear() > year) return 1;
      if (era.untilMonth() < month) return -1;
      if (era.untilMonth() > month) return 1;
      if (era.untilDay() > 1) return 1;
      //if (era.untilTimeSeconds() < 0) return -1; // never possible
      if (era.untilTimeSeconds() > 0) return 1;
      return 0;
    }

    /**
     * Create a MatchingEra object around the 'era' which intersects the
     * half-open [startYm, untilYm) interval. The interval is assumed to overlap
     * the ZoneEra using the eraOverlapsInterval() method. The 'prev' ZoneEra is
     * needed to define the startDateTime of the current era.
     */
    static MatchingEra createMatchingEra(
        MatchingEra* prevMatch,
        const ZEB& era,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm) {

      // If prevMatch is null, set startDate to be earlier than all valid
      // ZoneEra.
      extended::DateTuple startDate = (prevMatch == nullptr)
          ? extended::DateTuple{
              LocalDate::kInvalidYear,
              1,
              1,
              0,
              extended::ZoneContext::kSuffixW
            }
          : extended::DateTuple{
              prevMatch->era.untilYear(),
              prevMatch->era.untilMonth(),
              prevMatch->era.untilDay(),
              (int32_t) prevMatch->era.untilTimeSeconds(),
              prevMatch->era.untilTimeSuffix()
            };
      extended::DateTuple lowerBound{
        startYm.year,
        startYm.month,
        1,
        0,
        extended::ZoneContext::kSuffixW
      };
      if (startDate < lowerBound) {
        startDate = lowerBound;
      }

      extended::DateTuple untilDate{
        era.untilYear(),
        era.untilMonth(),
        era.untilDay(),
        (int32_t) era.untilTimeSeconds(),
        era.untilTimeSuffix()
      };
      extended::DateTuple upperBound{
        untilYm.year,
        untilYm.month,
        1,
        0,
        extended::ZoneContext::kSuffixW
      };
      if (upperBound < untilDate) {
        untilDate = upperBound;
      }

      return {startDate, untilDate, era, prevMatch, 0, 0};
    }

    /**
     * Create the Transition objects which are defined by the list of matches
     * and store them in the transitionStorage container. Step 2.
     */
    static void createTransitions(
        TransitionStorage& transitionStorage,
        MatchingEra* matches,
        uint8_t numMatches) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("createTransitions()\n");
      }

      for (uint8_t i = 0; i < numMatches; i++) {
        createTransitionsForMatch(transitionStorage, &matches[i]);
      }
    }

    /** Create the Transitions defined by the given match. Step 2. */
    static void createTransitionsForMatch(
        TransitionStorage& transitionStorage,
        MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("== createTransitionsForMatch()\n");
      }
      const ZPB policy = match->era.zonePolicy();
      if (policy.isNull()) {
        createTransitionsFromSimpleMatch(transitionStorage, match);
      } else {
        createTransitionsFromNamedMatch(transitionStorage, match);
      }
    }

    // Step 2A
    static void createTransitionsFromSimpleMatch(
        TransitionStorage& transitionStorage,
        MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("== createTransitionsFromSimpleMatch()\n");
      }

      Transition* freeTransition = transitionStorage.getFreeAgent();
      createTransitionForYear(freeTransition, 0 /*not used*/,
          ZRB() /*rule*/, match);
      freeTransition->compareStatus = extended::CompareStatus::kExactMatch;
      match->lastOffsetSeconds = freeTransition->offsetSeconds;
      match->lastDeltaSeconds = freeTransition->deltaSeconds;
      transitionStorage.addFreeAgentToActivePool();
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log();
      }
    }

    // Step 2B
    static void createTransitionsFromNamedMatch(
        TransitionStorage& transitionStorage,
        MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("== createTransitionsFromNamedMatch()\n");
      }

      transitionStorage.resetCandidatePool();
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        match->log(); logging::printf("\n");
      }

      // Pass 1: Find candidate transitions using whole years.
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("---- Pass 1: findCandidateTransitions()\n");
      }
      findCandidateTransitions(transitionStorage, match);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log();
      }

      // Pass 2: Fix the transitions times, converting 's' and 'u' into 'w'
      // uniformly.
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("---- Pass 2: fixTransitionTimes()\n");
      }
      fixTransitionTimes(
          transitionStorage.getCandidatePoolBegin(),
          transitionStorage.getCandidatePoolEnd());

      // Pass 3: Select only those Transitions which overlap with the actual
      // start and until times of the MatchingEra.
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("---- Pass 3: selectActiveTransitions()\n");
      }
      selectActiveTransitions(
          transitionStorage.getCandidatePoolBegin(),
          transitionStorage.getCandidatePoolEnd());
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log();
      }
      Transition* lastTransition =
          transitionStorage.addActiveCandidatesToActivePool();
      match->lastOffsetSeconds = lastTransition->offsetSeconds;
      match->lastDeltaSeconds = lastTransition->deltaSeconds;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log();
      }
    }

    // Step 2B: Pass 1
    static void findCandidateTransitions(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      using extended::CompareStatus;

      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findCandidateTransitions(): \n");
        match->log();
        logging::printf("\n");
      }
      const ZPB policy = match->era.zonePolicy();
      uint8_t numRules = policy.numRules();
      int16_t startY = match->startDateTime.year;
      int16_t endY = match->untilDateTime.year;

      // The prior is referenced through a handle (i.e. pointer to pointer)
      // because the actual pointer to the prior could change through the
      // transitionStorage.setFreeAgentAsPriorIfValid() method.
      Transition** prior = transitionStorage.reservePrior();
      (*prior)->isValidPrior = false; // indicates "no prior transition"
      for (uint8_t r = 0; r < numRules; r++) {
        const ZRB rule = policy.rule(r);

        // Add Transitions for interior years
        int16_t interiorYears[kMaxInteriorYears];
        uint8_t numYears = calcInteriorYears(interiorYears, kMaxInteriorYears,
            rule.fromYear(), rule.toYear(), startY, endY);
        for (uint8_t y = 0; y < numYears; y++) {
          int16_t year = interiorYears[y];
          Transition* t = transitionStorage.getFreeAgent();
          createTransitionForYear(t, year, rule, match);
          CompareStatus status = compareTransitionToMatchFuzzy(t, match);
          if (status == CompareStatus::kPrior) {
            transitionStorage.setFreeAgentAsPriorIfValid();
          } else if (status == CompareStatus::kWithinMatch) {
            transitionStorage.addFreeAgentToCandidatePool();
          } else {
            // Must be kFarFuture.
            // Do nothing, allowing the free agent to be reused.
          }
        }

        // Add Transition for prior year
        int16_t priorYear = getMostRecentPriorYear(
            rule.fromYear(), rule.toYear(), startY, endY);
        if (priorYear != LocalDate::kInvalidYear) {
          if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
            logging::printf(
              "findCandidateTransitions(): priorYear: %d\n", priorYear);
          }
          Transition* t = transitionStorage.getFreeAgent();
          createTransitionForYear(t, priorYear, rule, match);
          transitionStorage.setFreeAgentAsPriorIfValid();
        }
      }

      // Add the reserved prior into the Candidate pool only if 'isValidPrior'
      // is true.
      if ((*prior)->isValidPrior) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "findCandidateTransitions(): adding prior to Candidate pool\n");
          logging::printf("  ");
          (*prior)->log();
          logging::printf("\n");
        }
        transitionStorage.addPriorToCandidatePool();
      }
    }

    /**
     * Calculate interior years. Up to maxInteriorYears, usually 3 or 4.
     * Returns the number of interior years.
     *
     * If the MatchingEra's UNTIL year is Jan 1st 00:00, the end year is
     * technically the previous year. However, we treat the UNTIL year as an
     * inclusive endYear just in case there is a transition Rule on Jan 1 00:00.
     *
     * Normally we will use a 14-month matching interval (Dec of prev year,
     * until Feb of the following year), so the maximum number of interior years
     * that this will return should be 3.
     *
     * @param interiorYears a pointer to array of years
     * @param maxInteriorYears size of interiorYears
     * @param fromYear FROM year field of a Rule entry
     * @param toYear TO year field of a Rule entry
     * @param startYear start year of the matching ZoneEra
     * @param endYear until year of the matching ZoneEra
     */
    static uint8_t calcInteriorYears(
        int16_t* interiorYears,
        uint8_t maxInteriorYears,
        int16_t fromYear, int16_t toYear,
        int16_t startYear, int16_t endYear) {
      uint8_t i = 0;
      for (int16_t year = startYear; year <= endYear; year++) {
        if (fromYear <= year && year <= toYear) {
          interiorYears[i] = year;
          i++;
          if (i >= maxInteriorYears) break;
        }
      }
      return i;
    }

    /**
     * Populate Transition 't' using the startTime from 'rule' (if it exists)
     * else from the start time of 'match'. Fills in 'offsetSeconds',
     * 'deltaSeconds', and 'letter'.
     */
    static void createTransitionForYear(
        Transition* t,
        int16_t year,
        const ZRB& rule,
        const MatchingEra* match) {
      t->match = match;
      t->offsetSeconds = match->era.offsetSeconds();
    #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
      t->rule = rule;
    #endif

      if (rule.isNull()) {
        // Create a Transition using the MatchingEra for the transitionTime.
        // Used for simple MatchingEra.
        t->transitionTime = match->startDateTime;
        t->deltaSeconds = match->era.deltaSeconds();
        t->abbrev[0] = '\0';
      } else {
        t->transitionTime = getTransitionTime(year, rule);
        t->deltaSeconds = rule.deltaSeconds();
        ace_common::strncpy_T(
            t->abbrev, rule.letter(), internal::kAbbrevSize - 1);
        t->abbrev[internal::kAbbrevSize - 1] = '\0';
      }
    }

    /**
     * Return the most recent year from the Rule[fromYear, toYear] which is
     * prior to the matching ZoneEra years of [startYear, endYear].
     *
     * Return LocalDate::kInvalidYear if the rule[fromYear, to_year] has no
     * prior year to the MatchingEra[startYear, endYear].
     *
     * @param fromYear FROM year field of a Rule entry
     * @param toYear TO year field of a Rule entry
     * @param startYear start year of the matching ZoneEra
     * @param endYear until year of the matching ZoneEra (unused)
     */
    static int16_t getMostRecentPriorYear(
        int16_t fromYear, int16_t toYear,
        int16_t startYear, int16_t /*endYear*/) {

      if (fromYear < startYear) {
        if (toYear < startYear) {
          return toYear;
        } else {
          return startYear - 1;
        }
      } else {
        return LocalDate::kInvalidYear;
      }
    }

    /**
     * Return the DateTuple representing the transition time of the given rule
     * for the given year.
     */
    static extended::DateTuple getTransitionTime(
        int16_t year, const ZRB& rule) {

      internal::MonthDay monthDay = internal::calcStartDayOfMonth(
          year,
          rule.inMonth(),
          rule.onDayOfWeek(),
          rule.onDayOfMonth());
      return {
        year,
        monthDay.month,
        monthDay.day,
        (int32_t) rule.atTimeSeconds(),
        rule.atTimeSuffix()
      };
    }

    /**
     * Like compareTransitionToMatch() except perform a fuzzy match within at
     * least one-month of the match.start or match.until.
     *
     * Return:
     *    * kPrior if t less than match by at least one month
     *    * kWithinMatch if t within match,
     *    * kFarFuture if t greater than match by at least one month
     *    * kExactMatch is never returned, we cannot know that t == match.start
     */
    static extended::CompareStatus compareTransitionToMatchFuzzy(
        const Transition* t, const MatchingEra* match) {
      return compareDateTupleFuzzy(
          t->transitionTime,
          match->startDateTime,
          match->untilDateTime);
    }

    /**
     * Normalize the transitionTime* fields of the array of Transition objects.
     * Most Transition.transitionTime is given in 'w' mode. However, if it is
     * given in 's' or 'u' mode, we convert these into the 'w' mode for
     * consistency. To convert an 's' or 'u' into 'w', we need the UTC offset
     * of the current Transition, which happens to be given by the *previous*
     * Transition. Step 2B: Pass 2.
     */
    static void fixTransitionTimes(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("fixTransitionTimes(): START; #transitions=%d\n",
          (int) (end - begin));
        Transition::printTransitions("  ", begin, end);
      }

      // extend first Transition to -infinity
      Transition* prev = *begin;

      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* curr = *iter;
        expandDateTuple(
            &curr->transitionTime,
            prev->offsetSeconds,
            prev->deltaSeconds,
            &curr->transitionTime,
            &curr->transitionTimeS,
            &curr->transitionTimeU);
        prev = curr;
      }
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("fixTransitionTimes(): FIXED\n");
        Transition::printTransitions("  ", begin, end);
        logging::printf("fixTransitionTimes(): END\n");
      }
    }

    /**
     * Scan through the Candidate transitions, and mark the ones which are
     * active. Step 2B: Pass 3.
     */
    static void selectActiveTransitions(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("selectActiveTransitions(): #candidates: %d\n",
          (int) (end - begin));
      }

      Transition* prior = nullptr;
      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* transition = *iter;
        processTransitionCompareStatus(transition, &prior);
      }

      // If the latest prior transition is found, shift it to start at the
      // startDateTime of the current match.
      if (prior) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "selectActiveTransitions(): found latest prior\n");
        }
      #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
        prior->originalTransitionTime = prior->transitionTime;
      #endif
        prior->transitionTime = prior->match->startDateTime;
      }
    }

    /**
     * Determine the match status of a transition depending on the temporal
     * relationship to the given MatchingEra. Also determine the latest prior
     * transition before match, marking any previous prior transition as
     * kFarPast.
     */
    static void processTransitionCompareStatus(
        Transition* transition,
        Transition** prior) {
      using extended::CompareStatus;

      CompareStatus status = compareTransitionToMatch(
          transition, transition->match);
      transition->compareStatus = status;

      if (status == CompareStatus::kExactMatch) {
        if (*prior) {
          (*prior)->compareStatus = CompareStatus::kFarPast;
        }
        (*prior) = transition;
      } else if (status == CompareStatus::kPrior) {
        if (*prior) {
          if ((*prior)->transitionTimeU <= transition->transitionTimeU) {
            (*prior)->compareStatus = CompareStatus::kFarPast;
            (*prior) = transition;
          } else {
            transition->compareStatus = CompareStatus::kFarPast;
          }
        } else {
          (*prior) = transition;
        }
      }
    }

    /**
     * Compare the temporal location of transition compared to the interval
     * defined by  the match. The transition time of the Transition is expanded
     * to include all 3 versions ('w', 's', and 'u') of the time stamp. When
     * comparing against the MatchingEra.startDateTime and
     * MatchingEra.untilDateTime, the version will be determined by the suffix
     * of those parameters.
     */
    static extended::CompareStatus compareTransitionToMatch(
        const Transition* transition,
        const MatchingEra* match) {

      // Find the previous Match offsets.
      int32_t prevMatchOffsetSeconds;
      int32_t prevMatchDeltaSeconds;
      if (match->prevMatch) {
        prevMatchOffsetSeconds = match->prevMatch->lastOffsetSeconds;
        prevMatchDeltaSeconds = match->prevMatch->lastDeltaSeconds;
      } else {
        prevMatchOffsetSeconds = match->era.offsetSeconds();
        prevMatchDeltaSeconds = 0;
      }

      // Expand start times.
      extended::DateTuple stw;
      extended::DateTuple sts;
      extended::DateTuple stu;
      expandDateTuple(
          &match->startDateTime,
          prevMatchOffsetSeconds,
          prevMatchDeltaSeconds,
          &stw,
          &sts,
          &stu);

      // Transition times.
      const extended::DateTuple& ttw = transition->transitionTime;
      const extended::DateTuple& tts = transition->transitionTimeS;
      const extended::DateTuple& ttu = transition->transitionTimeU;

      // Compare Transition to Match, where equality is assumed if *any* of the
      // 'w', 's', or 'u' versions of the DateTuple are equal. This prevents
      // duplicate Transition instances from being created in a few cases.
      if (ttw == stw || tts == sts || ttu == stu) {
        return extended::CompareStatus::kExactMatch;
      }

      if (ttu < stu) {
        return extended::CompareStatus::kPrior;
      }

      // Now check if the transition occurs after the given match. The
      // untilDateTime of the current match uses the same UTC offsets as the
      // transitionTime of the current transition, so no complicated adjustments
      // are needed. We just make sure we compare 'w' with 'w', 's' with 's',
      // and 'u' with 'u'.
      const extended::DateTuple& matchUntil = match->untilDateTime;
      const extended::DateTuple* transitionTime;
      if (matchUntil.suffix == extended::ZoneContext::kSuffixS) {
        transitionTime = &tts;
      } else if (matchUntil.suffix == extended::ZoneContext::kSuffixU) {
        transitionTime = &ttu;
      } else { // assume 'w'
        transitionTime = &ttw;
      }
      if (*transitionTime < matchUntil) {
        return extended::CompareStatus::kWithinMatch;
      }
      return extended::CompareStatus::kFarFuture;
    }

    /**
     * Generate startDateTime and untilDateTime of the transitions defined by
     * the [start, end) iterators. The Transition::transitionTime should all be
     * in 'w' mode by the time this method is called.
     */
    static void generateStartUntilTimes(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf(
          "generateStartUntilTimes(): #transitions=%d\n",
          (int) (end - begin));
      }

      // It is possible that there are no matching transitions. This can happen
      // if the zonedbx is corrupted and ZoneInfo contains invalid fields.
      if (begin == end) return;

      Transition* prev = *begin;
      bool isAfterFirst = false;

      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* const t = *iter;

        // 1) Update the untilDateTime of the previous Transition
        const extended::DateTuple& tt = t->transitionTime;
        if (isAfterFirst) {
          prev->untilDateTime = tt;
        }

        // 2) Calculate the current startDateTime by shifting the
        // transitionTime (represented in the UTC offset of the previous
        // transition) into the UTC offset of the *current* transition.
        int32_t seconds = tt.seconds + (
            - prev->offsetSeconds - prev->deltaSeconds
            + t->offsetSeconds + t->deltaSeconds);
        t->startDateTime = {tt.year, tt.month, tt.day, seconds, tt.suffix};
        extended::normalizeDateTuple(&t->startDateTime);

        // 3) The epochSecond of the 'transitionTime' is determined by the
        // UTC offset of the *previous* Transition. However, the
        // transitionTime can be represented by an illegal time (e.g. 24:00).
        // So, it is better to use the properly normalized startDateTime
        // (calculated above) with the *current* UTC offset.
        //
        // NOTE: We should also be able to  calculate this directly from
        // 'transitionTimeU' which should still be a valid field, because it
        // hasn't been clobbered by 'untilDateTime' yet. Not sure if this saves
        // any CPU time though, since we still need to mutiply by 900.
        const extended::DateTuple& st = t->startDateTime;
        const acetime_t offsetSeconds =
            st.seconds - (t->offsetSeconds + t->deltaSeconds);
        LocalDate ld = LocalDate::forComponents(st.year, st.month, st.day);
        t->startEpochSeconds = ld.toEpochSeconds() + offsetSeconds;

        prev = t;
        isAfterFirst = true;
      }

      // The last Transition's until time is the until time of the MatchingEra.
      extended::DateTuple untilTimeW;
      extended::DateTuple untilTimeS;
      extended::DateTuple untilTimeU;
      expandDateTuple(
          &prev->match->untilDateTime,
          prev->offsetSeconds,
          prev->deltaSeconds,
          &untilTimeW,
          &untilTimeS,
          &untilTimeU);
      prev->untilDateTime = untilTimeW;
    }

    /**
     * Calculate the time zone abbreviations for each Transition.
     */
    static void calcAbbreviations(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("calcAbbreviations(): #transitions: %d\n",
          (int) (end - begin));
      }
      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* const t = *iter;
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "calcAbbreviations(): format:%s, deltaSeconds:%d, letter:%s\n",
            t->format(), t->deltaSeconds, t->abbrev);
        }
        internal::createAbbreviation(
            t->abbrev,
            internal::kAbbrevSize,
            t->format(),
            t->deltaSeconds,
            t->abbrev);
      }
    }

  private:
    const ZIS* mZoneInfoStore; // nullable
    ZIB mZoneInfoBroker;

    // NOTE: Maybe move mNumMatches and mMatches into a MatchStorage object.
    mutable uint8_t mNumMatches = 0; // actual number of matches
    mutable MatchingEra mMatches[kMaxMatches];
    mutable TransitionStorage mTransitionStorage;
};

/**
 * A specific implementation of ExtendedZoneProcessorTemplate that uses the
 * extended::ZoneXxxBrokers classes which read from 'zonedbx' files in PROGMEM
 * flash memory.
 */
class ExtendedZoneProcessor: public ExtendedZoneProcessorTemplate<
    extended::ZoneInfoStore,
    extended::ZoneInfoBroker,
    extended::ZoneEraBroker,
    extended::ZonePolicyBroker,
    extended::ZoneRuleBroker> {

  public:
    /** Unique TimeZone type identifier for ExtendedZoneProcessor. */
    static const uint8_t kTypeExtended = 4;

    explicit ExtendedZoneProcessor(const extended::ZoneInfo* zoneInfo = nullptr)
      : ExtendedZoneProcessorTemplate<
          extended::ZoneInfoStore,
          extended::ZoneInfoBroker,
          extended::ZoneEraBroker,
          extended::ZonePolicyBroker,
          extended::ZoneRuleBroker>(
              kTypeExtended, &mZoneInfoStore, (uintptr_t) zoneInfo)
    {}

  private:
    extended::ZoneInfoStore mZoneInfoStore;
};

} // namespace ace_time

#endif
