#ifndef ACE_TIME_EXTENDED_ZONE_SPECIFIER_H
#define ACE_TIME_EXTENDED_ZONE_SPECIFIER_H

#include <Arduino.h>
#include <string.h> // strchr()
#include <stdint.h>
#include "common/ZonePolicy.h"
#include "common/ZoneInfo.h"
#include "UtcOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneSpecifier.h"
#include "common/logger.h"

class ExtendedZoneSpecifierTest_compareEraToYearMonth;

namespace ace_time {

namespace internal {

struct DateTuple {
  int8_t yearTiny;
  uint8_t month;
  uint8_t day;
  int8_t timeCode; // 15-min intervals
  uint8_t modifier; // 's', 'w', 'u'
};

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

struct Transition {
  /**
   * Longest abbreviation seems to be 5 characters.
   * https://www.timeanddate.com/time/zones/
   */
  static const uint8_t kAbbrevSize = 5 + 1;

  /** The match which generated this Transition. */
  ExtendedZoneMatch zoneMatch;

  DateTuple originalTransitionTime;

  DateTuple transitionTime;

  DateTuple transitionTimeS;

  DateTuple transitionTimeU;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-'. We do not support a RULES column that
   * contains a UTC offset. There are only 2 time zones that has this property
   * as of version 2018g: Europe/Istanbul and America/Argentina/San_Luis.
   */
  const common::ZoneRule* rule;

  const char* format() const {
    return zoneMatch.era->format;
  }

  int8_t offsetCode() const {
    return zoneMatch.era->offsetCode;
  }

  char letter() const {
    return rule->letter;
  }

  int8_t deltaCode() const {
    return 0; // TODO: implement
  }

  /** Used only for debugging. */
  void log() const {
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
      const internal::Transition* transition = findTransition(epochSeconds);
      return UtcOffset::forOffsetCode(transition->offsetCode());
    }

    /** Return the DST delta offset at epochSeconds. */
    UtcOffset getDeltaOffset(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      init(epochSeconds);
      const internal::Transition* transition = findTransition(epochSeconds);
      if (transition->rule == nullptr) return UtcOffset();
      return UtcOffset::forOffsetCode(transition->rule->deltaCode);
    }

    /** Return the time zone abbreviation. */
    const char* getAbbrev(acetime_t epochSeconds) override {
      if (mZoneInfo == nullptr) return "UTC";
      init(epochSeconds);
      const internal::Transition* transition = findTransition(epochSeconds);
      return transition->abbrev;
    }

    bool equals(const ZoneSpecifier& that) const override {
      const auto& other = (const ExtendedZoneSpecifier&) that;
      return *this == other;
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
    friend bool operator==(const ExtendedZoneSpecifier& a,
        const ExtendedZoneSpecifier& b);

    friend class ::ExtendedZoneSpecifierTest_compareEraToYearMonth;

    /**
     * Number of Extended Matches. We look at the 3 years straddling the current
     * year, plus the most recent prior year, so that makes 4.
     */
    static const uint8_t kMaxMatches = 4;

    /**
     * Max number of Transitions required for a given Zone. The validator.py
     * script shows that it's 5. TODO: Maybe include that as a preprocessor
     * macro in a file in the generated zonedb/ files.
     */
    static const uint8_t kMaxTransitions = 5;

    /** A sentinel ZoneEra which has the smallest year. */
    static const common::ZoneEra kAnchorEra;

    /** Return the Transition matching the given epochSeconds. */
    const internal::Transition* findTransition(acetime_t epochSeconds) {
      const internal::Transition* match = nullptr;
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        const internal::Transition* candidate = &mTransitions[i];
        if (candidate->startEpochSeconds <= epochSeconds) {
          match = candidate;
        } else if (candidate->startEpochSeconds > epochSeconds) {
          break;
        }
      }
      return match;
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
      mNumTransitions = 0; // clear cache

      internal::YearMonthTuple startYm = {
        (int8_t) (year - LocalDate::kEpochYear - 1), 12 };
      internal::YearMonthTuple untilYm =  {
        (int8_t) (year - LocalDate::kEpochYear + 1), 2 };

      findMatches(startYm, untilYm);
      findTransitions(startYm, untilYm);
      generateStartUntilTimes();
      calcAbbreviations();

      mIsFilled = true;
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    void findMatches(const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm) {
      uint8_t iMatch = 0;
      const common::ZoneEra* prev = &kAnchorEra;
      for (uint8_t iEra = 0; iEra < mZoneInfo->numEras; iEra++) {
        const common::ZoneEra* era = &mZoneInfo->eras[iEra];
        if (eraOverlapsInterval(prev, era, startYm, untilYm)) {
          if (iMatch < kMaxMatches) {
            mMatches[iMatch] = createMatch(prev, era);
            iMatch++;
          }
        }
        prev = era;
      }
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
        const common::ZoneEra* prev, const common::ZoneEra* era) {
      internal::DateTuple startDate = {
        prev->untilYearTiny,
        prev->untilMonth,
        prev->untilDay,
        prev->untilTimeCode,
        prev->untilTimeModifier
      };
      internal::DateTuple untilDate = {
        era->untilYearTiny,
        era->untilMonth,
        era->untilDay,
        era->untilTimeCode,
        era->untilTimeModifier
      };
      return internal::ExtendedZoneMatch{startDate, untilDate, era};
    }

    void findTransitions(const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm) {
      mNumTransitions = 0;
      for (uint8_t iMatch = 0; iMatch < mNumMatches; iMatch++) {
        addTransitionsFromMatch(startYm, untilYm, &mMatches[iMatch]);
      }

      fixTransitions();
    }

    void fixTransitions() {
      // TODO: implement this
    }

    void addTransitionsFromMatch(
        const internal::YearMonthTuple& startYm,
        const internal::YearMonthTuple& untilYm,
        const internal::ExtendedZoneMatch* match) {
      const common::ZoneEra* era = match->era;
      const common::ZonePolicy* policy = era->zonePolicy;
      internal::ExtendedZoneMatch effMatch = calcEffectiveMatch(
          startYm, untilYm, match);
      if (policy == nullptr) {
        addTransitionsFromSimpleMatch(match);
      } else {
        addTransitionsFromNamedMatch(match);
      }
    }

    void addTransitionsFromSimpleMatch(
        const internal::ExtendedZoneMatch* match) {
      internal::Transition transition = {
        *match,
        internal::DateTuple(),
        match->startDateTime
      };
      if (mNumTransitions < kMaxTransitions) {
        mTransitions[mNumTransitions] = transition;
        mNumTransitions++;
      }
    }

    void addTransitionsFromNamedMatch(
        const internal::ExtendedZoneMatch* match) {
      // TODO: implement this
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
    uint8_t mNumTransitions = 0;; // actual number of transitions
    internal::Transition mTransitions[kMaxTransitions];
};

inline bool operator==(const ExtendedZoneSpecifier& a,
    const ExtendedZoneSpecifier& b) {
  return a.getZoneInfo() == b.getZoneInfo();
}

inline bool operator!=(const ExtendedZoneSpecifier& a,
    const ExtendedZoneSpecifier& b) {
  return ! (a == b);
}

}

#endif
