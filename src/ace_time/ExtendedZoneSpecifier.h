#ifndef ACE_TIME_EXTENDED_ZONE_SPECIFIER_H
#define ACE_TIME_EXTENDED_ZONE_SPECIFIER_H

#include <Arduino.h>
#include <string.h> // memset()
#include <stdint.h>
#include "common/ZonePolicy.h"
#include "common/ZoneInfo.h"
#include "common/logger.h"
#include "UtcOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneSpecifier.h"
#include "BasicZoneSpecifier.h"
#include "local_date_mutation.h"

class ExtendedZoneSpecifierTest_compareEraToYearMonth;
class ExtendedZoneSpecifierTest_createMatch;
class ExtendedZoneSpecifierTest_findMatches_simple;
class ExtendedZoneSpecifierTest_findMatches_named;
class ExtendedZoneSpecifierTest_findTransitionsFromNamedMatch;
class ExtendedZoneSpecifierTest_normalizeDateTuple;
class ExtendedZoneSpecifierTest_expandDateTuple;
class ExtendedZoneSpecifierTest_calcInteriorYears;
class ExtendedZoneSpecifierTest_getMostRecentPriorYear;
class ExtendedZoneSpecifierTest_compareTransitionToMatchFuzzy;
class ExtendedZoneSpecifierTest_compareTransitionToMatch;
class ExtendedZoneSpecifierTest_processActiveTransition;
class ExtendedZoneSpecifierTest_fixTransitionTimes_generateStartUntilTimes;
class TransitionStorageTest_getFreeAgent;
class TransitionStorageTest_getFreeAgent2;
class TransitionStorageTest_addFreeAgentToActivePool;
class TransitionStorageTest_reservePrior;
class TransitionStorageTest_addFreeAgentToCandidatePool;
class TransitionStorageTest_setFreeAgentAsPrior;
class TransitionStorageTest_addActiveCandidatesToActivePool;
class TransitionStorageTest_resetCandidatePool;

namespace ace_time {

namespace zonedbx {

// TODO: Consider compressing 'modifier' into 'month' field
struct DateTuple {
  int8_t yearTiny; // [-127, 126], 127 will cause bugs
  uint8_t month; // [1-12]
  uint8_t day; // [1-31]
  int8_t timeCode; // 15-min intervals, negative values allowed
  uint8_t modifier; // 's', 'w', 'u'
};

inline bool operator<(const DateTuple& a, const DateTuple& b) {
  if (a.yearTiny < b.yearTiny) return true;
  if (a.yearTiny > b.yearTiny) return false;
  if (a.month < b.month) return true;
  if (a.month > b.month) return false;
  if (a.day < b.day) return true;
  if (a.day > b.day) return false;
  if (a.timeCode < b.timeCode) return true;
  if (a.timeCode > b.timeCode) return false;
  return false;
}

inline bool operator==(const DateTuple& a, const DateTuple& b) {
  return a.yearTiny == b.yearTiny
      && a.month == b.month
      && a.day == b.day
      && a.timeCode == b.timeCode
      && a.modifier == b.modifier;
}

struct YearMonthTuple {
  int8_t yearTiny;
  uint8_t month;
};

/**
 * Data structure that captures the matching ZoneEra and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 */
struct ZoneMatch {
  /** The effective start time of the matching ZoneEra. */
  DateTuple startDateTime;

  /** The effective until time of the matching ZoneEra. */
  DateTuple untilDateTime;

  /** The ZoneEra that matched the given year. NonNullable. */
  const zonedbx::ZoneEra* era;
};

/**
 * Represents an interval of time where the time zone obeyed a certain UTC
 * offset and DST delta. The start of the interval is given by 'transitionTime'
 * which comes from the TZ Database file. The actual start and until time of
 * the interval (in the local time zone) is given by 'startDateTime' and
 * 'untilDateTime'.
 *
 * There are 2 types of Transition instances:
 *  1) Simple, indicated by 'rule' == nullptr. The base UTC offsetCode is given
 *  by ZoneMatch::offsetCode. The additional DST delta is given by
 *  match->deltaCode.
 *  2) Named, indicated by 'rule' != nullptr. The base UTC offsetCode is given
 *  by ZoneMatch::offsetCode. The additional DST delta is given by
 *  rule->deltaCode.
 */
struct Transition {
  /**
   * Longest abbreviation seems to be 5 characters.
   * https://www.timeanddate.com/time/zones/
   */
  static const uint8_t kAbbrevSize = 5 + 1;

  /** The match which generated this Transition. */
  const ZoneMatch* match;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-', indicating that the ZoneMatch was
   * a "simple" ZoneEra.
   */
  const zonedbx::ZoneRule* rule;

  /**
   * The original transition time, usually 'w' but sometimes 's' or 'u'. After
   * expandDateTuple() is called, this field will definitely be a 'w'. We must
   * remember that the transitionTime* fields are expressed using the UTC
   * offset of the *previous* Transition.
   */
  DateTuple transitionTime;

  union {
    /**
     * Version of transitionTime in 's' mode, using the UTC offset of the
     * *previous* Transition.
     */
    DateTuple transitionTimeS;

    /** Start time expressed using the UTC offset of the current Transition. */
    DateTuple startDateTime;
  };

  union {
    /**
     * Version of transitionTime in 'u' mode, using the UTC offset of the
     * *previous* transition.
     */
    DateTuple transitionTimeU;

    /** Until time expressed using the UTC offset of the current Transition. */
    DateTuple untilDateTime;
  };

  /**
   * If the transition is shifted to the beginning of a ZoneMatch, this is set
   * to the transitionTime for debugging. May be removed in the future.
   */
  DateTuple originalTransitionTime;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];


  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /** Determines if this transition is valid. */
  bool active;

  //-------------------------------------------------------------------------

  const char* format() const {
    return match->era->format;
  }

  int8_t offsetCode() const {
    return match->era->offsetCode;
  }

  char letter() const {
    if (!rule) return '\0';

    // TODO: Handle this condition properly using
    // match->era->zonePolicy->letters[rule->letter]
    if (rule->letter < 32) return '\0';

    return rule->letter;
  }

  int8_t deltaCode() const {
    return (rule) ? rule->deltaCode : match->era->deltaCode;
  }

  /** Used only for debugging. */
  void log() const {
    // FIXME: Check sizeof(acetime_t) == sizeof(int)
    common::logger("startEpochSeconds: %ld", startEpochSeconds);
    common::logger("offsetCode: %d", offsetCode());
    common::logger("abbrev: %s", abbrev);
    if (rule != nullptr) {
      common::logger("Rule.fromYear: %d", rule->fromYearTiny);
      common::logger("Rule.toYear: %d", rule->toYearTiny);
      common::logger("Rule.inMonth: %d", rule->inMonth);
      common::logger("Rule.onDayOfMonth: %d", rule->onDayOfMonth);
    }
  }
};

/**
 * Manages a collection of Transitions, keeping track of unused, used,
 * and active states, using a fixed array of Transitions. This class
 * was create to avoid dynamic allocation of memory. We create a fixed sized
 * array, then manage the pool through this class. There are 3 regions in
 * the mPool space:
 * 1) Active pool: starts at index 0
 * 2) Prior pool (0, 1): at mIndexPrior
 * 3) Candidate pool: at mIndexCandidates
 * 4) Free pool: at mIndexFree until index SIZE
 */
template<uint8_t SIZE>
class TransitionStorage {
  public:
    /** Constructor. */
    TransitionStorage() {}

    /** Initialize all pools. */
    void init() {
      for (uint8_t i = 0; i < SIZE; i++) {
        mTransitions[i] = &mPool[i];
      }
      mIndexPrior = 0;
      mIndexCandidates = 0;
      mIndexFree = 0;
    }

    /** Return the current prior transition. */
    Transition* getPrior() { return mTransitions[mIndexPrior]; }

    /**
     * Empty the Candidate pool by resetting the various indexes.
     *
     * If every iteration of findTransitionsForMatch() finishes with
     * addFreeAgentToActivePool() or addActiveCandidatesToActivePool(), it may
     * be possible to remove this. But it's safer to reset the indexes upon
     * each iteration.
     */
    void resetCandidatePool() {
      mIndexCandidates = mIndexPrior;
      mIndexFree = mIndexPrior;
    }

    Transition** getCandidatePoolBegin() {
      return &mTransitions[mIndexCandidates];
    }
    Transition** getCandidatePoolEnd() {
      return &mTransitions[mIndexFree];
    }

    Transition** getActivePoolBegin() { return &mTransitions[0]; }
    Transition** getActivePoolEnd() { return &mTransitions[mIndexFree]; }

    /**
     * Return a pointer to the first Transition in the free pool. If this
     * transition is not used, then it's ok to just drop it. The next time
     * getFreeAgent() is called, the same Transition will be returned.
     */
    Transition* getFreeAgent() {
      if (mIndexFree < SIZE) {
        return mTransitions[mIndexFree];
      } else {
        return mTransitions[SIZE - 1];
      }
    }

    /**
     * Immediately add the free agent Transition at index mIndexFree to the
     * Active pool. Then increment mIndexFree to remove the free agent
     * from the Free pool. This assumes that the Pending and Candidate pool are
     * empty, which makes the Active pool come immediately before the Free
     * pool.
     */
    void addFreeAgentToActivePool() {
      if (mIndexFree >= SIZE) return;
      mIndexPrior++;
      mIndexCandidates++;
      mIndexFree++;
    }

    /**
     * Allocate one Transition just after the Active pool, but before the
     * Candidate pool, to keep the most recent prior Transition.
     */
    Transition* reservePrior() {
      Transition* prior = mTransitions[mIndexPrior];
      mIndexCandidates++;
      mIndexFree++;
      return prior;
    }

    /** Swap the Free agrent transition with the current Prior transition. */
    void setFreeAgentAsPrior() {
      Transition* prevPrior = mTransitions[mIndexPrior];
      mTransitions[mIndexPrior] = mTransitions[mIndexFree];
      mTransitions[mIndexFree] = prevPrior;
    }

    /**
     * Add the current prior into the Candidates pool. Prior is always just
     * before the start of the Candidate pool, so we just need to shift back
     * the start index of the Candidate pool.
     */
    void addPriorToCandidatePool() {
      mIndexCandidates--;
    }

    /**
     * Add the free agent Transition at index mIndexFree to the Candidate pool,
     * sorted by transitionTime. Then increment mIndexFree by one to remove the
     * free agent from the Free pool.
     */
    void addFreeAgentToCandidatePool() {
      if (mIndexFree >= SIZE) return;
      for (uint8_t i = mIndexFree; i > mIndexCandidates; i--) {
        Transition* curr = mTransitions[i];
        Transition* prev = mTransitions[i - 1];
        if (curr->transitionTime < prev->transitionTime) {
          mTransitions[i] = prev;
          mTransitions[i - 1] = curr;
        } else {
          break;
        }
      }
      mIndexFree++;
    }

    /**
     * Add active candidates into the Active pool, and collapse the Candidate
     * pool.
     */
    void addActiveCandidatesToActivePool() {
      uint8_t iActive = mIndexCandidates;
      uint8_t iCandidate = mIndexCandidates;
      for (; iCandidate < mIndexFree; iCandidate++) {
        if (mTransitions[iCandidate]->active) {
          mTransitions[iActive] = mTransitions[iCandidate];
          ++iActive;
        }
      }
      mIndexPrior = iActive;
      mIndexCandidates = iActive;
      mIndexFree = iActive;
    }

    /**
     * Return the Transition matching the given epochSeconds. Return nullptr if
     * no matching Transition found.
     */
    const Transition* findTransition(acetime_t epochSeconds) {
      const Transition* match = nullptr;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        const Transition* candidate = mTransitions[i];
        if (candidate->startEpochSeconds <= epochSeconds) {
          match = candidate;
        } else if (candidate->startEpochSeconds > epochSeconds) {
          break;
        }
      }
      return match;
    }

  private:
    friend class ::TransitionStorageTest_getFreeAgent;
    friend class ::TransitionStorageTest_getFreeAgent2;
    friend class ::TransitionStorageTest_addFreeAgentToActivePool;
    friend class ::TransitionStorageTest_reservePrior;
    friend class ::TransitionStorageTest_addFreeAgentToCandidatePool;
    friend class ::TransitionStorageTest_setFreeAgentAsPrior;
    friend class ::TransitionStorageTest_addActiveCandidatesToActivePool;
    friend class ::TransitionStorageTest_resetCandidatePool;

    /** Return the transition at position i. */
    Transition* getTransition(uint8_t i) { return mTransitions[i]; }

    Transition mPool[SIZE];
    Transition* mTransitions[SIZE];
    uint8_t mIndexPrior;
    uint8_t mIndexCandidates;
    uint8_t mIndexFree;
};

} // namespace extended

/**
 * Version of BasicZoneSpecifier that works for more obscure zones
 * which obscure rules.
 *
 *  - Zone untilTimeModifier works for 's' or 'u' in addition to 'w'
 *  - Zone UNTIL field supports addtional month, day, or time
 *  - RULES column supports an offset (hh:mm)
 *
 * Not thread-safe.
 */
class ExtendedZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Constructor.
     * @param zoneInfo pointer to a ZoneInfo. Can be nullptr which is
     * interpreted as UTC.
     */
    explicit ExtendedZoneSpecifier(const zonedbx::ZoneInfo* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    /** Copy constructor. */
    explicit ExtendedZoneSpecifier(const ExtendedZoneSpecifier& that):
      mZoneInfo(that.mZoneInfo),
      mIsFilled(false) {}

    /** Return the underlying ZoneInfo. */
    const zonedbx::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    uint8_t getType() const override { return kTypeExtended; }

    UtcOffset getUtcOffset(acetime_t epochSeconds) override {
      if (mZoneInfo == nullptr) return UtcOffset();
      init(epochSeconds);
      const zonedbx::Transition* transition = findTransition(epochSeconds);
      return UtcOffset::forOffsetCode(transition->offsetCode());
    }

    /** Return the DST delta offset at epochSeconds. */
    UtcOffset getDeltaOffset(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      init(epochSeconds);
      const zonedbx::Transition* transition = findTransition(epochSeconds);
      if (transition->rule == nullptr) return UtcOffset();
      return UtcOffset::forOffsetCode(transition->rule->deltaCode);
    }

    const char* getAbbrev(acetime_t epochSeconds) override {
      if (mZoneInfo == nullptr) return "UTC";
      init(epochSeconds);
      const zonedbx::Transition* transition = findTransition(epochSeconds);
      return transition->abbrev;
    }

    void printTo(Print& printer) const override;

    /** Used only for debugging. */
    void log() const {
      common::logger("mYear: %d", mYear);
      common::logger("mNumMatches: %d", mNumMatches);
      /*
      for (int i = 0; i < mNumMatches; i++) {
        common::logger("---- Match: %d", i);
        mMatches[i].log();
      }
      */
    }

  private:
    friend class ::ExtendedZoneSpecifierTest_compareEraToYearMonth;
    friend class ::ExtendedZoneSpecifierTest_createMatch;
    friend class ::ExtendedZoneSpecifierTest_findMatches_simple;
    friend class ::ExtendedZoneSpecifierTest_findMatches_named;
    friend class ::ExtendedZoneSpecifierTest_findTransitionsFromNamedMatch;
    friend class ::ExtendedZoneSpecifierTest_normalizeDateTuple;
    friend class ::ExtendedZoneSpecifierTest_expandDateTuple;
    friend class ::ExtendedZoneSpecifierTest_calcInteriorYears;
    friend class ::ExtendedZoneSpecifierTest_getMostRecentPriorYear;
    friend class ::ExtendedZoneSpecifierTest_compareTransitionToMatchFuzzy;
    friend class ::ExtendedZoneSpecifierTest_compareTransitionToMatch;
    friend class ::ExtendedZoneSpecifierTest_processActiveTransition;
    friend class ::ExtendedZoneSpecifierTest_fixTransitionTimes_generateStartUntilTimes;

    /**
     * Number of Extended Matches. We look at the 3 years straddling the current
     * year, plus the most recent prior year, so that makes 4.
     */
    static const uint8_t kMaxMatches = 4;

    /**
     * Max number of Transitions required for a given Zone, including the most
     * recent prior Transition. The validator.py script shows that it's 7 or 8.
     * TODO: Include this in the generated zonedb* files.
     */
    static const uint8_t kMaxTransitions = 8;

    /**
     * Maximum number of interior years. For a viewing window of 14 months,
     * this will be 4.
     */
    static const uint8_t kMaxInteriorYears = 4;

    /** A sentinel ZoneEra which has the smallest year. */
    static const zonedbx::ZoneEra kAnchorEra;

    bool equals(const ZoneSpecifier& other) const override {
      const auto& that = (const ExtendedZoneSpecifier&) other;
      return getZoneInfo() == that.getZoneInfo();
    }

    /** Return the Transition matching the given epochSeconds. */
    const zonedbx::Transition* findTransition(acetime_t epochSeconds) {
      return mTransitionStorage.findTransition(epochSeconds);
    }

    /** Initialize using the epochSeconds. */
    void init(acetime_t epochSeconds) {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      init(ld);
    }

    /** Initialize the zone rules cache, keyed by the "current" year. */
    void init(const LocalDate& ld) {
      int16_t year = ld.year();
      if (isFilled(year)) return;

      mYear = year;
      mNumMatches = 0; // clear cache
      mTransitionStorage.init();

      zonedbx::YearMonthTuple startYm = {
        (int8_t) (year - LocalDate::kEpochYear - 1), 12 };
      zonedbx::YearMonthTuple untilYm =  {
        (int8_t) (year - LocalDate::kEpochYear + 1), 2 };

      mNumMatches = findMatches(mZoneInfo, startYm, untilYm, mMatches,
          kMaxMatches);
      findTransitions(mTransitionStorage, mMatches, mNumMatches);
      zonedbx::Transition** begin = mTransitionStorage.getActivePoolBegin();
      zonedbx::Transition** end = mTransitionStorage.getActivePoolEnd();
      fixTransitionTimes(begin, end);
      generateStartUntilTimes(begin, end);
      calcAbbreviations(begin, end);

      mIsFilled = true;
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Find the ZoneEras which overlap [startYm, untilYm), ignoring day, time
     * and timeModifier. The start and until fields of the ZoneEra are
     * truncated at the low and high end by startYm and untilYm, respectively.
     * Each matching ZoneEra is wrapped inside a ZoneMatch object, placed in
     * the 'matches' array, and the number of matches is returned.
     */
    static uint8_t findMatches(const zonedbx::ZoneInfo* zoneInfo,
        const zonedbx::YearMonthTuple& startYm,
        const zonedbx::YearMonthTuple& untilYm,
        zonedbx::ZoneMatch* matches, uint8_t maxMatches) {
      uint8_t iMatch = 0;
      const zonedbx::ZoneEra* prev = &kAnchorEra;
      for (uint8_t iEra = 0; iEra < zoneInfo->numEras; iEra++) {
        const zonedbx::ZoneEra* era = &zoneInfo->eras[iEra];
        if (eraOverlapsInterval(prev, era, startYm, untilYm)) {
          if (iMatch < maxMatches) {
            matches[iMatch] = createMatch(prev, era, startYm, untilYm);
            iMatch++;
          }
        }
        prev = era;
      }
      return iMatch;
    }

    /**
     * Determines if era overlaps the interval [startYm, untilYm). This does
     * not need to be exact since the startYm and untilYm are created to have
     * some slop of about one month at the low and high end, so we can ignore
     * the day, time and timeModifier fields of the era. The start date of the
     * current era is represented by the UNTIL fields of the previous era, so
     * the interval of the current era is [era.start=prev.UNTIL,
     * era.until=era.UNTIL). Overlap happens if (era.start < untilYm) and
     * (era.until > startYm).
     */
    static bool eraOverlapsInterval(
        const zonedbx::ZoneEra* prev,
        const zonedbx::ZoneEra* era,
        const zonedbx::YearMonthTuple& startYm,
        const zonedbx::YearMonthTuple& untilYm) {
      return compareEraToYearMonth(prev, untilYm.yearTiny, untilYm.month) < 0
          && compareEraToYearMonth(era, startYm.yearTiny, startYm.month) > 0;
    }

    /** Return (1, 0, -1) depending on how era compares to (yearTiny, month). */
    static int8_t compareEraToYearMonth(const zonedbx::ZoneEra* era,
        int8_t yearTiny, uint8_t month) {
      if (era->untilYearTiny < yearTiny) return -1;
      if (era->untilYearTiny > yearTiny) return 1;
      if (era->untilMonth < month) return -1;
      if (era->untilMonth > month) return 1;
      if (era->untilDay > 1) return 1;
      //if (era->untilTimeCode < 0) return -1; // never possible
      if (era->untilTimeCode > 0) return 1;
      return 0;
    }

    /**
     * Create a ZoneMatch object around the 'era' which intersects the half-open
     * [startYm, untilYm) interval. The interval is assumed to overlap the
     * ZoneEra using the eraOverlapsInterval() method. The 'prev' ZoneEra is
     * needed to define the startDateTime of the current era.
     */
    static zonedbx::ZoneMatch createMatch(
        const zonedbx::ZoneEra* prev,
        const zonedbx::ZoneEra* era,
        const zonedbx::YearMonthTuple& startYm,
        const zonedbx::YearMonthTuple& untilYm) {
      zonedbx::DateTuple startDate = {
        prev->untilYearTiny, prev->untilMonth, prev->untilDay,
        (int8_t) prev->untilTimeCode, prev->untilTimeModifier
      };
      zonedbx::DateTuple lowerBound = {
        startYm.yearTiny, startYm.month, 1, 0, 'w'
      };
      if (startDate < lowerBound) {
        startDate = lowerBound;
      }

      zonedbx::DateTuple untilDate = {
        era->untilYearTiny, era->untilMonth, era->untilDay,
        (int8_t) era->untilTimeCode, era->untilTimeModifier
      };
      zonedbx::DateTuple upperBound = {
        untilYm.yearTiny, untilYm.month, 1, 0, 'w'
      };
      if (upperBound < untilDate) {
        untilDate = upperBound;
      }

      return {startDate, untilDate, era};
    }

    /**
     * Create the Transition objects which are defined by the list of matches
     * and store them in the transitionStorage container.
     */
    static void findTransitions(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        zonedbx::ZoneMatch* matches,
        uint8_t numMatches) {
      for (uint8_t i = 0; i < numMatches; i++) {
        findTransitionsForMatch(transitionStorage, &matches[i]);
      }
    }

    /** Create the Transitions defined by the given match. */
    static void findTransitionsForMatch(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        const zonedbx::ZoneMatch* match) {
      const zonedbx::ZonePolicy* policy = match->era->zonePolicy;
      if (policy == nullptr) {
        findTransitionsFromSimpleMatch(transitionStorage, match);
      } else {
        findTransitionsFromNamedMatch(transitionStorage, match);
      }
    }

    static void findTransitionsFromSimpleMatch(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        const zonedbx::ZoneMatch* match) {
      zonedbx::Transition* freeTransition = transitionStorage.getFreeAgent();
      freeTransition->match = match;
      freeTransition->rule = nullptr;
      freeTransition->transitionTime = match->startDateTime;

      transitionStorage.addFreeAgentToActivePool();
    }

    static void findTransitionsFromNamedMatch(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        const zonedbx::ZoneMatch* match) {
      transitionStorage.resetCandidatePool();
      findCandidateTransitions(transitionStorage, match);
      fixTransitionTimes(
          transitionStorage.getCandidatePoolBegin(),
          transitionStorage.getCandidatePoolEnd());
      selectActiveTransitions(transitionStorage, match);

      transitionStorage.addActiveCandidatesToActivePool();
    }

    static void findCandidateTransitions(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        const zonedbx::ZoneMatch* match) {
      const zonedbx::ZonePolicy* policy = match->era->zonePolicy;
      uint8_t numRules = policy->numRules;
      const zonedbx::ZoneRule* rules = policy->rules;
      int8_t startY = match->startDateTime.yearTiny;
      int8_t endY = match->untilDateTime.yearTiny;

      zonedbx::Transition* prior = transitionStorage.reservePrior();
      prior->active = false; // indicates "no prior transition"
      for (uint8_t r = 0; r < numRules; r++) {
        const zonedbx::ZoneRule* const rule = &rules[r];

        // Add Transitions for interior years
        int8_t interiorYears[kMaxInteriorYears];
        uint8_t numYears = calcInteriorYears(interiorYears, kMaxInteriorYears,
            rule->fromYearTiny, rule->toYearTiny, startY, endY);
        for (uint8_t y = 0; y < numYears; y++) {
          int8_t year = interiorYears[y];
          zonedbx::Transition* t = transitionStorage.getFreeAgent();
          createTransitionForYear(t, year, rule, match);
          int8_t status = compareTransitionToMatchFuzzy(t, match);
          if (status < 0) {
            setAsPriorTransition(transitionStorage, t);
          } else if (status == 1) {
            transitionStorage.addFreeAgentToCandidatePool();
          }
        }

        // Add Transition for prior year
        int8_t priorYear = getMostRecentPriorYear(
            rule->fromYearTiny, rule->toYearTiny, startY, endY);
        if (priorYear != LocalDate::kInvalidYearTiny) {
          zonedbx::Transition* t = transitionStorage.getFreeAgent();
          createTransitionForYear(t, priorYear, rule, match);
          setAsPriorTransition(transitionStorage, t);
        }
      }

      // Add the reserved prior into the Candidate pool, whether or not
      // 'active' is true. The 'active' flag will be recalculated in
      // selectActiveTransitions().
      transitionStorage.addPriorToCandidatePool();
    }

    /** Return true if Transition 't' was created. */
    static void createTransitionForYear(zonedbx::Transition* t, int8_t year,
        const zonedbx::ZoneRule* rule,
        const zonedbx::ZoneMatch* match) {
      t->match = match;
      t->transitionTime = getTransitionTime(year, rule);
      t->rule = rule;
    }

    /**
     * Calculate interior years. Up to maxInteriorYears, usually 3 or 4.
     * Returns the number of interior years.
     */
    static uint8_t calcInteriorYears(int8_t* interiorYears,
        uint8_t maxInteriorYears, int8_t fromYear, int8_t toYear,
        int8_t startYear, int8_t endYear) {
      uint8_t i = 0;
      for (int8_t year = startYear; year <= endYear; year++) {
        if (fromYear <= year && year <= toYear) {
          interiorYears[i] = year;
          i++;
          if (i >= maxInteriorYears) break;
        }
      }
      return i;
    }

    /**
     * Return the most recent prior year of the rule[from_year, to_year].
     * Return LocalDate::kInvalidYearTiny (-128) if the rule[from_year,
     * to_year] has no prior year to the match[start_year, end_year].
     */
    static int8_t getMostRecentPriorYear(int8_t fromYear, int8_t toYear,
        int8_t startYear, int8_t endYear) {
      if (fromYear < startYear) {
        if (toYear < startYear) {
          return toYear;
        } else {
          return startYear - 1;
        }
      } else {
        return LocalDate::kInvalidYearTiny;
      }
    }

    static zonedbx::DateTuple getTransitionTime(
        int8_t year, const zonedbx::ZoneRule* rule) {
      uint8_t dayOfMonth = BasicZoneSpecifier::calcStartDayOfMonth(
          year, rule->inMonth, rule->onDayOfWeek, rule->onDayOfMonth);
      return {year, rule->inMonth, dayOfMonth,
          (int8_t) rule->atTimeCode, rule->atTimeModifier};
    }

    /**
     * Like compareTransitionToMatch() except perform a fuzzy match within at
     * least one-month of the match.start or match.until.
     *
     * Return:
     *     * -1 if t less than match by at least one month
     *     * 1 if t within match,
     *     * 2 if t greater than match by at least one month
     *     * 0 is never returned since we cannot know that t == match.start
     */
    static int8_t compareTransitionToMatchFuzzy(
        const zonedbx::Transition* t, const zonedbx::ZoneMatch* match) {
      int16_t ttMonths = t->transitionTime.yearTiny * 12
          + t->transitionTime.month;

      int16_t matchStartMonths = match->startDateTime.yearTiny * 12
          + match->startDateTime.month;
      if (ttMonths < matchStartMonths - 1) return -1;

      int16_t matchUntilMonths = match->untilDateTime.yearTiny * 12
          + match->untilDateTime.month;
      if (matchUntilMonths + 2 <= ttMonths) return 2;

      return 1;
    }

    /** Set the current transition as the most recent prior if it fits. */
    static void setAsPriorTransition(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        zonedbx::Transition* t) {
      zonedbx::Transition* prior = transitionStorage.getPrior();
      if (prior->active) {
        if (prior->transitionTime < t->transitionTime) {
          t->active = true;
          transitionStorage.setFreeAgentAsPrior();
        }
      } else {
        t->active = true;
        transitionStorage.setFreeAgentAsPrior();
      }
    }

    /**
     * Normalize the transitionTime* fields of the array of Transition objects.
     * Most Transition.transitionTime is given in 'w' mode. However, if it is
     * given in 's' or 'u' mode, we convert these into the 'w' mode for
     * consistency. To convert an 's' or 'u' into 'w', we need the UTC offset
     * of the current Transition, which happens to be given by the *previous*
     * Transition.
     */
    static void fixTransitionTimes(
        zonedbx::Transition** begin, zonedbx::Transition** end) {
      // extend first Transition to -infinity
      zonedbx::Transition* prev = *begin;
      for (zonedbx::Transition** iter = begin; iter != end; ++iter) {
        zonedbx::Transition* curr = *iter;
        expandDateTuple(&curr->transitionTime,
            &curr->transitionTimeS, &curr->transitionTimeU,
            prev->offsetCode(), prev->deltaCode());
        prev = curr;
      }
    }

    /**
     * Convert the given 'tt', offsetCode, and deltaCode into the 'w', 's' and
     * 'u' versions of the DateTuple. The 'tt' may become a 'w' if it was
     * originally 's' or 'u'. On return, tt, tts and ttu are all modified.
     */
    static void expandDateTuple(zonedbx::DateTuple* tt,
        zonedbx::DateTuple* tts, zonedbx::DateTuple* ttu,
        int8_t offsetCode, int8_t deltaCode) {
      if (tt->modifier == 's') {
        *tts = *tt;
        *ttu = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode - offsetCode), 'u'};
        *tt = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode + deltaCode), 'w'};
      } else if (tt->modifier == 'u') {
        *ttu = *tt;
        *tts = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode + offsetCode), 's'};
        *tt = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode + offsetCode + deltaCode), 'w'};
      } else {
        // Explicit set the modifier to 'w' in case it was something else.
        tt->modifier = 'w';
        *tts = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode - deltaCode), 's'};
        *ttu = {tt->yearTiny, tt->month, tt->day,
            (int8_t) (tt->timeCode - deltaCode - offsetCode), 'u'};
      }

      normalizeDateTuple(tt);
      normalizeDateTuple(tts);
      normalizeDateTuple(ttu);
    }

    /** Normalize DateTuple::timeCode if its magnitude is more than 24 hours. */
    static void normalizeDateTuple(zonedbx::DateTuple* dt) {
      const int8_t kOneDayAsCode = 4 * 24;
      if (dt->timeCode <= -kOneDayAsCode) {
        LocalDate ld(dt->yearTiny, dt->month, dt->day);
        local_date_mutation::decrementOneDay(ld);
        dt->yearTiny = ld.yearTiny();
        dt->month = ld.month();
        dt->day = ld.day();
        dt->timeCode += kOneDayAsCode;
      } else if (kOneDayAsCode <= dt->timeCode) {
        LocalDate ld(dt->yearTiny, dt->month, dt->day);
        local_date_mutation::incrementOneDay(ld);
        dt->yearTiny = ld.yearTiny();
        dt->month = ld.month();
        dt->day = ld.day();
        dt->timeCode -= kOneDayAsCode;
      } else {
        // do nothing
      }
    }

    /**
     * Scan through the Candidate transitions, and mark the ones which are
     * active.
     */
    static void selectActiveTransitions(
        zonedbx::TransitionStorage<kMaxTransitions>& transitionStorage,
        const zonedbx::ZoneMatch* match) {
      zonedbx::Transition** begin = transitionStorage.getCandidatePoolBegin();
      zonedbx::Transition** end = transitionStorage.getCandidatePoolEnd();
      zonedbx::Transition* prior = nullptr;
      for (zonedbx::Transition** iter = begin; iter != end; ++iter) {
        zonedbx::Transition* transition = *iter;
        processActiveTransition(match, transition, &prior);
      }

      // If the latest prior transition is found, shift it to start at the
      // startDateTime of the current match.
      if (prior) {
        prior->originalTransitionTime = prior->transitionTime;
        prior->transitionTime = match->startDateTime;
      }
    }

    /**
     * Determine the active status of a transition depending on the temporal
     * relationship to the given match. If the transition is outside of the
     * interval defined by match, then it is inactive. Otherwise active. Also
     * determine the latest prior transition before match, and mark that as
     * active.
     */
    static void processActiveTransition(
        const zonedbx::ZoneMatch* match,
        zonedbx::Transition* transition,
        zonedbx::Transition** prior) {
      int8_t status = compareTransitionToMatch(transition, match);
      if (status == 2) {
        transition->active = false;
      } else if (status == 1) {
        transition->active = true;
      } else if (status == 0) {
        if (*prior) {
          (*prior)->active = false;
        }
        transition->active = true;
        (*prior) = transition;
      } else { // (status < 0)
        if (*prior) {
          if ((*prior)->transitionTime < transition->transitionTime) {
            (*prior)->active = false;
            transition->active = true;
            (*prior) = transition;
          }
        } else {
          transition->active = true;
          (*prior) = transition;
        }
      }
    }

    /**
     * Compare the temporal location of transition compared to the interval
     * defined by  the match. The transition time of the Transition is expanded
     * to include all 3 versions ('w', 's', and 'u') of the time stamp. When
     * comparing against the ZoneMatch.startDateTime and
     * ZoneMatch.untilDateTime, the version will be determined by the modifier
     * of those parameters.
     *
     * Returns:
     *     * -1 if less than match
     *     * 0 if equal to match_start
     *     * 1 if within match,
     *     * 2 if greater than match
     */
    static int8_t compareTransitionToMatch(
        const zonedbx::Transition* transition,
        const zonedbx::ZoneMatch* match) {
      const zonedbx::DateTuple* transitionTime;

      const zonedbx::DateTuple& matchStart = match->startDateTime;
      if (matchStart.modifier == 's') {
        transitionTime = &transition->transitionTimeS;
      } else if (matchStart.modifier == 'u') {
        transitionTime = &transition->transitionTimeU;
      } else { // assume 'w'
        transitionTime = &transition->transitionTime;
      }
      if (*transitionTime < matchStart) return -1;
      if (*transitionTime == matchStart) return 0;

      const zonedbx::DateTuple& matchUntil = match->untilDateTime;
      if (matchUntil.modifier == 's') {
        transitionTime = &transition->transitionTimeS;
      } else if (matchUntil.modifier == 'u') {
        transitionTime = &transition->transitionTimeU;
      } else { // assume 'w'
        transitionTime = &transition->transitionTime;
      }
      if (*transitionTime < matchUntil) return 1;
      return 2;
    }

    /**
     * Generate startDateTime and untilDateTime of the transitions defined by
     * the [start, end) iterators. The Transition::transitionTime should all be
     * in 'w' mode by the time this method is called.
     */
    static void generateStartUntilTimes(
        zonedbx::Transition** begin, zonedbx::Transition** end) {
      zonedbx::Transition* prev = *begin;
      bool isAfterFirst = false;
      for (zonedbx::Transition** iter = begin; iter != end; ++iter) {
        zonedbx::Transition* const t = *iter;
        const zonedbx::DateTuple& tt = t->transitionTime;

        // 1) Update the untilDateTime of the previous Transition
        if (isAfterFirst) {
          prev->untilDateTime = tt;
        }

        // 2) Calculate the current startDateTime by shifting the
        // transitionTime (represented in the UTC offset of the previous
        // transition) into the UTC offset of the *current* transition.
        int8_t code = tt.timeCode - prev->offsetCode() - prev->deltaCode()
            + t->offsetCode() + t->deltaCode();
        t->startDateTime = {tt.yearTiny, tt.month, tt.day, code, tt.modifier};
        normalizeDateTuple(&t->startDateTime);

        // 3) The epochSecond of the 'transitionTime' is determined by the
        // UTC offset of the *previous* Transition. However, the
        // transitionTime can be represented by an illegal time (e.g. 24:00).
        // So, it is better to use the properly normalized startDateTime
        // (calculated above) with the *current* UTC offset.
        //
        // TODO: We should also be able to  calculate this directly from
        // 'transitionTimeU' which should still be a valid field, because it
        // hasn't been clobbered by 'untilDateTime' yet. Not sure if this saves
        // any CPU time though, since we still need to mutiply by 900.
        const zonedbx::DateTuple& st = t->startDateTime;
        const acetime_t offsetSeconds = (acetime_t) 900
            * (st.timeCode - t->offsetCode() - t->deltaCode());
        LocalDate ld(st.yearTiny, st.month, st.day);
        t->startEpochSeconds = ld.toEpochSeconds() + offsetSeconds;

        prev = t;
        isAfterFirst = true;
      }

      // The last Transition's until time is the until time of the ZoneMatch.
      zonedbx::DateTuple untilTime = prev->match->untilDateTime;
      zonedbx::DateTuple untilTimeS; // needed only for expandDateTuple
      zonedbx::DateTuple untilTimeU; // needed only for expandDateTuple
      expandDateTuple(&untilTime, &untilTimeS, &untilTimeU,
          prev->offsetCode(), prev->deltaCode());
      prev->untilDateTime = untilTime;
    }

    /**
     * Calculate the time zone abbreviations for each Transition.
     * There are several cases:
     *     1) 'format' contains 'A/B', meaning 'A' for standard time, and 'B'
     *         for DST time.
     *     2) 'format' contains a %s, which substitutes the 'letter'
     *         2a) If 'letter' is '-', replace with nothing.
     *         2b) The 'format' could be just a '%s'.
     */
    static void calcAbbreviations(
        zonedbx::Transition** begin, zonedbx::Transition** end) {
      for (zonedbx::Transition** iter = begin; iter != end; ++iter) {
        zonedbx::Transition* const t = *iter;
        const char* format = t->format();
        int8_t deltaCode = t->deltaCode();
        uint8_t letter = t->letter();
        // TODO: Incoporate 'letter' that's more than 1-character.
        BasicZoneSpecifier::createAbbreviation(
            t->abbrev, zonedbx::Transition::kAbbrevSize,
            format, deltaCode, letter);
      }
    }

    const zonedbx::ZoneInfo* const mZoneInfo;
    int16_t mYear = 0;
    bool mIsFilled = false;
    uint8_t mNumMatches = 0; // actual number of matches
    zonedbx::ZoneMatch mMatches[kMaxMatches];
    zonedbx::TransitionStorage<kMaxTransitions> mTransitionStorage;
};

} // namespace ace_time

#endif
