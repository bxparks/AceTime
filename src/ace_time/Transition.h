/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_TRANSITION_H
#define ACE_TIME_EXTENDED_TRANSITION_H

#include <stdint.h> // uint8_t
#include "common/logging.h"
#include "local_date_mutation.h"
#include "DateTuple.h"

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

inline bool isCompareStatusActive(CompareStatus status) {
  return status == CompareStatus::kExactMatch
      || status == CompareStatus::kWithinMatch
      || status == CompareStatus::kPrior;
}

/**
 * Data structure that captures the matching ZoneEra and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 *
 * @tparam D container type of ZoneInfo database
 */
template<typename D>
struct MatchingEraTemplate {
  /**
   * The effective start time of the matching ZoneEra, which uses the
   * UTC offsets of the previous matching era.
   */
  DateTuple startDateTime;

  /** The effective until time of the matching ZoneEra. */
  DateTuple untilDateTime;

  /** The ZoneEra that matched the given year. NonNullable. */
  typename D::ZoneEraBroker era;

  /** The previous MatchingEra, needed to interpret startDateTime.  */
  MatchingEraTemplate* prevMatch;

  /** The STD offset of the last Transition in this MatchingEra. */
  int32_t lastOffsetSeconds;

  /** The DST offset of the last Transition in this MatchingEra. */
  int32_t lastDeltaSeconds;

  void log() const {
    logging::printf("MatchingEra(");
    logging::printf("start="); startDateTime.log();
    logging::printf("; until="); untilDateTime.log();
    logging::printf("; era=%c", (era.isNull()) ? '-' : '*');
    logging::printf("; prevMatch=%c", (prevMatch) ? '*' : '-');
    logging::printf(")");
  }
};

//---------------------------------------------------------------------------

/**
 * Represents an interval of time where the time zone obeyed a certain UTC
 * offset and DST delta. The start of the interval is given by 'transitionTime'
 * which comes from the TZ Database file. The actual start and until time of
 * the interval (in the local time zone) is given by 'startDateTime' and
 * 'untilDateTime'.
 *
 * There are 2 types of Transition instances:
 *  1) Simple, indicated by 'rule' == nullptr. The base UTC offsetSeconds is
 *  given by match->offsetSeconds. The additional DST delta is given by
 *  match->deltaSeconds.
 *  2) Named, indicated by 'rule' != nullptr. The base UTC offsetSeconds is
 *  given by match->offsetSeconds. The additional DST delta is given by
 *  rule->deltaSeconds.
 *
 * Some of the instance variables (e.g. 'isValidPrior', 'compareStatus',
 * 'transitionTime', 'transitionTimeS', 'transitionTimeU', 'letter()' and
 * 'format()') are transient parameters which are in the implementation of the
 * TransitionStorage::init() method.
 *
 * Other variables (e.g. 'startDateTime', 'startEpochSeconds', 'offsetSeconds',
 * 'deltaSeconds', 'abbrev', 'letterBuf') are essential parameters which are
 * required to find a matching Transition and construct the corresponding
 * ZonedDateTime.
 *
 * Ordering of fields are optimized along 4-byte boundaries to help 32-bit
 * processors without making the program size bigger for 8-bit processors.
 *
 * @tparam D container type of ZoneInfo database
 */
template <typename D>
struct TransitionTemplate {

  /** The match which generated this Transition. */
  const MatchingEraTemplate<D>* match;

#if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-', indicating that the MatchingEra was
   * a "simple" ZoneEra. This not required for actual calculation, but it is
   * useful to have a reference to it for debugging purposes.
   */
  typename D::ZoneRuleBroker rule;
#endif

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

  /** The standard time offset seconds, not the total offset. */
  int32_t offsetSeconds;

  /** The DST delta seconds. */
  int32_t deltaSeconds;

  /**
   * The calculated effective time zone abbreviation, e.g. "PST" or "PDT".
   * Initially this string buffer temporarily holds the `ZoneRule.letter()`
   * string, until `createAbbreviation()` consumes the `letter` and creates the
   * actual abbreviation.
   */
  char abbrev[kAbbrevSize];

  union {
    /**
     * During findCandidateTransitions(), this flag indicates whether the
     * current transition is a valid "prior" transition that occurs before other
     * transitions. It is set by setFreeAgentAsPriorIfValid() if the transition
     * is a prior transition.
     */
    bool isValidPrior;

    /**
     * During processTransitionCompareStatus(), this flag indicates how the
     * transition falls within the time interval of the MatchingEra.
     */
    CompareStatus compareStatus;
  };

  const char* format() const {
    return match->era.format();
  }

  /** Used only for debugging. */
  void log() const {
    logging::printf("Transition(");
    if (sizeof(acetime_t) <= sizeof(int)) {
      logging::printf("start=%d", startEpochSeconds);
    } else {
      logging::printf("start=%ld", startEpochSeconds);
    }
    logging::printf("; status=%d", compareStatus);
    logging::printf("; UTC");
    logHourMinuteSecond(offsetSeconds);
    logHourMinuteSecond(deltaSeconds);
    logging::printf("; tt="); transitionTime.log();
    logging::printf("; tts="); transitionTimeS.log();
    logging::printf("; ttu="); transitionTimeU.log();
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    if (rule.isNull()) {
      logging::printf("; rule=-");
    } else {
      logging::printf("; rule=");
      logging::printf("[%d,%d]", rule.fromYear(), rule.toYear());
    }
  #endif
  }

  /** Print seconds as [+/-]hh:mm[:ss]. */
  static void logHourMinuteSecond(int32_t seconds) {
    char sign;
    if (seconds < 0) {
      sign = '-';
      seconds = -seconds;
    } else {
      sign = '+';
    }
    uint16_t minutes = seconds / 60;
    uint8_t second = seconds - minutes * int32_t(60);
    uint8_t hour = minutes / 60;
    uint8_t minute = minutes - hour * 60;
    if (second == 0) {
      logging::printf("%c%02u:%02u", sign, (unsigned) hour, (unsigned) minute);
    } else {
      logging::printf("%c%02u:%02u:%02u",
          sign, (unsigned) hour, (unsigned) minute, (unsigned) second);
    }
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
template <typename D>
struct TransitionForSecondsTemplate {
  /** The matching transition, or null if not found. */
  const TransitionTemplate<D>* curr;

  /** 1 if corresponding datetime occurred the second time */
  uint8_t fold;

  /**
   * Number of occurrences of the resulting LocalDateTime: 0, 1, or 2.
   * This is needed because a fold=0 can mean that the LocalDateTime occurs
   * exactly once, or that the first of two occurrences of LocalDateTime was
   * selected by the epochSeconds.
   */
  uint8_t num;
};

/**
 * The result of the findTransitionForDateTime(const LocalDatetime& ldt) method
 * which can return 0, 1, or 2 matching Transitions depending on whether the
 * DateTime is an exact match, in the gap, or in the overlap.
 *
 * There are 5 possibilities:
 *
 *  * num=0, prev==NULL, curr=curr: datetime is far past (should not happen)
 *  * num=1, prev==prev, curr=prev: exact match to datetime
 *  * num=2, prev==prev, curr=curr: datetime in overlap
 *  * num=0, prev==prev, curr=curr: datetime in gap
 *  * num=0, prev==prev, curr=NULL: datetime is far future (should not happen)
 */
template <typename D>
struct TransitionForDateTimeTemplate {
  /** The previous transition. */

  const TransitionTemplate<D>* prev;
  /** The matching transition, or null if not found or in gap. */

  const TransitionTemplate<D>* curr;

  /** Number of matches: 0, 1, 2 */
  uint8_t num;
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
 * @tparam D container type of ZoneInfo database
 */
template<uint8_t SIZE, typename D>
class TransitionStorageTemplate {
  public:
    /**
     * Template instantiation of TransitionTemplate used by this class. This
     * should be treated as a private, it is exposed only for testing purposes.
     */
    typedef TransitionTemplate<D> Transition;

    /**
     * Template instantiation of TransitionForSecondsTemplate used by this
     * class. This should be treated as a private, it is exposed only for
     * testing purposes.
     */
    typedef TransitionForSecondsTemplate<D> TransitionForSeconds;

    /**
     * Template instantiation of TransitionForDateTimeTemplate used by this
     * class. This should be treated as a private, it is exposed only for
     * testing purposes.
     */
    typedef TransitionForDateTimeTemplate<D> TransitionForDateTime;

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
        if (isCompareStatusActive(mTransitions[iCandidate]->compareStatus)) {
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

      const Transition* prev = nullptr;
      const Transition* curr = nullptr;
      const Transition* next = nullptr;
      for (uint8_t i = 0; i < mIndexFree; i++) {
        next = mTransitions[i];
        if (next->startEpochSeconds > epochSeconds) break;
        prev = curr;
        curr = next;
        next = nullptr;
      }

      uint8_t fold;
      uint8_t num;
      calcFoldAndOverlap(&fold, &num, prev, curr, next, epochSeconds);
      //fprintf(stderr, "prev=%p;curr=%p;next=%p;fold=%d;num=%d\n",
      //  prev, curr, next, fold, num);
      return TransitionForSeconds{curr, fold, num};
    }

    /**
     * Calculate the fold and num parameters of TransitionForSecond.
     *
     * The `num` parameter is the number of transitions which can shadow a given
     * epochSeconds. It is 0 if `curr` is NULL, which means that epochSeconds
     * cannot be mapped to any transition. It is 1 if the epochSeconds in the
     * `curr` transition is unique and does not overlap with the `prev` or
     * `next` transition. It is 2 if the epochSeconds in the `curr` transition
     * maps to a LocalDateTime that overlaps with either the `prev` or `next`
     * transition. (In theory, I suppose it could overlap with both, but it is
     * improbable that any timezone in the TZDB will ever let that happen.)
     *
     * The `fold` parameter specifies whether the `curr` transition is the first
     * instance (0) or the second instance (1). It is relevant only if `num` is
     * 2. If `num` is 0 or 1, `fold` will always be 0. If `num` is 2, then
     * `fold` indicates whether `curr` is the earlier (0) or later (1)
     * transition of the overlap. This `fold` parameter will be copied into the
     * corresponding `fold` parameter in LocalDateTime.
     */
    static void calcFoldAndOverlap(
        uint8_t* fold,
        uint8_t* num,
        const Transition* prev,
        const Transition* curr,
        const Transition* next,
        acetime_t epochSeconds) {

      if (curr == nullptr) {
        *fold = 0;
        *num = 0;
        return;
      }

      // Check if within forward overlap shadow from prev
      bool isOverlap;
      if (prev == nullptr) {
        isOverlap = false;
      } else {
        // Extract the shift from prev transition. Can be 0 in some cases where
        // the zone changed from DST of one zone to the STD into another zone,
        // causing the overall UTC offset to remain unchanged.
        acetime_t shiftSeconds = subtractDateTuple(
            curr->startDateTime, prev->untilDateTime);
        if (shiftSeconds >= 0) {
          // spring forward, or unchanged
          isOverlap = false;
        } else {
          // Check if within the forward overlap shadow from prev
          isOverlap = epochSeconds - curr->startEpochSeconds < -shiftSeconds;
        }
      }
      if (isOverlap) {
        *fold = 1; // epochSeconds selects the second match
        *num = 2;
        return;
      }

      // Check if within backward overlap shawdow from next
      if (next == nullptr) {
        isOverlap = false;
      } else {
        // Extract the shift to next transition. Can be 0 in some cases where
        // the zone changed from DST of one zone to the STD into another zone,
        // causing the overall UTC offset to remain unchanged.
        acetime_t shiftSeconds = subtractDateTuple(
            next->startDateTime, curr->untilDateTime);
        if (shiftSeconds >= 0) {
          // spring forward, or unchanged
          isOverlap = false;
        } else {
          // Check if within the backward overlap shadow from next
          isOverlap = next->startEpochSeconds - epochSeconds <= -shiftSeconds;
        }
      }
      if (isOverlap) {
        *fold = 0; // epochSeconds selects the first match
        *num = 2;
        return;
      }

      // Normal single match, no overlap.
      *fold = 0;
      *num = 1;
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
          ((ldt.hour() * int32_t(60) + ldt.minute()) * 60 + ldt.second()),
          extended::Info::ZoneContext::kSuffixW,
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
