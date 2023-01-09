/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_H
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_H

#include <stdint.h> // uintptr_t
#include <string.h> // memcpy(), strncpy()
#include <AceCommon.h> // copyReplaceString()
#include "common/compat.h"
#include "internal/common.h" // kAbbrevSize
#include "internal/ZonePolicy.h"
#include "internal/ZoneInfo.h"
#include "internal/ExtendedBrokers.h"
#include "common/logging.h"
#include "TimeOffset.h"
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
class ExtendedZoneProcessorTest_normalizeDateTuple;
class ExtendedZoneProcessorTest_expandDateTuple;
class ExtendedZoneProcessorTest_calcInteriorYears;
class ExtendedZoneProcessorTest_getMostRecentPriorYear;
class ExtendedZoneProcessorTest_compareDateTupleFuzzy;
class ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
class ExtendedZoneProcessorTest_compareTransitionToMatch;
class ExtendedZoneProcessorTest_processTransitionMatchStatus;
class ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
class ExtendedZoneProcessorTest_createAbbreviation;
class ExtendedZoneProcessorTest_setZoneKey;
class TransitionValidation;

class Print;

namespace ace_time {

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
 * @tparam BF type of BrokerFactory, needed for implementations that require
 *    more complex brokers, and allows this template class to be independent
 *    of the exact type of the zone primary key
 * @tparam ZIB type of ZoneInfoBroker
 * @tparam ZEB type of ZoneEraBroker
 * @tparam ZPB type of ZonePolicyBroker
 * @tparam ZRB type of ZoneRuleBroker
 */
template <typename BF, typename ZIB, typename ZEB, typename ZPB, typename ZRB>
class ExtendedZoneProcessorTemplate: public ZoneProcessor {
  public:
    /**
     * Max number of Transitions required for all Zones supported by this class.
     * This includes the most recent prior Transition. The max transitions for
     * each Zone is given by the kZoneBufSize{zoneName} constant in the
     * generated `zonedb[x]/zone_infos.h` file. The maximum over all zones is
     * given in the 'MaxBufSize' comment in the `zone_infos.h` file. Currently
     * that overall maximum is 7, which has been verified by the
     * ExtendedDateUtilTest, ExtendedJavaTest, and ExtendedAcetzTest validation
     * tests. We set this to one more than 7 for safety.
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

    bool isLink() const override { return mZoneInfoBroker.isLink(); }

    uint32_t getZoneId(bool followLink = false) const override {
      ZIB zib = (isLink() && followLink)
          ? mZoneInfoBroker.targetZoneInfo()
          : mZoneInfoBroker;
      return zib.zoneId();
    }

    FindResult findByLocalDateTime(
        const LocalDateTime& ldt) const override {
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
        result.type = FindResult::Type::kExact;
      } else {
        if (transitionForDateTime.prev == nullptr
            || transitionForDateTime.curr == nullptr) {
          // ldt was far past or far future
          transition = nullptr;
          result.type = FindResult::Type::kNotFound;
        } else {
          if (transitionForDateTime.num == 0) { // Gap
            result.type = FindResult::Type::kGap;
            if (ldt.fold() == 0) {
              // ldt wants to use the 'prev' transition to convert to
              // epochSeconds.
              result.origOffsetMinutes =
                  transitionForDateTime.prev->offsetMinutes
                  + transitionForDateTime.prev->deltaMinutes;
              // But after normalization, it will be shifted into the curr
              // transition, so select 'curr' as the target transition.
              transition = transitionForDateTime.curr;
            } else {
              // ldt wants to use the 'curr' transition to convert to
              // epochSeconds.
              result.origOffsetMinutes =
                  transitionForDateTime.curr->offsetMinutes
                  + transitionForDateTime.curr->deltaMinutes;
              // But after normalization, it will be shifted into the prev
              // transition, so select 'prev' as the target transition.
              transition = transitionForDateTime.prev;
            }
          } else { // num==2, Overlap
            transition = (ldt.fold() == 0) ? transitionForDateTime.prev :
            transitionForDateTime.curr;
            result.type = FindResult::Type::kOverlap;
          }
        }
      }

      if (! transition) {
        return result;
      }

      result.stdOffsetMinutes = transition->offsetMinutes;
      result.dstOffsetMinutes = transition->deltaMinutes;
      if (result.type != FindResult::Type::kGap) {
        // If not in the gap, calc the origOffsetMinutes for consistency.
        result.origOffsetMinutes = result.stdOffsetMinutes
            + result.dstOffsetMinutes;
      }
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
      const Transition* transition = transitionForSeconds.transition;
      if (!transition) return result;

      result.stdOffsetMinutes = transition->offsetMinutes;
      result.dstOffsetMinutes = transition->deltaMinutes;
      result.origOffsetMinutes = transition->offsetMinutes
          + transition->deltaMinutes;
      result.abbrev = transition->abbrev;
      result.type = (transitionForSeconds.fold == 1)
          ? FindResult::Type::kOverlap
          : FindResult::Type::kExact;
      return result;
    }

    void printNameTo(Print& printer, bool followLink = false) const override {
      ZIB zib = (isLink() && followLink)
          ? mZoneInfoBroker.targetZoneInfo()
          : mZoneInfoBroker;
      zib.printNameTo(printer);
    }

    void printShortNameTo(Print& printer, bool followLink = false)
        const override {
      ZIB zib = (isLink() && followLink)
          ? mZoneInfoBroker.targetZoneInfo()
          : mZoneInfoBroker;
      zib.printShortNameTo(printer);
    }

    /** Used only for debugging. */
    void log() const {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("ExtendedZoneProcessor:\n");
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
      if (! mBrokerFactory) return;
      if (mZoneInfoBroker.equals(zoneKey)) return;

      mZoneInfoBroker = mBrokerFactory->createZoneInfoBroker(zoneKey);
      mYear = LocalDate::kInvalidYear;
      mIsFilled = false;
      mNumMatches = 0;
      resetTransitionAllocSize(); // clear the alloc size for new zone
    }

    bool equalsZoneKey(uintptr_t zoneKey) const override {
      return mZoneInfoBroker.equals(zoneKey);
    }

    /**
     * Set the broker factory at runtime. This is an advanced usage where the
     * custom subclass of ExtendedZoneProcessorTemplate does not know its broker
     * factory at compile time, so it must be set at runtime through this
     * method.
     */
    void setBrokerFactory(const BF* brokerFactory) {
      mBrokerFactory = brokerFactory;
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
      if (isFilled(year)) return true;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("initForYear(): %d\n", year);
      }

      mYear = year;
      mNumMatches = 0; // clear cache
      mTransitionStorage.init();

      if (year < mZoneInfoBroker.zoneContext()->startYear - 1
          || mZoneInfoBroker.zoneContext()->untilYear < year) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
              "initForYear(): Year %d out of valid range [%d, %d)\n",
              year,
              mZoneInfoBroker.zoneContext()->startYear,
              mZoneInfoBroker.zoneContext()->untilYear);
        }
        return false;
      }

      extended::YearMonthTuple startYm = { (int16_t) (year - 1), 12 };
      extended::YearMonthTuple untilYm =  { (int16_t) (year + 1), 2 };

      // Step 1. The equivalent steps for the Python version are in the
      // AceTimePython project, under
      // zone_processor.ZoneProcessor.init_for_year().
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

      mIsFilled = true;
      return true;
    }

  protected:
    /**
     * Constructor. When first initialized inside a cache, the brokerFactory may
     * be set to nullptr, and the zoneKey should be ignored.
     *
     * @param type indentifier for the specific subclass of ZoneProcessor (e.g.
     *    Basic versus Extended) mostly used for debugging
     * @param brokerFactory pointer to a BrokerFactory that creates a ZIB
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    explicit ExtendedZoneProcessorTemplate(
        uint8_t type,
        const BF* brokerFactory /*nullable*/,
        uintptr_t zoneKey
    ) :
        ZoneProcessor(type),
        mBrokerFactory(brokerFactory)
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
    friend class ::ExtendedZoneProcessorTest_normalizeDateTuple;
    friend class ::ExtendedZoneProcessorTest_expandDateTuple;
    friend class ::ExtendedZoneProcessorTest_calcInteriorYears;
    friend class ::ExtendedZoneProcessorTest_getMostRecentPriorYear;
    friend class ::ExtendedZoneProcessorTest_compareDateTupleFuzzy;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatch;
    friend class ::ExtendedZoneProcessorTest_processTransitionMatchStatus;
    friend class ::ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
    friend class ::ExtendedZoneProcessorTest_createAbbreviation;
    friend class ::ExtendedZoneProcessorTest_setZoneKey;
    friend class ::TransitionValidation;

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
     * Determines if era overlaps the interval [startYm, untilYm). This does
     * not need to be exact since the startYm and untilYm are created to have
     * some slop of about one month at the low and high end, so we can ignore
     * the day, time and timeSuffix fields of the era. The start date of the
     * current era is represented by the UNTIL fields of the previous era, so
     * the interval of the current era is [era.start=prev.UNTIL,
     * era.until=era.UNTIL). Overlap happens if (era.start < untilYm) and
     * (era.until > startYm). If prev.isNull(), then interpret prev as the
     * earliest ZoneEra.
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
      //if (era.untilTimeMinutes() < 0) return -1; // never possible
      if (era.untilTimeMinutes() > 0) return 1;
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
              internal::ZoneContext::kSuffixW
            }
          : extended::DateTuple{
              prevMatch->era.untilYear(),
              prevMatch->era.untilMonth(),
              prevMatch->era.untilDay(),
              (int16_t) prevMatch->era.untilTimeMinutes(),
              prevMatch->era.untilTimeSuffix()
            };
      extended::DateTuple lowerBound{
        startYm.year,
        startYm.month,
        1,
        0,
        internal::ZoneContext::kSuffixW
      };
      if (startDate < lowerBound) {
        startDate = lowerBound;
      }

      extended::DateTuple untilDate{
        era.untilYear(),
        era.untilMonth(),
        era.untilDay(),
        (int16_t) era.untilTimeMinutes(),
        era.untilTimeSuffix()
      };
      extended::DateTuple upperBound{
        untilYm.year,
        untilYm.month,
        1,
        0,
        internal::ZoneContext::kSuffixW
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
      freeTransition->matchStatus = extended::MatchStatus::kExactMatch;
      match->lastOffsetMinutes = freeTransition->offsetMinutes;
      match->lastDeltaMinutes = freeTransition->deltaMinutes;
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
      match->lastOffsetMinutes = lastTransition->offsetMinutes;
      match->lastDeltaMinutes = lastTransition->deltaMinutes;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log();
      }
    }

    // Step 2B: Pass 1
    static void findCandidateTransitions(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      using extended::MatchStatus;

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
          MatchStatus status = compareTransitionToMatchFuzzy(t, match);
          if (status == MatchStatus::kPrior) {
            transitionStorage.setFreeAgentAsPriorIfValid();
          } else if (status == MatchStatus::kWithinMatch) {
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
     * else from the start time of 'match'. Fills in 'offsetMinutes' and
     * 'deltaMinutes' as well. 'letterBuf' is also well-defined, either an empty
     * string, or filled with rule->letter with a NUL terminator.
     */
    static void createTransitionForYear(
        Transition* t,
        int16_t year,
        const ZRB& rule,
        const MatchingEra* match) {
      t->match = match;
      t->rule = rule;
      t->offsetMinutes = match->era.offsetMinutes();
      t->letterBuf[0] = '\0';

      if (! rule.isNull()) {
        t->transitionTime = getTransitionTime(year, rule);
        t->deltaMinutes = rule.deltaMinutes();

        char letter = rule.letter();
        if (letter >= 32) {
          // If LETTER is a '-', treat it the same as an empty string.
          if (letter != '-') {
            t->letterBuf[0] = letter;
            t->letterBuf[1] = '\0';
          }
        } else {
          // rule->letter is a long string, so is referenced as an offset index
          // into the ZonePolicy.letters array. The string cannot fit in
          // letterBuf, so will be retrieved by the letter() method below.
        }
      } else {
        // Create a Transition using the MatchingEra for the transitionTime.
        // Used for simple MatchingEra.
        t->transitionTime = match->startDateTime;
        t->deltaMinutes = match->era.deltaMinutes();
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
        (int16_t) rule.atTimeMinutes(),
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
    static extended::MatchStatus compareTransitionToMatchFuzzy(
        const Transition* t, const MatchingEra* match) {
      return compareDateTupleFuzzy(
          t->transitionTime,
          match->startDateTime,
          match->untilDateTime);
    }

    /**
     * Determine the relationship of t to the time interval defined by `[start,
     * until)`. The comparison is fuzzy, with a slop of about one month so that
     * we can ignore the day and minutes fields.
     *
     * The following values are returned:
     *
     *  * kPrior if 't' is less than 'start' by at least one month,
     *  * kFarFuture if 't' is greater than 'until' by at least one month,
     *  * kWithinMatch if 't' is within [start, until) with a one month slop,
     *  * kExactMatch is never returned.
     */
    static extended::MatchStatus compareDateTupleFuzzy(
        const extended::DateTuple& t,
        const extended::DateTuple& start,
        const extended::DateTuple& until) {
      using extended::MatchStatus;
      // Use int32_t because a delta year of 2730 or greater will exceed
      // the range of an int16_t.
      int32_t tMonths = t.year * (int32_t) 12 + t.month;
      int32_t startMonths = start.year * (int32_t) 12 + start.month;
      if (tMonths < startMonths - 1) return MatchStatus::kPrior;
      int32_t untilMonths = until.year * 12 + until.month;
      if (untilMonths + 1 < tMonths) return MatchStatus::kFarFuture;
      return MatchStatus::kWithinMatch;
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
            prev->offsetMinutes,
            prev->deltaMinutes,
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
     * Convert the given 'tt', offsetMinutes, and deltaMinutes into the 'w', 's'
     * and 'u' versions of the DateTuple. It is allowed for 'ttw' to be an alias
     * of 'tt'.
     */
    static void expandDateTuple(
        const extended::DateTuple* tt,
        int16_t offsetMinutes,
        int16_t deltaMinutes,
        extended::DateTuple* ttw,
        extended::DateTuple* tts,
        extended::DateTuple* ttu) {

      if (tt->suffix == internal::ZoneContext::kSuffixS) {
        *tts = *tt;
        *ttu = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes - offsetMinutes),
            internal::ZoneContext::kSuffixU};
        *ttw = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes + deltaMinutes),
            internal::ZoneContext::kSuffixW};
      } else if (tt->suffix == internal::ZoneContext::kSuffixU) {
        *ttu = *tt;
        *tts = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes + offsetMinutes),
            internal::ZoneContext::kSuffixS};
        *ttw = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes + (offsetMinutes + deltaMinutes)),
            internal::ZoneContext::kSuffixW};
      } else {
        // Explicit set the suffix to 'w' in case it was something else.
        *ttw = *tt;
        ttw->suffix = internal::ZoneContext::kSuffixW;
        *tts = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes - deltaMinutes),
            internal::ZoneContext::kSuffixS};
        *ttu = {tt->year, tt->month, tt->day,
            (int16_t) (tt->minutes - (deltaMinutes + offsetMinutes)),
            internal::ZoneContext::kSuffixU};
      }

      extended::normalizeDateTuple(ttw);
      extended::normalizeDateTuple(tts);
      extended::normalizeDateTuple(ttu);
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
        processTransitionMatchStatus(transition, &prior);
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
    static void processTransitionMatchStatus(
        Transition* transition,
        Transition** prior) {
      using extended::MatchStatus;

      MatchStatus status = compareTransitionToMatch(
          transition, transition->match);
      transition->matchStatus = status;

      if (status == MatchStatus::kExactMatch) {
        if (*prior) {
          (*prior)->matchStatus = MatchStatus::kFarPast;
        }
        (*prior) = transition;
      } else if (status == MatchStatus::kPrior) {
        if (*prior) {
          if ((*prior)->transitionTimeU <= transition->transitionTimeU) {
            (*prior)->matchStatus = MatchStatus::kFarPast;
            (*prior) = transition;
          } else {
            transition->matchStatus = MatchStatus::kFarPast;
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
    static extended::MatchStatus compareTransitionToMatch(
        const Transition* transition,
        const MatchingEra* match) {

      // Find the previous Match offsets.
      int16_t prevMatchOffsetMinutes;
      int16_t prevMatchDeltaMinutes;
      if (match->prevMatch) {
        prevMatchOffsetMinutes = match->prevMatch->lastOffsetMinutes;
        prevMatchDeltaMinutes = match->prevMatch->lastDeltaMinutes;
      } else {
        prevMatchOffsetMinutes = match->era.offsetMinutes();
        prevMatchDeltaMinutes = 0;
      }

      // Expand start times.
      extended::DateTuple stw;
      extended::DateTuple sts;
      extended::DateTuple stu;
      expandDateTuple(
          &match->startDateTime,
          prevMatchOffsetMinutes,
          prevMatchDeltaMinutes,
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
        return extended::MatchStatus::kExactMatch;
      }

      if (ttu < stu) {
        return extended::MatchStatus::kPrior;
      }

      // Now check if the transition occurs after the given match. The
      // untilDateTime of the current match uses the same UTC offsets as the
      // transitionTime of the current transition, so no complicated adjustments
      // are needed. We just make sure we compare 'w' with 'w', 's' with 's',
      // and 'u' with 'u'.
      const extended::DateTuple& matchUntil = match->untilDateTime;
      const extended::DateTuple* transitionTime;
      if (matchUntil.suffix == internal::ZoneContext::kSuffixS) {
        transitionTime = &tts;
      } else if (matchUntil.suffix == internal::ZoneContext::kSuffixU) {
        transitionTime = &ttu;
      } else { // assume 'w'
        transitionTime = &ttw;
      }
      if (*transitionTime < matchUntil) {
        return extended::MatchStatus::kWithinMatch;
      }
      return extended::MatchStatus::kFarFuture;
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
        int16_t minutes = tt.minutes + (
            - prev->offsetMinutes - prev->deltaMinutes
            + t->offsetMinutes + t->deltaMinutes);
        t->startDateTime = {tt.year, tt.month, tt.day, minutes, tt.suffix};
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
        const acetime_t offsetSeconds = (acetime_t) 60
            * (st.minutes - (t->offsetMinutes + t->deltaMinutes));
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
          prev->offsetMinutes,
          prev->deltaMinutes,
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
            "calcAbbreviations(): format:%s, deltaMinutes:%d, letter:%s\n",
            t->format(), t->deltaMinutes, t->letter());
        }
        createAbbreviation(
            t->abbrev,
            internal::kAbbrevSize,
            t->format(),
            t->deltaMinutes,
            t->letter());
      }
    }

    /**
     * Functionally the same as BasicZoneProcessor::createAbbreviation() execpt
     * that 'letter' is a string.
     *
     * @param letterString nullptr if RULES is a '- or an 'hh:mm', an empty
     * string if the LETTER was a '-', or a pointer to a non-empty string if
     * LETTER was a 'S', 'D', 'WAT' etc.
     */
    static void createAbbreviation(
        char* dest,
        uint8_t destSize,
        const char* format,
        uint16_t deltaMinutes,
        const char* letterString) {

      // Check if FORMAT contains a '%'.
      if (strchr(format, '%') != nullptr) {
        // Check if RULES column empty, therefore no 'letter'
        if (letterString == nullptr) {
          strncpy(dest, format, destSize - 1);
          dest[destSize - 1] = '\0';
        } else {
          ace_common::copyReplaceString(
              dest, destSize, format, '%', letterString);
        }
      } else {
        // Check if FORMAT contains a '/'.
        const char* slashPos = strchr(format, '/');
        if (slashPos != nullptr) {
          if (deltaMinutes == 0) {
            uint8_t headLength = (slashPos - format);
            if (headLength >= destSize) headLength = destSize - 1;
            memcpy(dest, format, headLength);
            dest[headLength] = '\0';
          } else {
            uint8_t tailLength = strlen(slashPos+1);
            if (tailLength >= destSize) tailLength = destSize - 1;
            memcpy(dest, slashPos+1, tailLength);
            dest[tailLength] = '\0';
          }
        } else {
          // Just copy the FORMAT disregarding deltaMinutes and letterString.
          strncpy(dest, format, destSize);
          dest[destSize - 1] = '\0';
        }
      }
    }

  private:
    const BF* mBrokerFactory; // nullable
    ZIB mZoneInfoBroker;

    // NOTE: Maybe move mNumMatches and mMatches into a MatchStorage object.
    mutable uint8_t mNumMatches = 0; // actual number of matches
    mutable MatchingEra mMatches[kMaxMatches];
    mutable TransitionStorage mTransitionStorage;
};


/**
 * A specific implementation of ExtendedZoneProcessorTemplate that uses
 * ZoneXxxBrokers which read from zonedb files in PROGMEM flash memory.
 */
class ExtendedZoneProcessor: public ExtendedZoneProcessorTemplate<
    extended::BrokerFactory,
    extended::ZoneInfoBroker,
    extended::ZoneEraBroker,
    extended::ZonePolicyBroker,
    extended::ZoneRuleBroker> {

  public:
    /** Unique TimeZone type identifier for ExtendedZoneProcessor. */
    static const uint8_t kTypeExtended = 4;

    explicit ExtendedZoneProcessor(const extended::ZoneInfo* zoneInfo = nullptr)
      : ExtendedZoneProcessorTemplate<
          extended::BrokerFactory,
          extended::ZoneInfoBroker,
          extended::ZoneEraBroker,
          extended::ZonePolicyBroker,
          extended::ZoneRuleBroker>(
              kTypeExtended, &mBrokerFactory, (uintptr_t) zoneInfo)
    {}

  private:
    extended::BrokerFactory mBrokerFactory;
};

} // namespace ace_time

#endif
