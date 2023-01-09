/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_TRANSITION_H
#define ACE_TIME_EXTENDED_TRANSITION_H

#include <stdint.h> // uintptr_t
#include "common/logging.h"
#include "local_date_mutation.h"

class TransitionStorageTest_getFreeAgent;
class TransitionStorageTest_getFreeAgent2;
class TransitionStorageTest_addFreeAgentToActivePool;
class TransitionStorageTest_reservePrior;
class TransitionStorageTest_addPriorToCandidatePool;
class TransitionStorageTest_addFreeAgentToCandidatePool;
class TransitionStorageTest_setFreeAgentAsPriorIfValid;
class TransitionStorageTest_addActiveCandidatesToActivePool;
class TransitionStorageTest_findTransitionForDateTime;
class TransitionStorageTest_resetCandidatePool;

class Print;

#ifndef ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
#define ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG 0
#endif

namespace ace_time {
namespace extended {

//---------------------------------------------------------------------------

/**
 * A tuple that represents a date and time. Packed to 4-byte boundaries to
 * save space on 32-bit processors.
 */
struct DateTuple {
  DateTuple() = default;

  DateTuple(int16_t y, uint8_t mon, uint8_t d, int16_t min, uint8_t mod):
      year(y), month(mon), day(d), suffix(mod), minutes(min) {}

  int16_t year; // [-1,10000]
  uint8_t month; // [1,12]
  uint8_t day; // [1,31]
  uint8_t suffix; // kSuffixS, kSuffixW, kSuffixU
  int16_t minutes; // negative values allowed

  /** Used only for debugging. */
  void log() const {
    if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
      int hour = minutes / 60;
      int minute = minutes - hour * 60;
      char c = "wsu"[(suffix>>4)];
      logging::printf("%04d-%02u-%02uT%02d:%02d%c",
          year, month, day, hour, minute, c);
    }
  }
};

/** Determine if DateTuple a is less than DateTuple b, ignoring the suffix. */
inline bool operator<(const DateTuple& a, const DateTuple& b) {
  if (a.year < b.year) return true;
  if (a.year > b.year) return false;
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
  return a.year == b.year
      && a.month == b.month
      && a.day == b.day
      && a.minutes == b.minutes
      && a.suffix == b.suffix;
}

/** Normalize DateTuple::minutes if its magnitude is more than 24 hours. */
inline void normalizeDateTuple(DateTuple* dt) {
  const int16_t kOneDayAsMinutes = 60 * 24;
  if (dt->minutes <= -kOneDayAsMinutes) {
    LocalDate ld = LocalDate::forComponents(dt->year, dt->month, dt->day);
    local_date_mutation::decrementOneDay(ld);
    dt->year = ld.year();
    dt->month = ld.month();
    dt->day = ld.day();
    dt->minutes += kOneDayAsMinutes;
  } else if (kOneDayAsMinutes <= dt->minutes) {
    LocalDate ld = LocalDate::forComponents(dt->year, dt->month, dt->day);
    local_date_mutation::incrementOneDay(ld);
    dt->year = ld.year();
    dt->month = ld.month();
    dt->day = ld.day();
    dt->minutes -= kOneDayAsMinutes;
  } else {
    // do nothing
  }
}

/** Return the number of seconds in (a - b), ignoring suffix. */
inline acetime_t subtractDateTuple(const DateTuple& a, const DateTuple& b) {
  int32_t epochDaysA = LocalDate::forComponents(
      a.year, a.month, a.day).toEpochDays();
  int32_t epochSecondsA = epochDaysA * 86400 + a.minutes * 60;

  int32_t epochDaysB = LocalDate::forComponents(
      b.year, b.month, b.day).toEpochDays();
  int32_t epochSecondsB = epochDaysB * 86400 + b.minutes * 60;

  return epochSecondsA - epochSecondsB;
}

//---------------------------------------------------------------------------

/** A simple tuple to represent a year/month pair. */
struct YearMonthTuple {
  int16_t year;
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
  /**
   * The effective start time of the matching ZoneEra, which uses the
   * UTC offsets of the previous matching era.
   */
  DateTuple startDateTime;

  /** The effective until time of the matching ZoneEra. */
  DateTuple untilDateTime;

  /** The ZoneEra that matched the given year. NonNullable. */
  ZEB era;

  /** The previous MatchingEra, needed to interpret startDateTime.  */
  MatchingEraTemplate* prevMatch;

  /** The STD offset of the last Transition in this MatchingEra. */
  int16_t lastOffsetMinutes;

  /** The DST offset of the last Transition in this MatchingEra. */
  int16_t lastDeltaMinutes;

  void log() const {
    logging::printf("MatchingEra(");
    logging::printf("start="); startDateTime.log();
    logging::printf("; until="); untilDateTime.log();
    logging::printf("; era=%c", (era.isNull()) ? '-' : '*');
    logging::printf("; prevMatch=%c", (prevMatch) ? '*' : '-');
    logging::printf(")");
  }
};

/** Swap 2 parameters. */
template <typename T>
void swap(T& a, T& b) {
  T tmp = a;
  a = b;
  b = tmp;
}

/**
 * The result of comparing the transition time of a Transition to the
 * time interval of its corresponding MatchingEra.
 */
enum class MatchStatus : uint8_t {
  kFarPast, // 0
  kPrior, // 1
  kExactMatch, // 2
  kWithinMatch, // 3
  kFarFuture, // 4
};

inline bool isMatchStatusActive(MatchStatus status) {
  return status == MatchStatus::kExactMatch
      || status == MatchStatus::kWithinMatch
      || status == MatchStatus::kPrior;
}

//---------------------------------------------------------------------------

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
 * Some of the instance variables (e.g. 'isValidPrior', 'matchStatus',
 * 'transitionTime', 'transitionTimeS', 'transitionTimeU', 'letter()' and
 * 'format()') are transient parameters which are in the implementation of the
 * TransitionStorage::init() method.
 *
 * Other variables (e.g. 'startDateTime', 'startEpochSeconds', 'offsetMinutes',
 * 'deltaMinutes', 'abbrev', 'letterBuf') are essential parameters which are
 * required to find a matching Transition and construct the corresponding
 * ZonedDateTime.
 *
 * Ordering of fields are optimized along 4-byte boundaries to help 32-bit
 * processors without making the program size bigger for 8-bit processors.
 *
 * @tparam ZEB type of ZoneEraBroker
 * @tparam ZPB type of ZonePolicyBroker
 * @tparam ZRB type of ZoneRuleBroker
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

#if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
  /**
   * If the transition is shifted to the beginning of a MatchingEra, this is set
   * to the transitionTime for debugging.
   */
  DateTuple originalTransitionTime;
#endif

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

  union {
    /**
     * During findCandidateTransitions(), this flag indicates whether the
     * current transition is a valid "prior" transition that occurs before other
     * transitions. It is set by setFreeAgentAsPriorIfValid() if the transition
     * is a prior transition.
     */
    bool isValidPrior;

    /**
     * During processTransitionMatchStatus(), this flag indicates how the
     * transition falls within the time interval of the MatchingEra.
     */
    MatchStatus matchStatus;
  };

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

    // RULES point to a named rule, and LETTER is a single, printable character.
    // Return the letterBuf which contains a NUL-terminated string containing
    // the single character, as initialized in createTransitionForYear().
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
      logging::printf("start=%d", startEpochSeconds);
    } else {
      logging::printf("start=%ld", startEpochSeconds);
    }
    logging::printf("; status=%d", matchStatus);
    logging::printf("; UTC");
    logHourMinute(offsetMinutes);
    logHourMinute(deltaMinutes);
    logging::printf("; tt="); transitionTime.log();
    logging::printf("; tts="); transitionTimeS.log();
    logging::printf("; ttu="); transitionTimeU.log();
    if (rule.isNull()) {
      logging::printf("; rule=-");
    } else {
      logging::printf("; rule=");
      logging::printf("[%d,%d]", rule.fromYear(), rule.toYear());
    }
  }

  /** Print minutes as [+/-]hh:mm. */
  static void logHourMinute(int16_t minutes) {
    char sign;
    if (minutes < 0) {
      sign = '-';
      minutes = -minutes;
    } else {
      sign = '+';
    }
    uint8_t hour = minutes / 60;
    uint8_t minute = minutes - hour * 60;
    logging::printf("%c%02u:%02u", sign, (unsigned) hour, (unsigned) minute);
  }

#ifdef ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
  /** Print an iterable of Transitions from 'begin' to 'end'. */
  static void printTransitions(
      const char* prefix,
      const TransitionTemplate* const* begin,
      const TransitionTemplate* const* end) {
    for (const TransitionTemplate* const* iter = begin; iter != end; ++iter) {
      logging::printf(prefix);
      (*iter)->log();
      logging::printf("\n");
    }
  }
#endif
};

/**
 * Tuple of a matching Transition and its 'fold'. Used by
 * findTransitionForSeconds() which is guaranteed to return only a single
 * Transition if found. Usually `fold=0`. But if the epochSeconds maps to a
 * LocalDateTime which occurs a second time during a "fall back", then `fold` is
 * set to 1.
 */
template <typename ZEB, typename ZPB, typename ZRB>
struct TransitionForSecondsTemplate {
  const TransitionTemplate<ZEB, ZPB, ZRB>* transition;

  uint8_t fold; // 1 if corresponding datetime occurred the second time
};

/**
 * The result of the findTransitionForDateTime(const LocalDatetime& ldt) method
 * which can return 0, 1, or 2 matching Transitions depending on whether the
 * DateTime is an exact match, in the gap, or in the overlap.
 *
 * There are 5 possibilities:
 *
 *  * num=0, prev==NULL, curr=curr: datetime is far past
 *  * num=1, prev==prev, curr=prev: exact match to datetime
 *  * num=2, prev==prev, curr=curr: datetime in overlap
 *  * num=0, prev==prev, curr=curr: datetime in gap
 *  * num=0, prev==prev, curr=NULL: datetime is far future
 */
template <typename ZEB, typename ZPB, typename ZRB>
struct TransitionForDateTimeTemplate {
  const TransitionTemplate<ZEB, ZPB, ZRB>* prev;
  const TransitionTemplate<ZEB, ZPB, ZRB>* curr;
  uint8_t num; // number of exact matches
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
 * There are 4 pools indicated by the following half-open (inclusive to
 * exclusive) index ranges:
 *
 * 1) Active pool: [0, mIndexPrior)
 * 2) Prior pool: [mIndexPrior, mIndexCandidates), either 0 or 1 element
 * 3) Candidate pool: [mIndexCandidates, mIndexFree)
 * 4) Free agent pool: [mIndexFree, mAllocSize), 0 or 1 element
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

    /**
     * Template instantiation of TransitionForSecondsTemplate used by this
     * class. This should be treated as a private, it is exposed only for
     * testing purposes.
     */
    typedef TransitionForSecondsTemplate<ZEB, ZPB, ZRB> TransitionForSeconds;

    /**
     * Template instantiation of TransitionForDateTimeTemplate used by this
     * class. This should be treated as a private, it is exposed only for
     * testing purposes.
     */
    typedef TransitionForDateTimeTemplate<ZEB, ZPB, ZRB> TransitionForDateTime;

    /** Constructor. */
    TransitionStorageTemplate() {}

    /**
     * Initialize all pools to 0 size, usually when a new year is initialized.
     * The mAllocSize is not reset, so that we can determine the maximum
     * allocation size across multiple years. Call resetAllocSize() manually to
     * reset the mAllocSize.
     */
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

    /**
     * Empty the Candidate pool by resetting the various indexes.
     *
     * If every iteration of createTransitionsForMatch() finishes with
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
      if (mIndexFree < SIZE) {
        // Allocate a free transition.
        if (mIndexFree >= mAllocSize) {
          mAllocSize = mIndexFree + 1;
        }
        return mTransitions[mIndexFree];
      } else {
        // No more transition available in the buffer, so just return the last
        // one. This will probably cause a bug in the timezone calculations, but
        // I think this is better than triggering undefined behavior by running
        // off the end of the mTransitions buffer.
        return mTransitions[SIZE - 1];
      }
    }

    /**
     * Immediately add the free agent Transition at index mIndexFree to the
     * Active pool. Then increment mIndexFree to consume the free agent
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
     * Allocate a free Transition then add it to the Prior pool. This assumes
     * that the Prior pool and Candidate pool were both empty before calling
     * this method. Shift the Candidate pool and Free pool up by one. Return a
     * handle (pointer to pointer) to the Transition, so that the prior
     * Transition can be swapped with another Transition, while keeping the
     * handle valid.
     */
    Transition** reservePrior() {
      getFreeAgent(); // allocate a new Transition

      mIndexCandidates++;
      mIndexFree++;
      return &mTransitions[mIndexPrior];
    }

    /** Set the free agent transition as the most recent prior. */
    void setFreeAgentAsPriorIfValid() {
      Transition* ft = mTransitions[mIndexFree];
      Transition* prior = mTransitions[mIndexPrior];
      if ((prior->isValidPrior && prior->transitionTime < ft->transitionTime)
          || !prior->isValidPrior) {
        ft->isValidPrior = true;
        prior->isValidPrior = false;
        swap(mTransitions[mIndexPrior], mTransitions[mIndexFree]);
      }
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

      // This implementation makes pair-wise swaps to shift the current
      // Transition leftwards into its correctly sorted position. At first
      // glance, this seem inefficient compared to the alternative
      // implementation where we save the current Transition, then slide all the
      // elements to the left by one position rightwards. However,
      // MemoryBenchmark shows that this implementation is 46 bytes smaller on
      // an AVR processor.
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
     * pool. Every MatchingEra will have at least one Transition.
     *
     * @return the last Transition that was added
     */
    Transition* addActiveCandidatesToActivePool() {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf("addActiveCandidatesToActivePool()\n");
      }

      // Shift active candidates to the left into the Active pool.
      uint8_t iActive = mIndexPrior;
      uint8_t iCandidate = mIndexCandidates;
      for (; iCandidate < mIndexFree; iCandidate++) {
        if (isMatchStatusActive(mTransitions[iCandidate]->matchStatus)) {
          if (iActive != iCandidate) {
            // Must use swap(), because we are moving pointers instead of the
            // actual Transition objects.
            swap(mTransitions[iActive], mTransitions[iCandidate]);
          }
          ++iActive;
        }
      }

      mIndexPrior = iActive;
      mIndexCandidates = iActive;
      mIndexFree = iActive;

      return mTransitions[iActive - 1];
    }

    /**
     * Return the Transition matching the given epochSeconds. Return nullptr if
     * no matching Transition found. If a zone does not have any transition
     * according to TZ Database, the AceTimeTools/transformer.py script adds an
     * "anchor" transition at the "beginning of time" which happens to be the
     * year 1872 (because the year is stored as an int8_t). Therefore, this
     * method should never return a nullptr for a well-formed ZoneInfo file.
     */
    TransitionForSeconds findTransitionForSeconds(acetime_t epochSeconds)
        const {
      if (ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG) {
        logging::printf(
            "findTransitionForSeconds(): mIndexFree: %d\n", mIndexFree);
      }

      const Transition* prevMatch = nullptr;
      const Transition* match = nullptr;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        const Transition* candidate = mTransitions[i];
        if (candidate->startEpochSeconds > epochSeconds) break;
        prevMatch = match;
        match = candidate;
      }
      uint8_t fold = calculateFold(epochSeconds, match, prevMatch);
      return TransitionForSeconds{ match, fold };
    }

    static uint8_t calculateFold(
        acetime_t epochSeconds,
        const Transition* match,
        const Transition* prevMatch) {

      if (match == nullptr) return 0;
      if (prevMatch == nullptr) return 0;

      // Check if epochSeconds occurs during a "fall back" DST transition.
      acetime_t overlapSeconds = subtractDateTuple(
          prevMatch->untilDateTime, match->startDateTime);
      if (overlapSeconds <= 0) return 0;
      acetime_t secondsFromTransitionStart =
          epochSeconds - match->startEpochSeconds;
      if (secondsFromTransitionStart >= overlapSeconds) return 0;

      // EpochSeconds occurs within the "fall back" overlap.
      return 1;
    }

    /**
     * Return the candidate Transitions matching the given dateTime. The
     * search may return 0, 1 or 2 Transitions, depending on whether the
     * dateTime falls in a gap or overlap.
     */
    TransitionForDateTime findTransitionForDateTime(
        const LocalDateTime& ldt) const {
      // Convert LocalDateTime to DateTuple.
      DateTuple localDate{
          ldt.year(),
          ldt.month(),
          ldt.day(),
          (int16_t) (ldt.hour() * 60 + ldt.minute()),
          internal::ZoneContext::kSuffixW,
      };

      // Examine adjacent pairs of Transitions, looking for an exact match, gap,
      // or overlap.
      const Transition* prev = nullptr;
      const Transition* curr = nullptr;
      uint8_t num = 0;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        curr = mTransitions[i];

        const DateTuple& startDateTime = curr->startDateTime;
        const DateTuple& untilDateTime = curr->untilDateTime;
        bool isExactMatch = (startDateTime <= localDate)
            && (localDate < untilDateTime);

        if (isExactMatch) {
          // Check for a previous exact match to detect an overlap.
          if (num == 1) {
            num++;
            break;
          }

          // Loop again to detect an overlap.
          num = 1;
        } else if (startDateTime > localDate) {
          // Exit loop since no more candidate transition.
          break;
        }

        prev = curr;

        // Set the curr to nullptr so that if the loop runs off the end of the
        // list of Transitions, the curr is marked as nullptr.
        curr = nullptr;
      }

      // Check if the prev was an exact match, and set the curr to be identical.
      // avoid confusion.
      if (num == 1) {
        curr = prev;
      }

      // This should get optimized by RVO.
      return TransitionForDateTime{prev, curr, num};
    }

    /** Verify that the indexes are valid. Used only for debugging. */
    void log() const {
      logging::printf("TransitionStorage: ");
      logging::printf("SIZE=%d, mAllocSize=%d\n", SIZE, mAllocSize);
      int nActives = mIndexPrior;
      int nPrior = mIndexCandidates - mIndexPrior;
      int nCandidates = mIndexFree - mIndexCandidates;
      int nAllocFree = mAllocSize - mIndexFree;
      int nVirginFree = SIZE - mAllocSize;

      logging::printf("  Actives: %d\n", nActives);
      Transition::printTransitions(
          "    ", &mTransitions[0], &mTransitions[mIndexPrior]);

      logging::printf("  Prior: %d\n", nPrior);
      Transition::printTransitions(
          "    ", &mTransitions[mIndexPrior], &mTransitions[mIndexCandidates]);

      logging::printf("  Candidates: %d\n", nCandidates);
      Transition::printTransitions(
          "    ", &mTransitions[mIndexCandidates], &mTransitions[mIndexFree]);

      logging::printf("  Allocated Free: %d\n", nAllocFree);
      logging::printf("  Virgin Free: %d\n", nVirginFree);
    }

    /** Reset the current allocation size. For debugging. */
    void resetAllocSize() { mAllocSize = 0; }

    /**
     * Return the maximum number of transitions which was allocated. If this is
     * greater than SIZE, it indicates that the Transition mPool overflowed.
     * This method is intended for debugging.
     */
    uint8_t getAllocSize() const { return mAllocSize; }

  private:
    friend class ::TransitionStorageTest_getFreeAgent;
    friend class ::TransitionStorageTest_getFreeAgent2;
    friend class ::TransitionStorageTest_addFreeAgentToActivePool;
    friend class ::TransitionStorageTest_reservePrior;
    friend class ::TransitionStorageTest_addPriorToCandidatePool;
    friend class ::TransitionStorageTest_addFreeAgentToCandidatePool;
    friend class ::TransitionStorageTest_setFreeAgentAsPriorIfValid;
    friend class ::TransitionStorageTest_addActiveCandidatesToActivePool;
    friend class ::TransitionStorageTest_findTransitionForDateTime;
    friend class ::TransitionStorageTest_resetCandidatePool;

    /** Return the transition at position i.*/
    Transition* getTransition(uint8_t i) {
      return mTransitions[i];
    }

    Transition mPool[SIZE];
    Transition* mTransitions[SIZE];
    uint8_t mIndexPrior;
    uint8_t mIndexCandidates;
    uint8_t mIndexFree;

    /** Number of allocated transitions. */
    uint8_t mAllocSize = 0;
};

} // namespace extended
} // namespace ace_time

#endif
