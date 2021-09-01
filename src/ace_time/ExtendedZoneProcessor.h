/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_H
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_H

#include <string.h> // memcpy()
#include <stdint.h> // uintptr_t
#include <AceCommon.h> // copyReplaceString()
#include "common/compat.h"
#include "internal/ZonePolicy.h"
#include "internal/ZoneInfo.h"
#include "internal/ExtendedBrokers.h"
#include "common/logging.h"
#include "TimeOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneProcessor.h"
#include "local_date_mutation.h"

#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG 0

class ExtendedZoneProcessorTest_compareEraToYearMonth;
class ExtendedZoneProcessorTest_compareEraToYearMonth2;
class ExtendedZoneProcessorTest_createMatch;
class ExtendedZoneProcessorTest_findMatches_simple;
class ExtendedZoneProcessorTest_findMatches_named;
class ExtendedZoneProcessorTest_findCandidateTransitions;
class ExtendedZoneProcessorTest_findTransitionsFromNamedMatch;
class ExtendedZoneProcessorTest_getTransitionTime;
class ExtendedZoneProcessorTest_createTransitionForYear;
class ExtendedZoneProcessorTest_normalizeDateTuple;
class ExtendedZoneProcessorTest_expandDateTuple;
class ExtendedZoneProcessorTest_calcInteriorYears;
class ExtendedZoneProcessorTest_getMostRecentPriorYear;
class ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
class ExtendedZoneProcessorTest_compareTransitionToMatch;
class ExtendedZoneProcessorTest_processActiveTransition;
class ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
class ExtendedZoneProcessorTest_createAbbreviation;
class ExtendedZoneProcessorTest_setZoneKey;
class TransitionStorageTest_getFreeAgent;
class TransitionStorageTest_getFreeAgent2;
class TransitionStorageTest_addFreeAgentToActivePool;
class TransitionStorageTest_reservePrior;
class TransitionStorageTest_addFreeAgentToCandidatePool;
class TransitionStorageTest_setFreeAgentAsPrior;
class TransitionStorageTest_addActiveCandidatesToActivePool;
class TransitionStorageTest_resetCandidatePool;
class TransitionStorageTest_findTransitionForDateTime;

class Print;

namespace ace_time {
namespace extended {

/**
 * A tuple that represents a date and time. Packed to 4-byte boundaries to
 * save space on 32-bit processors.
 */
struct DateTuple {
  DateTuple() = default;

  DateTuple(int8_t y, uint8_t mon, uint8_t d, int16_t min, uint8_t mod):
      yearTiny(y), month(mon), day(d), suffix(mod), minutes(min) {}

  int8_t yearTiny; // [-127, 126], 127 will cause bugs
  uint8_t month; // [1-12]
  uint8_t day; // [1-31]
  uint8_t suffix; // kSuffixS, kSuffixW, kSuffixU
  int16_t minutes; // negative values allowed

  /** Used only for debugging. */
  void log() const {
    if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
      char c = "wsu"[(suffix>>4)];
      logging::printf("DateTuple(%04d-%02u-%02uT%d'%c')",
          yearTiny+LocalDate::kEpochYear, month, day, minutes, c);
    }
  }
};

/** Determine if DateTuple a is less than DateTuple b, ignoring the suffix. */
inline bool operator<(const DateTuple& a, const DateTuple& b) {
  if (a.yearTiny < b.yearTiny) return true;
  if (a.yearTiny > b.yearTiny) return false;
  if (a.month < b.month) return true;
  if (a.month > b.month) return false;
  if (a.day < b.day) return true;
  if (a.day > b.day) return false;
  if (a.minutes < b.minutes) return true;
  if (a.minutes > b.minutes) return false;
  return false;
}

inline bool operator>=(const DateTuple& a, const DateTuple& b) {
  return ! (a < b);
}

inline bool operator<=(const DateTuple& a, const DateTuple& b) {
  return ! (b < a);
}

inline bool operator>(const DateTuple& a, const DateTuple& b) {
  return (b < a);
}

/** Determine if DateTuple a is equal to DateTuple b, including the suffix. */
inline bool operator==(const DateTuple& a, const DateTuple& b) {
  return a.yearTiny == b.yearTiny
      && a.month == b.month
      && a.day == b.day
      && a.minutes == b.minutes
      && a.suffix == b.suffix;
}

/** A simple tuple to represent a year/month pair. */
struct YearMonthTuple {
  int8_t yearTiny;
  uint8_t month;
};

/**
 * Data structure that captures the matching ZoneEra and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 *
 * @tparam ZEB type of ZoneEraBroker
 */
template<typename ZEB>
struct MatchingEraTemplate {
  /** The effective start time of the matching ZoneEra. */
  DateTuple startDateTime;

  /** The effective until time of the matching ZoneEra. */
  DateTuple untilDateTime;

  /** The ZoneEra that matched the given year. NonNullable. */
  ZEB era;

  void log() const {
    logging::printf("MatchingEra(");
    logging::printf("Start:"); startDateTime.log();
    logging::printf("; Until:"); untilDateTime.log();
    logging::printf("; Era: %snull", (era.isNull()) ? "" : "!");
    logging::printf(")");
  }
};

/**
 * Represents an interval of time where the time zone obeyed a certain UTC
 * offset and DST delta. The start of the interval is given by 'transitionTime'
 * which comes from the TZ Database file. The actual start and until time of
 * the interval (in the local time zone) is given by 'startDateTime' and
 * 'untilDateTime'.
 *
 * There are 2 types of Transition instances:
 *  1) Simple, indicated by 'rule' == nullptr. The base UTC offsetMinutes is
 *  given by match->offsetMinutes. The additional DST delta is given by
 *  match->deltaMinutes.
 *  2) Named, indicated by 'rule' != nullptr. The base UTC offsetMinutes is
 *  given by match->offsetMinutes. The additional DST delta is given by
 *  rule->deltaMinutes.
 *
 * The 'match', 'rule', 'transitionTime', 'transitionTimeS', 'transitionTimeU',
 * 'active', 'originalTransitionTime', 'letter()' and 'format()' are temporary
 * variables or parameters used in the init() method.
 *
 * The 'offsetMinutes', 'deltaMinutes', 'startDateTime', 'abbrev' are the
 * derived parameters used in the findTransition() search.
 *
 * Ordering of fields optimized along 4-byte boundaries to help 32-bit
 * processors without making the program size bigger for 8-bit processors.
 *
 * @tparm ZEB type of ZoneEraBroker
 * @tparm ZPB type of ZonePolicyBroker
 * @tparm ZRB type of ZoneRuleBroker
 */
template <typename ZEB, typename ZPB, typename ZRB>
struct TransitionTemplate {
  /** The match which generated this Transition. */
  const MatchingEraTemplate<ZEB>* match;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-', indicating that the MatchingEra was
   * a "simple" ZoneEra.
   */
  ZRB rule;

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
     * *previous* Transition. Valid before
     * ExtendedZoneProcessor::generateStartUntilTimes() is called.
     */
    DateTuple transitionTimeS;

    /**
     * Start time expressed using the UTC offset of the current Transition.
     * Valid after ExtendedZoneProcessor::generateStartUntilTimes() is called.
     */
    DateTuple startDateTime;
  };

  union {
    /**
     * Version of transitionTime in 'u' mode, using the UTC offset of the
     * *previous* transition. Valid before
     * ExtendedZoneProcessor::generateStartUntilTimes() is called.
     */
    DateTuple transitionTimeU;

    /**
     * Until time expressed using the UTC offset of the current Transition.
     * Valid after ExtendedZoneProcessor::generateStartUntilTimes() is called.
     */
    DateTuple untilDateTime;
  };

  /**
   * If the transition is shifted to the beginning of a MatchingEra, this is set
   * to the transitionTime for debugging. May be removed in the future.
   */
  DateTuple originalTransitionTime;

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /**
   * The base offset minutes, not the total effective UTC offset. Note that
   * this is different than basic::Transition::offsetMinutes used by
   * BasicZoneProcessor which is the total effective offsetMinutes. (It may be
   * possible to make this into an effective offsetMinutes (i.e. offsetMinutes
   * + deltaMinutes) but it does not seem worth making that change right now.)
   */
  int16_t offsetMinutes;

  /** The DST delta minutes. */
  int16_t deltaMinutes;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[internal::kAbbrevSize];

  /** Storage for the single letter 'letter' field if 'rule' is not null. */
  char letterBuf[2];

  /**
   * Flag used for 2 slightly different meanings at different stages of init()
   * processing.
   *
   * 1) During findCandidateTransitions(), this flag indicates whether the
   * current transition is a valid "prior" transition that occurs before other
   * transitions. It is set by setAsPriorTransition() if the transition is a
   * prior transition.
   *
   * 2) During processActiveTransition(), this flag indicates if the current
   * transition falls within the date range of interest.
   */
  bool active;

  //-------------------------------------------------------------------------

  const char* format() const {
    return match->era.format();
  }

  /**
   * Return the letter string. Returns nullptr if the RULES column is empty
   * since that means that the ZoneRule is not used, which means LETTER does
   * not exist. A LETTER of '-' is returned as an empty string "".
   */
  const char* letter() const {
    // RULES column is '-' or hh:mm, so return nullptr to indicate this.
    if (rule.isNull()) {
      return nullptr;
    }

    // RULES point to a named rule, and LETTER is a single, printable
    // character.
    char letter = rule.letter();
    if (letter >= 32) {
      return letterBuf;
    }

    // RULES points to a named rule, and the LETTER is a string. The
    // rule->letter is a non-printable number < 32, which is an index into
    // a list of strings given by match->era->zonePolicy->letters[].
    const ZPB policy = match->era.zonePolicy();
    uint8_t numLetters = policy.numLetters();
    if (letter >= numLetters) {
      // This should never happen unless there is a programming error. If it
      // does, return an empty string. (createTransitionForYear() sets
      // letterBuf to a NUL terminated empty string if rule->letter < 32)
      return letterBuf;
    }

    // Return the string at index 'rule->letter'.
    return policy.letter(letter);
  }

  /** Used only for debugging. */
  void log() const {
    logging::printf("Transition(");
    if (sizeof(acetime_t) <= sizeof(int)) {
      logging::printf("sE: %d", startEpochSeconds);
    } else {
      logging::printf("sE: %ld", startEpochSeconds);
    }
    logging::printf("; match: %snull", (match) ? "!" : "");
    logging::printf("; era: %snull",
        (match && match->era.isNull()) ? "" : "!");
    logging::printf("; oMinutes: %d", offsetMinutes);
    logging::printf("; dMinutes: %d", deltaMinutes);
    logging::printf("; tt: "); transitionTime.log();
    if (! rule.isNull()) {
      logging::printf("; R.fY: %d", rule.fromYearTiny());
      logging::printf("; R.tY: %d", rule.toYearTiny());
      logging::printf("; R.M: %d", rule.inMonth());
      logging::printf("; R.dow: %d", rule.onDayOfWeek());
      logging::printf("; R.dom: %d", rule.onDayOfMonth());
    }
  }
};

/**
 * A heap manager which is specialized and tuned to manage a collection of
 * Transitions, keeping track of unused, used, and active states, using a fixed
 * array of Transitions. Its main purpose is to provide some illusion of
 * dynamic memory allocation without actually performing any dynamic memory
 * allocation.
 *
 * We create a fixed sized array for the total pool, determined by the template
 * parameter SIZE, then manage the various sub-pools of Transition objects.
 * The allocation of the various sub-pools is intricately tied to the precise
 * pattern of creation and release of the various Transition objects within the
 * ExtendedZoneProcessor class.
 *
 * There are 4 pools indicated by the following half-open (exclusive) index
 * ranges:
 *
 * 1) Active pool: [0, mIndexPrior)
 * 2) Prior pool: [mIndexPrior, mIndexCandidates), either 0 or 1 element
 * 3) Candidate pool: [mIndexCandidates, mIndexFree)
 * 4) Free pool: [mIndexFree, SIZE)
 *
 * At the completion of the ExtendedZoneProcessor::init(LocalDate& ld) method,
 * the Active pool will contain the active Transitions relevant to the
 * 'year' defined by the LocalDate. The Prior and Candidate pools will be
 * empty, with the Free pool taking up the remaining space.
 *
 * @tparam SIZE size of internal cache
 * @tparam ZEB type of ZoneEraBroker
 * @tparam ZPB type of ZonePolicyBroker
 * @tparam ZRB type of ZoneRuleBroker
 */
template<uint8_t SIZE, typename ZEB, typename ZPB, typename ZRB>
class TransitionStorageTemplate {
  public:
    /**
     * Template instantiation of TransitionTemplate used by this class. This
     * should be treated as a private, it is exposed only for testing purposes.
     */
    typedef TransitionTemplate<ZEB, ZPB, ZRB> Transition;

    /** Constructor. */
    TransitionStorageTemplate() {}

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
    Transition* getPrior() {
      return mTransitions[mIndexPrior];
    }

    /** Swap 2 transitions. */
    static void swap(Transition** a, Transition** b) {
      auto* tmp = *a;
      *a = *b;
      *b = tmp;
    }

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

    Transition** getActivePoolBegin() {
      return &mTransitions[0];
    }
    Transition** getActivePoolEnd() {
      return &mTransitions[mIndexFree];
    }

    /**
     * Return a pointer to the first Transition in the free pool. If this
     * transition is not used, then it's ok to just drop it. The next time
     * getFreeAgent() is called, the same Transition will be returned.
     */
    Transition* getFreeAgent() {
      // Set the internal high water mark. If that index becomes SIZE,
      // then we know we have an overflow.
      if (mIndexFree > mHighWater) {
        mHighWater = mIndexFree;
      }

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
      mIndexFree++;
      mIndexPrior = mIndexFree;
      mIndexCandidates = mIndexFree;
    }

    /**
     * Allocate one Transition just after the Active pool, but before the
     * Candidate pool, to keep the most recent prior Transition. Shift the
     * Candidate pool and Free pool up by one.
     */
    Transition** reservePrior() {
      mIndexCandidates++;
      mIndexFree++;
      return &mTransitions[mIndexPrior];
    }

    /** Swap the Free agrent transition with the current Prior transition. */
    void setFreeAgentAsPrior() {
      swap(&mTransitions[mIndexPrior], &mTransitions[mIndexFree]);
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
     * free agent from the Free pool. Essentially this is an Insertion Sort
     * keyed by the 'transitionTime' (ignoring the DateTuple.suffix).
     */
    void addFreeAgentToCandidatePool() {
      if (mIndexFree >= SIZE) return;
      for (uint8_t i = mIndexFree; i > mIndexCandidates; i--) {
        Transition* curr = mTransitions[i];
        Transition* prev = mTransitions[i - 1];
        if (curr->transitionTime >= prev->transitionTime) break;
        mTransitions[i] = prev;
        mTransitions[i - 1] = curr;
      }
      mIndexFree++;
    }

    /**
     * Add active candidates into the Active pool, and collapse the Candidate
     * pool.
     */
    void addActiveCandidatesToActivePool() {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addActiveCandidatesToActivePool()\n");
      }
      uint8_t iActive = mIndexPrior;
      uint8_t iCandidate = mIndexCandidates;
      for (; iCandidate < mIndexFree; iCandidate++) {
        if (mTransitions[iCandidate]->active) {
          if (iActive != iCandidate) {
            swap(&mTransitions[iActive], &mTransitions[iCandidate]);
          }
          ++iActive;
        }
      }
      mIndexPrior = iActive;
      mIndexCandidates = iActive;
      mIndexFree = iActive;
    }

    /**
     * Return the Transition matching the given epochSeconds. Return nullptr if
     * no matching Transition found. If a zone does not have any transition
     * according to TZ Database, the AceTimeTools/transformer.py script adds an
     * "anchor" transition at the "beginning of time" which happens to be the
     * year 1872 (because the year is stored as an int8_t). Therefore, this
     * method should never return a nullptr for a well-formed ZoneInfo file.
     */
    const Transition* findTransition(acetime_t epochSeconds) const {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf( "findTransition(): mIndexFree: %d\n", mIndexFree);
      }

      const Transition* match = nullptr;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        const Transition* candidate = mTransitions[i];
        if (candidate->startEpochSeconds > epochSeconds) break;
        match = candidate;
      }
      return match;
    }

    /**
     * Return the Transition matching the given dateTime. Return nullptr if no
     * matching Transition found. During DST changes, a particlar LocalDateTime
     * may correspond to 2 Transitions or 0 Transitions, and there are
     * potentially multiple ways to handle this. This method implements the
     * following algorithm:
     *
     * 1) If the localDateTime falls in the DST transition gap where 0
     * Transitions ought to be found, e.g. between 02:00 and 03:00 in
     * America/Los_Angeles when standard time switches to DST time), the
     * immediate prior Transition is returned (in effect extending the UTC
     * offset of the prior Transition through the gap. For example, when DST
     * starts, 02:00 becomes 03:00, so a time of 02:30 does not exist, but the
     * Transition returned will be the one valid at 01:59. When it is converted
     * to epoch_seconds and converted back to a LocalDateTime, the 02:30 time
     * will become 03:30, since the later UTC offset will be used.
     *
     * 2) If the localDateTime falls in a time period where there are 2
     * Transitions, hence 2 valid UTC offsets, the later Transition is
     * returned. For example, when DST ends in America/Los_Angeles, 02:00
     * becomes 01:00, so a time of 01:30 could belong to the earlier or later
     * Transition. This method returns the later Transition.
     */
    const Transition* findTransitionForDateTime(const LocalDateTime& ldt)
        const {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf(
            "findTransitionForDateTime(): mIndexFree: %d\n", mIndexFree);
      }

      // Convert LocalDateTime to DateTuple.
      DateTuple localDate = { ldt.yearTiny(), ldt.month(), ldt.day(),
          (int16_t) (ldt.hour() * 60 + ldt.minute()),
          internal::ZoneContext::kSuffixW };
      const Transition* match = nullptr;

      // Find the last Transition that matches
      for (uint8_t i = 0; i < mIndexFree; i++) {
        const Transition* candidate = mTransitions[i];
        if (candidate->startDateTime > localDate) break;
        match = candidate;
      }
      return match;
    }

    /** Verify that the indexes are valid. Used only for debugging. */
    void log() const {
      logging::printf("TransitionStorage:\n");
      logging::printf("  mIndexPrior: %d\n", mIndexPrior);
      logging::printf("  mIndexCandidates: %d\n", mIndexCandidates);
      logging::printf("  mIndexFree: %d\n", mIndexFree);
      if (mIndexPrior != 0) {
        logging::printf("  Actives:\n");
        for (uint8_t i = 0; i < mIndexPrior; i++) {
          mTransitions[i]->log();
          logging::printf("\n");
        }
      }
      if (mIndexPrior != mIndexCandidates) {
        logging::printf("  Prior: ");
        mTransitions[mIndexPrior]->log();
        logging::printf("\n");
      }
      if (mIndexCandidates != mIndexFree) {
        logging::printf("  Candidates:\n");
        for (uint8_t i = mIndexCandidates; i < mIndexFree; i++) {
          mTransitions[i]->log();
          logging::printf("\n");
        }
      }
    }

    /** Reset the high water mark. For debugging. */
    void resetHighWater() { mHighWater = 0; }

    /**
     * Return the high water mark. This is the largest value of mIndexFree that
     * was used. If this returns SIZE, it indicates that the Transition mPool
     * overflowed. For debugging.
     */
    uint8_t getHighWater() const { return mHighWater; }

  private:
    friend class ::TransitionStorageTest_getFreeAgent;
    friend class ::TransitionStorageTest_getFreeAgent2;
    friend class ::TransitionStorageTest_addFreeAgentToActivePool;
    friend class ::TransitionStorageTest_reservePrior;
    friend class ::TransitionStorageTest_addFreeAgentToCandidatePool;
    friend class ::TransitionStorageTest_setFreeAgentAsPrior;
    friend class ::TransitionStorageTest_addActiveCandidatesToActivePool;
    friend class ::TransitionStorageTest_findTransitionForDateTime;
    friend class ::TransitionStorageTest_resetCandidatePool;

    /** Return the transition at position i. */
    Transition* getTransition(uint8_t i) {
      return mTransitions[i];
    }

    Transition mPool[SIZE];
    Transition* mTransitions[SIZE];
    uint8_t mIndexPrior;
    uint8_t mIndexCandidates;
    uint8_t mIndexFree;

    /** High water mark. For debugging. */
    uint8_t mHighWater = 0;
};

} // namespace extended

/**
 * An implementation of ZoneProcessor that supports for *all* zones defined by
 * the TZ Database. The supported zones are defined in the zonedbx/zone_infos.h
 * header file. The constructor expects a pointer to one of the ZoneInfo
 * structures declared in the zonedbx/zone_infos.h file. The zone_specifier.py
 * file is the initial Python implementation of this class, which got
 * translated into C++.
 *
 * Currently (as of v0.7), the underlying zoneinfo files (extended::ZoneInfo,
 * etc) store the UTC and DST offsets of a timezone as a single signed byte in
 * 15-minute increments. This is sufficient to accurate describe all time zones
 * from the year 2000 until  2050. The AT and UNTIL transition times are stored
 * using a 1-minute resolution, which correctly handles the 5 timezones whose
 * DST transition times occur at 00:01. Those zones are:
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
     * Max number of Transitions required for a given Zone, including the most
     * recent prior Transition. This value for each Zone is given by the
     * kZoneBufSize{zoneName} constant in the generated
     * `zonedb[x]/zone_infos.h` file. The ExtendedPythonTest and
     * ExtendedJavaTest tests show that the maximum is 7. Set this to 8 for
     * safety.
     */
    static const uint8_t kMaxTransitions = 8;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionTemplate<ZEB, ZPB, ZRB> Transition;

    /** Exposed only for testing purposes. */
    typedef extended::MatchingEraTemplate<ZEB> MatchingEra;

    /** Exposed only for testing purposes. */
    typedef extended::TransitionStorageTemplate<kMaxTransitions, ZEB, ZPB, ZRB>
        TransitionStorage;

    uint32_t getZoneId() const override { return mZoneInfoBroker.zoneId(); }

    TimeOffset getUtcOffset(acetime_t epochSeconds) const override {
      bool success = init(epochSeconds);
      if (!success) return TimeOffset::forError();
      const Transition* transition = findTransition(epochSeconds);
      return (transition)
          ? TimeOffset::forMinutes(
              transition->offsetMinutes + transition->deltaMinutes)
          : TimeOffset::forError();
    }

    TimeOffset getDeltaOffset(acetime_t epochSeconds) const override {
      bool success = init(epochSeconds);
      if (!success) return TimeOffset::forError();
      const Transition* transition = findTransition(epochSeconds);
      return TimeOffset::forMinutes(transition->deltaMinutes);
    }

    const char* getAbbrev(acetime_t epochSeconds) const override {
      bool success = init(epochSeconds);
      if (!success) return "";
      const Transition* transition = findTransition(epochSeconds);
      return transition->abbrev;
    }

    OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt) const override {
      TimeOffset offset;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("getOffsetDateTime(): ldt=");
        ldt.printTo(SERIAL_PORT_MONITOR);
        SERIAL_PORT_MONITOR.println();
      }
      bool success = init(ldt.localDate());

      // Find the Transition to get the DST offset
      if (success) {
        const Transition* transition =
            mTransitionStorage.findTransitionForDateTime(ldt);
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf("getOffsetDateTime(): match transition=");
          transition->log();
          logging::printf("\n");
        }
        offset = (transition)
            ? TimeOffset::forMinutes(
                transition->offsetMinutes + transition->deltaMinutes)
            : TimeOffset::forError();
      } else {
        offset = TimeOffset::forError();
      }

      auto odt = OffsetDateTime::forLocalDateTimeAndOffset(ldt, offset);
      if (offset.isError()) {
        return odt;
      }
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("getOffsetDateTime(): odt=");
        odt.printTo(SERIAL_PORT_MONITOR);
        SERIAL_PORT_MONITOR.println();
      }

      // Normalize the OffsetDateTime, causing LocalDateTime in the DST
      // transtion gap to be shifted forward one hour. For LocalDateTime in an
      // overlap (DST->STD transition), the earlier UTC offset is selected// by
      // findTransitionForDateTime(). Use that to calculate the epochSeconds,
      // then recalculate the offset. Use this final offset to determine the
      // effective OffsetDateTime that will survive a round-trip unchanged.
      acetime_t epochSeconds = odt.toEpochSeconds();
      const Transition* transition =
          mTransitionStorage.findTransition(epochSeconds);
      offset =  (transition)
            ? TimeOffset::forMinutes(
                transition->offsetMinutes + transition->deltaMinutes)
            : TimeOffset::forError();
      odt = OffsetDateTime::forEpochSeconds(epochSeconds, offset);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("getOffsetDateTime(): normalized(odt)=");
        odt.printTo(SERIAL_PORT_MONITOR);
        SERIAL_PORT_MONITOR.println();
      }
      return odt;
    }

    void printNameTo(Print& printer) const override {
      mZoneInfoBroker.printNameTo(printer);
    }

    void printShortNameTo(Print& printer) const override {
      mZoneInfoBroker.printShortNameTo(printer);
    }

    /** Used only for debugging. */
    void log() const {
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

    /** Reset the TransitionStorage high water mark. For debugging. */
    void resetTransitionHighWater() {
      mTransitionStorage.resetHighWater();
    }

    /** Get the TransitionStorage high water mark. For debugging. */
    uint8_t getTransitionHighWater() const {
      return mTransitionStorage.getHighWater();
    }

    void setZoneKey(uintptr_t zoneKey) override {
      if (mZoneInfoBroker.equals(zoneKey)) return;

      mZoneInfoBroker = mBrokerFactory->createZoneInfoBroker(zoneKey);
      mYear = 0;
      mIsFilled = false;
      mNumMatches = 0;
      resetTransitionHighWater();
    }

    bool equalsZoneKey(uintptr_t zoneKey) const override {
      return mZoneInfoBroker.equals(zoneKey);
    }

    void setBrokerFactory(const BF* brokerFactory) {
      mBrokerFactory = brokerFactory;
    }

  protected:

    /**
     * Constructor.
     *
     * @param brokerFactory pointer to a BrokerFactory that creates a ZIB
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*)
     */
    explicit ExtendedZoneProcessorTemplate(
        uint8_t type,
        const BF* brokerFactory,
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
    friend class ::ExtendedZoneProcessorTest_createMatch;
    friend class ::ExtendedZoneProcessorTest_findMatches_simple;
    friend class ::ExtendedZoneProcessorTest_findMatches_named;
    friend class ::ExtendedZoneProcessorTest_findCandidateTransitions;
    friend class ::ExtendedZoneProcessorTest_findTransitionsFromNamedMatch;
    friend class ::ExtendedZoneProcessorTest_getTransitionTime;
    friend class ::ExtendedZoneProcessorTest_createTransitionForYear;
    friend class ::ExtendedZoneProcessorTest_normalizeDateTuple;
    friend class ::ExtendedZoneProcessorTest_expandDateTuple;
    friend class ::ExtendedZoneProcessorTest_calcInteriorYears;
    friend class ::ExtendedZoneProcessorTest_getMostRecentPriorYear;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatchFuzzy;
    friend class ::ExtendedZoneProcessorTest_compareTransitionToMatch;
    friend class ::ExtendedZoneProcessorTest_processActiveTransition;
    friend class ::ExtendedZoneProcessorTest_fixTransitionTimes_generateStartUntilTimes;
    friend class ::ExtendedZoneProcessorTest_createAbbreviation;
    friend class ::ExtendedZoneProcessorTest_setZoneKey;
    friend class ::TransitionStorageTest_findTransitionForDateTime;

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
     * this will be 4.
     */
    static const uint8_t kMaxInteriorYears = 4;

    bool equals(const ZoneProcessor& other) const override {
      return mZoneInfoBroker.equals(
          ((const ExtendedZoneProcessorTemplate&) other).mZoneInfoBroker);
    }

    /**
     * Return the Transition matching the given epochSeconds. Returns nullptr
     * if no matching Transition found.
     */
    const Transition* findTransition(acetime_t epochSeconds) const {
      return mTransitionStorage.findTransition(epochSeconds);
    }

    /** Initialize using the epochSeconds. */
    bool init(acetime_t epochSeconds) const {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      return init(ld);
    }

    /**
     * Initialize the zone rules cache, keyed by the "current" year.
     * Returns success status: true if successful, false if an error occurred.
     */
    bool init(const LocalDate& ld) const {
      int16_t year = ld.year();
      if (isFilled(year)) return true;
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("init(): %d\n", year);
      }

      mYear = year;
      mNumMatches = 0; // clear cache
      mTransitionStorage.init();

      if (year < mZoneInfoBroker.zoneContext()->startYear - 1
          || mZoneInfoBroker.zoneContext()->untilYear < year) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf("init(): Year %d out of valid range [%d, %d)\n",
              year,
              mZoneInfoBroker.zoneContext()->startYear,
              mZoneInfoBroker.zoneContext()->untilYear);
        }
        return false;
      }

      extended::YearMonthTuple startYm = {
        (int8_t) (year - LocalDate::kEpochYear - 1), 12 };
      extended::YearMonthTuple untilYm =  {
        (int8_t) (year - LocalDate::kEpochYear + 1), 2 };

      // Step 1 (See equivalent steps in
      // zone_specifier.ZoneSpecifier.init_for_year())
      mNumMatches = findMatches(mZoneInfoBroker, startYm, untilYm, mMatches,
          kMaxMatches);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 2
      findTransitions(mTransitionStorage, mMatches, mNumMatches);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 3
      Transition** begin = mTransitionStorage.getActivePoolBegin();
      Transition** end = mTransitionStorage.getActivePoolEnd();
      fixTransitionTimes(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 4
      generateStartUntilTimes(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      // Step 5
      calcAbbreviations(begin, end);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) { log(); }

      mIsFilled = true;
      return true;
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Find the ZoneEras which overlap [startYm, untilYm), ignoring day, time
     * and timeSuffix. The start and until fields of the ZoneEra are
     * truncated at the low and high end by startYm and untilYm, respectively.
     * Each matching ZoneEra is wrapped inside a MatchingEra object, placed in
     * the 'matches' array, and the number of matches is returned.
     */
    static uint8_t findMatches(const ZIB& zoneInfo,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm,
        MatchingEra* matches, uint8_t maxMatches) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findMatches()\n");
      }
      uint8_t iMatch = 0;
      ZEB prev; // anchor ZoneEra, representing the earliest untilTime
      for (uint8_t iEra = 0; iEra < zoneInfo.numEras(); iEra++) {
        const ZEB era = zoneInfo.era(iEra);
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
     * the day, time and timeSuffix fields of the era. The start date of the
     * current era is represented by the UNTIL fields of the previous era, so
     * the interval of the current era is [era.start=prev.UNTIL,
     * era.until=era.UNTIL). Overlap happens if (era.start < untilYm) and
     * (era.until > startYm). If prev.isNull(), then interpret prev as the
     * earliest ZoneEra.
     */
    static bool eraOverlapsInterval(
        const ZEB& prev,
        const ZEB& era,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm) {
      return (prev.isNull() || compareEraToYearMonth(
              prev, untilYm.yearTiny, untilYm.month) < 0)
          && compareEraToYearMonth(era, startYm.yearTiny, startYm.month) > 0;
    }

    /** Return (1, 0, -1) depending on how era compares to (yearTiny, month). */
    static int8_t compareEraToYearMonth(const ZEB& era,
        int8_t yearTiny, uint8_t month) {
      if (era.untilYearTiny() < yearTiny) return -1;
      if (era.untilYearTiny() > yearTiny) return 1;
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
    static MatchingEra createMatch(
        const ZEB& prev,
        const ZEB& era,
        const extended::YearMonthTuple& startYm,
        const extended::YearMonthTuple& untilYm) {
      // If prev.isNull(), set startDate to be earlier than all valid ZoneEra.
      extended::DateTuple startDate = prev.isNull()
          ? extended::DateTuple{
              LocalDate::kInvalidYearTiny, 1, 1, 0,
              internal::ZoneContext::kSuffixW
            }
          : extended::DateTuple{
              prev.untilYearTiny(), prev.untilMonth(), prev.untilDay(),
              (int16_t) prev.untilTimeMinutes(), prev.untilTimeSuffix()
            };
      extended::DateTuple lowerBound = {
        startYm.yearTiny, startYm.month, 1, 0,
        internal::ZoneContext::kSuffixW
      };
      if (startDate < lowerBound) {
        startDate = lowerBound;
      }

      extended::DateTuple untilDate = {
        era.untilYearTiny(), era.untilMonth(), era.untilDay(),
        (int16_t) era.untilTimeMinutes(), era.untilTimeSuffix()
      };
      extended::DateTuple upperBound = {
        untilYm.yearTiny, untilYm.month, 1, 0,
        internal::ZoneContext::kSuffixW
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
        TransitionStorage& transitionStorage,
        MatchingEra* matches,
        uint8_t numMatches) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findTransitions()\n");
      }
      for (uint8_t i = 0; i < numMatches; i++) {
        findTransitionsForMatch(transitionStorage, &matches[i]);
      }
    }

    /** Create the Transitions defined by the given match. */
    static void findTransitionsForMatch(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findTransitionsForMatch()\n");
      }
      const ZPB policy = match->era.zonePolicy();
      if (policy.isNull()) {
        findTransitionsFromSimpleMatch(transitionStorage, match);
      } else {
        findTransitionsFromNamedMatch(transitionStorage, match);
      }
    }

    static void findTransitionsFromSimpleMatch(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findTransitionsFromSimpleMatch()\n");
      }
      Transition* freeTransition = transitionStorage.getFreeAgent();
      createTransitionForYear(freeTransition, 0 /*not used*/,
          ZRB() /*rule*/, match);
      transitionStorage.addFreeAgentToActivePool();
    }

    static void findTransitionsFromNamedMatch(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findTransitionsFromNamedMatch()\n");
      }
      transitionStorage.resetCandidatePool();
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        match->log(); logging::printf("\n");
      }
      findCandidateTransitions(transitionStorage, match);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log(); logging::printf("\n");
      }
      fixTransitionTimes(
          transitionStorage.getCandidatePoolBegin(),
          transitionStorage.getCandidatePoolEnd());
      selectActiveTransitions(transitionStorage, match);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log(); logging::printf("\n");
      }

      transitionStorage.addActiveCandidatesToActivePool();
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        transitionStorage.log(); logging::printf("\n");
      }
    }

    static void findCandidateTransitions(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("findCandidateTransitions(): ");
        match->log();
        logging::printf("\n");
      }
      const ZPB policy = match->era.zonePolicy();
      uint8_t numRules = policy.numRules();
      int8_t startY = match->startDateTime.yearTiny;
      int8_t endY = match->untilDateTime.yearTiny;

      Transition** prior = transitionStorage.reservePrior();
      (*prior)->active = false; // indicates "no prior transition"
      for (uint8_t r = 0; r < numRules; r++) {
        const ZRB rule = policy.rule(r);

        // Add Transitions for interior years
        int8_t interiorYears[kMaxInteriorYears];
        uint8_t numYears = calcInteriorYears(interiorYears, kMaxInteriorYears,
            rule.fromYearTiny(), rule.toYearTiny(), startY, endY);
        for (uint8_t y = 0; y < numYears; y++) {
          int8_t year = interiorYears[y];
          Transition* t = transitionStorage.getFreeAgent();
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
            rule.fromYearTiny(), rule.toYearTiny(), startY, endY);
        if (priorYear != LocalDate::kInvalidYearTiny) {
          if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
            logging::printf(
              "findCandidateTransitions(): priorYear: %d\n", priorYear);
          }
          Transition* t = transitionStorage.getFreeAgent();
          createTransitionForYear(t, priorYear, rule, match);
          setAsPriorTransition(transitionStorage, t);
        }
      }

      // Add the reserved prior into the Candidate pool only if 'active' is
      // true, meaning that a prior was found.
      if ((*prior)->active) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "findCandidateTransitions(): adding prior to Candidate pool\n");
        }
        transitionStorage.addPriorToCandidatePool();
      }
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
     * Populate Transition 't' using the startTime from 'rule' (if it exists)
     * else from the start time of 'match'. Fills in 'offsetMinutes' and
     * 'deltaMinutes' as well. 'letterBuf' is also well-defined, either an empty
     * string, or filled with rule->letter with a NUL terminator.
     */
    static void createTransitionForYear(Transition* t, int8_t year,
        const ZRB& rule, const MatchingEra* match) {
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
        t->transitionTime = match->startDateTime;
        t->deltaMinutes = match->era.deltaMinutes();
      }
    }

    /**
     * Return the most recent prior year of the rule[from_year, to_year].
     * Return LocalDate::kInvalidYearTiny (-128) if the rule[from_year,
     * to_year] has no prior year to the match[start_year, end_year].
     */
    static int8_t getMostRecentPriorYear(int8_t fromYear, int8_t toYear,
        int8_t startYear, int8_t /*endYear*/) {
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

    static extended::DateTuple getTransitionTime(
        int8_t yearTiny, const ZRB& rule) {
      internal::MonthDay monthDay = internal::calcStartDayOfMonth(
          yearTiny + LocalDate::kEpochYear, rule.inMonth(), rule.onDayOfWeek(),
          rule.onDayOfMonth());
      return {yearTiny, monthDay.month, monthDay.day,
          (int16_t) rule.atTimeMinutes(), rule.atTimeSuffix()};
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
        const Transition* t, const MatchingEra* match) {
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
        TransitionStorage& transitionStorage,
        Transition* t) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("setAsPriorTransition()\n");
      }
      Transition* prior = transitionStorage.getPrior();
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
    static void fixTransitionTimes(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("fixTransitionTimes(): #transitions: %d;\n",
          (int) (end - begin));
      }

      // extend first Transition to -infinity
      Transition* prev = *begin;

      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* curr = *iter;
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf("fixTransitionTimes(): LOOP\n");
          curr->log();
          logging::printf("\n");
        }
        expandDateTuple(&curr->transitionTime,
            &curr->transitionTimeS, &curr->transitionTimeU,
            prev->offsetMinutes, prev->deltaMinutes);
        prev = curr;
      }
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("fixTransitionTimes(): END\n");
      }
    }

    /**
     * Convert the given 'tt', offsetMinutes, and deltaMinutes into the 'w',
     * 's' and 'u' versions of the DateTuple. The 'tt' may become a 'w' if it
     * was originally 's' or 'u'. On return, tt, tts and ttu are all modified.
     */
    static void expandDateTuple(extended::DateTuple* tt,
        extended::DateTuple* tts, extended::DateTuple* ttu,
        int16_t offsetMinutes, int16_t deltaMinutes) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("expandDateTuple()\n");
      }
      if (tt->suffix == internal::ZoneContext::kSuffixS) {
        *tts = *tt;
        *ttu = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes - offsetMinutes),
            internal::ZoneContext::kSuffixU};
        *tt = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes + deltaMinutes),
            internal::ZoneContext::kSuffixW};
      } else if (tt->suffix == internal::ZoneContext::kSuffixU) {
        *ttu = *tt;
        *tts = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes + offsetMinutes),
            internal::ZoneContext::kSuffixS};
        *tt = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes + (offsetMinutes + deltaMinutes)),
            internal::ZoneContext::kSuffixW};
      } else {
        // Explicit set the suffix to 'w' in case it was something else.
        tt->suffix = internal::ZoneContext::kSuffixW;
        *tts = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes - deltaMinutes),
            internal::ZoneContext::kSuffixS};
        *ttu = {tt->yearTiny, tt->month, tt->day,
            (int16_t) (tt->minutes - (deltaMinutes + offsetMinutes)),
            internal::ZoneContext::kSuffixU};
      }

      normalizeDateTuple(tt);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("expandDateTuple(): normalizeDateTuple(tt): ");
        tt->log();
        logging::printf("\n");
      }

      normalizeDateTuple(tts);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("expandDateTuple(): normalizeDateTuple(tts): ");
        tts->log();
        logging::printf("\n");
      }

      normalizeDateTuple(ttu);
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("expandDateTuple(): normalizeDateTuple(ttu): ");
        ttu->log();
        logging::printf("\n");
      }
    }

    /**
     * Normalize DateTuple::minutes if its magnitude is more than 24
     * hours.
     */
    static void normalizeDateTuple(extended::DateTuple* dt) {
      const int16_t kOneDayAsMinutes = 60 * 24;
      if (dt->minutes <= -kOneDayAsMinutes) {
        LocalDate ld = LocalDate::forTinyComponents(
            dt->yearTiny, dt->month, dt->day);
        local_date_mutation::decrementOneDay(ld);
        dt->yearTiny = ld.yearTiny();
        dt->month = ld.month();
        dt->day = ld.day();
        dt->minutes += kOneDayAsMinutes;
      } else if (kOneDayAsMinutes <= dt->minutes) {
        LocalDate ld = LocalDate::forTinyComponents(
            dt->yearTiny, dt->month, dt->day);
        local_date_mutation::incrementOneDay(ld);
        dt->yearTiny = ld.yearTiny();
        dt->month = ld.month();
        dt->day = ld.day();
        dt->minutes -= kOneDayAsMinutes;
      } else {
        // do nothing
      }
    }

    /**
     * Scan through the Candidate transitions, and mark the ones which are
     * active.
     */
    static void selectActiveTransitions(
        TransitionStorage& transitionStorage,
        const MatchingEra* match) {
      Transition** begin = transitionStorage.getCandidatePoolBegin();
      Transition** end = transitionStorage.getCandidatePoolEnd();
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("selectActiveTransitions(): #candidates: %d\n",
          (int) (end - begin));
      }
      Transition* prior = nullptr;
      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* transition = *iter;
        processActiveTransition(match, transition, &prior);
      }

      // If the latest prior transition is found, shift it to start at the
      // startDateTime of the current match.
      if (prior) {
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "selectActiveTransitions(): found latest prior\n");
        }
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
        const MatchingEra* match,
        Transition* transition,
        Transition** prior) {
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
     * comparing against the MatchingEra.startDateTime and
     * MatchingEra.untilDateTime, the version will be determined by the suffix
     * of those parameters.
     *
     * Returns:
     *     * -1 if less than match
     *     * 0 if equal to match_start
     *     * 1 if within match,
     *     * 2 if greater than match
     */
    static int8_t compareTransitionToMatch(
        const Transition* transition,
        const MatchingEra* match) {
      const extended::DateTuple* transitionTime;

      const extended::DateTuple& matchStart = match->startDateTime;
      if (matchStart.suffix == internal::ZoneContext::kSuffixS) {
        transitionTime = &transition->transitionTimeS;
      } else if (matchStart.suffix == internal::ZoneContext::kSuffixU) {
        transitionTime = &transition->transitionTimeU;
      } else { // assume 'w'
        transitionTime = &transition->transitionTime;
      }
      if (*transitionTime < matchStart) return -1;
      if (*transitionTime == matchStart) return 0;

      const extended::DateTuple& matchUntil = match->untilDateTime;
      if (matchUntil.suffix == internal::ZoneContext::kSuffixS) {
        transitionTime = &transition->transitionTimeS;
      } else if (matchUntil.suffix ==
          internal::ZoneContext::kSuffixU) {
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
    static void generateStartUntilTimes(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf(
          "generateStartUntilTimes(): #transitions: %d;\n",
          (int) (end - begin));
      }

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
        t->startDateTime = {tt.yearTiny, tt.month, tt.day, minutes,
            tt.suffix};
        normalizeDateTuple(&t->startDateTime);

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
        LocalDate ld = LocalDate::forTinyComponents(
            st.yearTiny, st.month, st.day);
        t->startEpochSeconds = ld.toEpochSeconds() + offsetSeconds;

        prev = t;
        isAfterFirst = true;
      }

      // The last Transition's until time is the until time of the MatchingEra.
      extended::DateTuple untilTime = prev->match->untilDateTime;
      extended::DateTuple untilTimeS; // needed only for expandDateTuple
      extended::DateTuple untilTimeU; // needed only for expandDateTuple
      expandDateTuple(&untilTime, &untilTimeS, &untilTimeU,
          prev->offsetMinutes, prev->deltaMinutes);
      prev->untilDateTime = untilTime;
    }

    /**
     * Calculate the time zone abbreviations for each Transition.
     */
    static void calcAbbreviations(Transition** begin, Transition** end) {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("calcAbbreviations(): #transitions: %d;\n",
          (int) (end - begin));
      }
      for (Transition** iter = begin; iter != end; ++iter) {
        Transition* const t = *iter;
        if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
          logging::printf(
            "calcAbbreviations(): format:%s, deltaMinutes:%d, letter:%s\n",
            t->format(), t->deltaMinutes, t->letter());
        }
        createAbbreviation(t->abbrev, internal::kAbbrevSize,
            t->format(), t->deltaMinutes, t->letter());
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
    static void createAbbreviation(char* dest, uint8_t destSize,
        const char* format, uint16_t deltaMinutes, const char* letterString) {
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

    const BF* mBrokerFactory;
    ZIB mZoneInfoBroker;

    mutable int16_t mYear = 0; // maybe create LocalDate::kInvalidYear?
    mutable bool mIsFilled = false;
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
