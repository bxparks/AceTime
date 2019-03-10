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
#include "AutoZoneSpecifier.h"

class ExtendedZoneSpecifierTest_compareEraToYearMonth;

namespace ace_time {

namespace internal {

struct DateTuple {
  int8_t yearTiny;
  uint8_t month;
  uint8_t day;
  uint8_t timeCode; // 15-min intervals
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

struct YearMonthTuple {
  int8_t yearTiny;
  uint8_t month;
};

/**
 * Data structure that captures the matching ZoneEra and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 */
struct ExtendedZoneMatch {
  /** The effective start time of the matching ZoneEra. */
  DateTuple startDateTime;

  /** The effective until time of the matching ZoneEra. */
  DateTuple untilDateTime;

  /** The ZoneEra that matched the given year. NonNullable. */
  const common::ZoneEra* era;
};

struct ExtendedTransition {
  /**
   * Longest abbreviation seems to be 5 characters.
   * https://www.timeanddate.com/time/zones/
   */
  static const uint8_t kAbbrevSize = 5 + 1;

  /** The match which generated this ExtendedTransition. */
  const ExtendedZoneMatch* zoneMatch; // TODO: Rename to just 'match'?

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-'. We do not support a RULES column that
   * contains a UTC offset. There are only 2 time zones that has this property
   * as of version 2018g: Europe/Istanbul and America/Argentina/San_Luis.
   */
  const common::ZoneRule* rule;

  DateTuple originalTransitionTime;

  DateTuple transitionTime;

  DateTuple transitionTimeS;

  DateTuple transitionTimeU;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /** Determines if this transition is valid. */
  bool active;

  //-------------------------------------------------------------------------

  const char* format() const {
    return zoneMatch->era->format;
  }

  int8_t offsetCode() const {
    return zoneMatch->era->offsetCode;
  }

  char letter() const {
    return rule->letter;
  }

  int8_t deltaCode() const {
    return 0; // TODO: implement
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
 * Manages a collection of ExtendedTransitions, keeping track of unused, used,
 * and active states, using a fixed array of ExtendedTransitions. This class
 * was create to avoid dynamic allocation of memory. We create a fixed sized
 * array, then manage the pool through this class. There are 3 regions in
 * the mPool space:
 * 1) Actives (mNumActive): always starts at 0
 * 2) Prior (1): at index mNumActive
 * 3) Candidates (mNumCandidates): starts at mNumActive + 1
 * 4) Free: starts at mNumActive + mNumCandidates + 1
 *          num = SIZE - mNumActive - mNumCandidates - 1
 */
template<uint8_t SIZE>
class TransitionStorage {
  public:
    /** Constructor. */
    TransitionStorage() {
    }

    void init() {
      for (uint8_t i = 0; i < SIZE; i++) {
        mTransitions[i] = &mPool[i];
      }
      mIndexPrior = 0;
      mIndexCandidates = 0;
      mIndexFree = 0;
    }

    ExtendedTransition* getTransition(uint8_t i) { return mTransitions[i]; }

    ExtendedTransition* getFree() { return mTransitions[mIndexFree]; }

    ExtendedTransition* getPrior() {
      ExtendedTransition* prior = mTransitions[mIndexPrior];
      mIndexCandidates++;
      mIndexFree++;
      return prior;
    }

    /** Empty the Candidates pool. */
    void resetCandidates() {
      mIndexPrior = mIndexFree;
      mIndexCandidates = mIndexFree;
    }

    /**
     * Add the first free Transition at index mIndexFree to the pool of
     * Candidates, sorted by transitionTime. Then increment mIndexFree by one
     * to remove the no-longer free Transition from the Free pool.
     */
    void addFreeToCandidates() {
      for (uint8_t i = mIndexFree; i > mIndexCandidates; i--) {
        ExtendedTransition* curr = mTransitions[i];
        ExtendedTransition* prev = mTransitions[i - 1];
        if (curr->transitionTime < prev->transitionTime) {
          mTransitions[i] = prev;
          mTransitions[i - 1] = curr;
        }
      }
      mIndexFree++;
    }

    /**
     * Immediately add the first free Transition at index mIndexFree to the
     * list of Actives. Then increment mIndexFree to remove the no-longer free
     * Transition from the Free pool. This assumes that the Pending and
     * Candidates pool are empty, which makes the Active pool come immediately
     * before the Free pool.
     */
    void addFreeToActive() {
      mIndexFree++;
    }

    void setFreeAsPrior() {
      // TODO: Implement this
    }

    void addPriorToCandidates() {
      // TODO: Implement this
    }

    /** Return the ExtendedTransition matching the given epochSeconds. */
    const ExtendedTransition* findTransition(acetime_t epochSeconds) {
      const ExtendedTransition* match = nullptr;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        const ExtendedTransition* candidate = mTransitions[i];
        if (candidate->startEpochSeconds <= epochSeconds) {
          match = candidate;
        } else if (candidate->startEpochSeconds > epochSeconds) {
          break;
        }
      }
      return match;
    }

  private:
    ExtendedTransition mPool[SIZE];
    ExtendedTransition* mTransitions[SIZE];
    uint8_t mIndexPrior;
    uint8_t mIndexCandidates;
    uint8_t mIndexFree;
};

}

/**
 * Version of AutoZoneSpecifier that works for more obscure zones
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
    explicit ExtendedZoneSpecifier(const common::ZoneInfo* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    /** Copy constructor. */
    explicit ExtendedZoneSpecifier(const ExtendedZoneSpecifier& that):
      mZoneInfo(that.mZoneInfo),
      mIsFilled(false) {}

    /** Return the underlying ZoneInfo. */
    const common::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    uint8_t getType() const override { return kTypeExtended; }

    /** Return the UTC offset at epochSeconds. */
    UtcOffset getUtcOffset(acetime_t epochSeconds) override {
      if (mZoneInfo == nullptr) return UtcOffset();
      init(epochSeconds);
      const internal::ExtendedTransition* transition =
          findTransition(epochSeconds);
      return UtcOffset::forOffsetCode(transition->offsetCode());
    }

    /** Return the DST delta offset at epochSeconds. */
    UtcOffset getDeltaOffset(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      init(epochSeconds);
      const internal::ExtendedTransition* transition =
          findTransition(epochSeconds);
      if (transition->rule == nullptr) return UtcOffset();
      return UtcOffset::forOffsetCode(transition->rule->deltaCode);
    }

    /** Return the time zone abbreviation. */
    const char* getAbbrev(acetime_t epochSeconds) override {
      if (mZoneInfo == nullptr) return "UTC";
      init(epochSeconds);
      const internal::ExtendedTransition* transition =
          findTransition(epochSeconds);
      return transition->abbrev;
    }

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
    static const common::ZoneEra kAnchorEra;

    bool equals(const ZoneSpecifier& other) const override {
      const auto& that = (const ExtendedZoneSpecifier&) other;
      return getZoneInfo() == that.getZoneInfo();
    }

    /** Return the ExtendedTransition matching the given epochSeconds. */
    const internal::ExtendedTransition* findTransition(acetime_t epochSeconds) {
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

      internal::YearMonthTuple startYm = {
        (int8_t) (year - LocalDate::kEpochYear - 1), 12 };
      internal::YearMonthTuple untilYm =  {
        (int8_t) (year - LocalDate::kEpochYear + 1), 2 };

      findMatches(startYm, untilYm);
      findTransitions();
      fixTransitions();
      generateStartUntilTimes();
      calcAbbreviations();

      mIsFilled = true;
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    void findMatches(
        const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm) {
      uint8_t iMatch = 0;
      const common::ZoneEra* prev = &kAnchorEra;
      for (uint8_t iEra = 0; iEra < mZoneInfo->numEras; iEra++) {
        const common::ZoneEra* era = &mZoneInfo->eras[iEra];
        if (eraOverlapsInterval(prev, era, startYm, untilYm)) {
          if (iMatch < kMaxMatches) {
            mMatches[iMatch] = createMatch(prev, era, startYm, untilYm);
            iMatch++;
          }
        }
        prev = era;
      }
      mNumMatches = iMatch;
    }

    static bool eraOverlapsInterval(
        const common::ZoneEra* prev,
        const common::ZoneEra* era,
        const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm) {
      return compareEraToYearMonth(prev, untilYm.yearTiny, untilYm.month) < 0
          && compareEraToYearMonth(era, startYm.yearTiny, startYm.month) > 0;
    }

    static int8_t compareEraToYearMonth(const common::ZoneEra* era,
        int8_t yearTiny, uint8_t month) {
      if (era->untilYearTiny < yearTiny) return -1;
      if (era->untilYearTiny > yearTiny) return 1;
      if (era->untilMonth < month) return -1;
      if (era->untilMonth > month) return 1;
      if (era->untilDay > 1) return 1;
      if (era->untilTimeCode < 0) return -1;
      if (era->untilTimeCode > 0) return 1;
      return 0;
    }

    internal::ExtendedZoneMatch createMatch(
        const common::ZoneEra* prev,
        const common::ZoneEra* era,
        const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm) {
      internal::DateTuple startDate = {
        prev->untilYearTiny,
        prev->untilMonth,
        prev->untilDay,
        prev->untilTimeCode,
        prev->untilTimeModifier
      };
      internal::DateTuple lowerBound = {
        startYm.yearTiny, startYm.month, 1, 0, 'w'
      };
      if (startDate < lowerBound) {
        startDate = lowerBound;
      }

      internal::DateTuple untilDate = {
        era->untilYearTiny,
        era->untilMonth,
        era->untilDay,
        era->untilTimeCode,
        era->untilTimeModifier
      };
      internal::DateTuple upperBound = {
        untilYm.yearTiny, untilYm.month, 1, 0, 'w'
      };
      if (upperBound < untilDate) {
        untilDate = upperBound;
      }

      return internal::ExtendedZoneMatch{startDate, untilDate, era};
    }

    void findTransitions() {
      for (uint8_t iMatch = 0; iMatch < mNumMatches; iMatch++) {
        findTransitionsForMatch(&mMatches[iMatch]);
      }
    }

    void fixTransitions() {
    }

    void findTransitionsForMatch(const internal::ExtendedZoneMatch* match) {
      const common::ZonePolicy* policy = match->era->zonePolicy;
      if (policy == nullptr) {
        findTransitionsFromSimpleMatch(match);
      } else {
        findTransitionsFromNamedMatch(match);
      }
    }

    void findTransitionsFromSimpleMatch(
        const internal::ExtendedZoneMatch* match) {
      internal::ExtendedTransition* freeTransition =
          mTransitionStorage.getFree();
      freeTransition->zoneMatch = match;
      freeTransition->transitionTime = match->startDateTime;

      mTransitionStorage.addFreeToActive();
    }

    void findTransitionsFromNamedMatch(
        const internal::ExtendedZoneMatch* match) {
      findCandidateTransitions(match);
    }

    void findCandidateTransitions(const internal::ExtendedZoneMatch* match) {
      const common::ZonePolicy* policy = match->era->zonePolicy;
      uint8_t numRules = policy->numRules;
      const common::ZoneRule* rules = policy->rules;
      int8_t startY = match->startDateTime.yearTiny;
      int8_t endY = match->untilDateTime.yearTiny;

      mTransitionStorage.resetCandidates();
      internal::ExtendedTransition* prior = mTransitionStorage.getPrior();
      prior->active = false;
      for (uint8_t r = 0; r < numRules; r++) {
        const common::ZoneRule* const rule = &rules[r];

        // Add Transitions for interior years
        calcInteriorYears(rule->fromYearTiny, rule->toYearTiny, startY, endY);
        for (uint8_t y = 0; y < kMaxInteriorYears; y++) {
          int8_t year = mInteriorYears[y];
          if (year == 0) continue;

          internal::ExtendedTransition* t = mTransitionStorage.getFree();
          createTransitionForYear(t, year, rule, match);
          int8_t status = compareTransitionToMatchFuzzy(t, match);
          if (status < 0) {
            calcPriorTransition(prior, t);
          } else if (status == 1) {
            mTransitionStorage.addFreeToCandidates();
          }
        }

        // Add Transition for prior year
        int8_t priorYear = getMostRecentPriorYear(
            rule->fromYearTiny, rule->toYearTiny, startY, endY);
        if (priorYear >= 0) {
          internal::ExtendedTransition* t = mTransitionStorage.getFree();
          createTransitionForYear(t, priorYear, rule, match);
          calcPriorTransition(prior, t);
        }
      }
      if (prior->active) {
        mTransitionStorage.addPriorToCandidates();
      }
    }

    /** Return true if Transition 't' was created. */
    void createTransitionForYear(internal::ExtendedTransition* t, int8_t year,
        const common::ZoneRule* rule,
        const internal::ExtendedZoneMatch* match) {
      t->zoneMatch = match;
      t->transitionTime = getTransitionTime(year, rule);
      t->rule = rule;
    }

    /** Calculate interior years. Up to 3 years. */
    void calcInteriorYears(int8_t fromYear, int8_t toYear, int8_t startYear,
        int8_t endYear) {
      memset(mInteriorYears, 0, kMaxInteriorYears);
      uint8_t i = 0;
      for (int8_t year = startYear; year <= endYear; year++) {
        if (fromYear <= year && year <= toYear) {
          mInteriorYears[i] = year;
          i++;
        }
      }
    }

    /**
     * Return the most recent prior year of the rule[from_year, to_year].
     * Return -1 if the rule[from_year, to_year] has no prior year to the
     * match[start_year, end_year].
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
        return -1;
      }
    }

    static internal::DateTuple getTransitionTime(
        int8_t year, const common::ZoneRule* rule) {
      uint8_t dayOfMonth = AutoZoneSpecifier::calcStartDayOfMonth(
          year, rule->inMonth, rule->onDayOfWeek, rule->onDayOfMonth);
      return internal::DateTuple{year, rule->inMonth, dayOfMonth,
          rule->atTimeCode, rule->atTimeModifier};
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
        const internal::ExtendedTransition* t,
        const internal::ExtendedZoneMatch* match) {
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

    void calcPriorTransition(
        const internal::ExtendedTransition* prior,
        const internal::ExtendedTransition* t) {
      if (prior->active) {
        if (prior->transitionTime < t->transitionTime) {
          mTransitionStorage.setFreeAsPrior();
        }
      } else {
        mTransitionStorage.setFreeAsPrior();
      }
    }

    static internal::ExtendedZoneMatch calcEffectiveMatch(
        const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm,
        const internal::ExtendedZoneMatch* match) {
      internal::DateTuple startDateTime = match->startDateTime;
      if (compareDateTupleToYearMonth(
          startDateTime, startYm.yearTiny, startYm.month) < 0) {
        startDateTime = {startYm.yearTiny, startYm.month, 1, 0, 'w'};
      }
      internal::DateTuple untilDateTime = match->untilDateTime;
      if (compareDateTupleToYearMonth(
          untilDateTime, untilYm.yearTiny, untilYm.month) > 0) {
        untilDateTime = {untilYm.yearTiny, untilYm.month, 1, 0, 'w'};
      }

      return internal::ExtendedZoneMatch{startDateTime, untilDateTime,
          match->era};
    }

    static int8_t compareDateTupleToYearMonth(const internal::DateTuple& date,
        int8_t yearTiny, uint8_t month) {
      if (date.yearTiny < yearTiny) return -1;
      if (date.yearTiny > yearTiny) return 1;
      if (date.month < month) return -1;
      if (date.month > month) return 1;
      if (date.day > 1) return 1;
      if (date.timeCode < 0) return -1;
      if (date.timeCode > 0) return 1;
      return 0;
    }

    void generateStartUntilTimes() {
      // TODO: implement this
    }

    void calcAbbreviations() {
      // TODO: implement this
    }

    const common::ZoneInfo* mZoneInfo;
    int16_t mYear = 0;
    bool mIsFilled = false;
    uint8_t mNumMatches = 0; // actual number of matches
    internal::ExtendedZoneMatch mMatches[kMaxMatches];
    internal::TransitionStorage<kMaxTransitions> mTransitionStorage;
    int8_t mInteriorYears[kMaxInteriorYears];
};

}

#endif
