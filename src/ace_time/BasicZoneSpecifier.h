#ifndef ACE_TIME_BASIC_ZONE_SPECIFIER_H
#define ACE_TIME_BASIC_ZONE_SPECIFIER_H

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

class BasicZoneSpecifierTest_init_primitives;
class BasicZoneSpecifierTest_init;
class BasicZoneSpecifierTest_createAbbreviation;
class BasicZoneSpecifierTest_calcStartDayOfMonth;
class BasicZoneSpecifierTest_calcRuleOffsetCode;

namespace ace_time {

namespace zonedb {

/**
 * Data structure that defines the start of a specific UTC offset as described
 * by the matching ZoneEra and its ZoneRule for a given year. If the ZoneEra
 * does not have a ZoneRule, then the Transition is defined by the start date
 * of the ZoneEra.
 */
struct Transition {
  /**
   * Longest abbreviation currently seems to be 5 characters
   * (https://www.timeanddate.com/time/zones/) but the TZ database spec says
   * that abbreviations are 3 to 6 characters
   * (https://data.iana.org/time-zones/theory.html#abbreviations), so use 6 as
   * the maximum.
   */
  static const uint8_t kAbbrevSize = 6 + 1;

  /** The ZoneEra that matched the given year. NonNullable. */
  const ZoneEra* era;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-'. We do not support a RULES column that
   * contains a UTC offset. There are only 2 time zones that has this property
   * as of version 2018g: Europe/Istanbul and America/Argentina/San_Luis.
   */
  const ZoneRule* rule;

  /** Year which applies to the ZoneEra or ZoneRule. */
  int8_t yearTiny;

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /**
   * The total effective UTC offsetCode at the start of transition, including
   * DST offset.
   */
  int8_t offsetCode;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];

  /** Used only for debugging. */
  void log() const {
    if (sizeof(acetime_t) == sizeof(int)) {
      logging::println("startEpochSeconds: %d", startEpochSeconds);
    } else {
      logging::println("startEpochSeconds: %ld", startEpochSeconds);
    }
    logging::println("offsetCode: %d", offsetCode);
    logging::println("abbrev: %s", abbrev);
    if (rule != nullptr) {
      logging::println("Rule.fromYear: %d", rule->fromYearTiny);
      logging::println("Rule.toYear: %d", rule->toYearTiny);
      logging::println("Rule.inMonth: %d", rule->inMonth);
      logging::println("Rule.onDayOfMonth: %d", rule->onDayOfMonth);
    }
  }
};

} // namespace zonedb

/**
 * Manages a given ZoneInfo. The ZoneRule and ZoneEra records that match the
 * year of the given epochSeconds are cached internally for performance. The
 * expectation is that repeated calls to the various methods will have
 * epochSeconds which do not vary too greatly and will occur in the same year.
 *
 * The Rule records are transition points which look like this:
 * @verbatim
 * Rule  NAME  FROM    TO  TYPE    IN     ON        AT      SAVE    LETTER/S
 * @endverbatim
 *
 * Each record is represented by zonedb::ZoneRule and the entire
 * collection is represented by zonedb::ZonePolicy.
 *
 * The Zone records define the region which follows a specific set of Rules
 * for certain time periods (given by UNTIL below):
 * @verbatim
 * Zone NAME              GMTOFF    RULES FORMAT  [UNTIL]
 * @endverbatim
 *
 * Each record is represented by zonedb::ZoneEra and the entire collection is
 * represented by zonedb::ZoneInfo.
 *
 * Limitations:
 *
 *  - supports Zone Infos whose untilTimeModifier is 'w' (not 's' or 'u')
 *  - supports Zone Infos whose RULES column refers to a named Zone Rule, not
 *    an offset (hh:mm)
 *  - supports Zone Infos whose UNTIL field contains only full year component,
 *    not month, day, or time
 *  - supports Zone Rules whose atTimeModifier can be any of ('w', 's', and 'u')
 *
 * Not thread-safe.
 */
class BasicZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Constructor.
     * @param zoneInfo pointer to a ZoneInfo. Must not be nullptr.
     */
    explicit BasicZoneSpecifier(const zonedb::ZoneInfo* zoneInfo):
        ZoneSpecifier(kTypeBasic),
        mZoneInfo(zoneInfo) {}

    /** Return the underlying ZoneInfo. */
    const zonedb::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    UtcOffset getUtcOffset(acetime_t epochSeconds) const override {
      const zonedb::Transition* transition = getTransition(epochSeconds);
      return UtcOffset::forOffsetCode(transition->offsetCode);
    }

    UtcOffset getDeltaOffset(acetime_t epochSeconds) const override {
      const zonedb::Transition* transition = getTransition(epochSeconds);
      if (transition->rule == nullptr) return UtcOffset();
      return UtcOffset::forOffsetCode(transition->rule->deltaCode);
    }

    const char* getAbbrev(acetime_t epochSeconds) const override {
      const zonedb::Transition* transition = getTransition(epochSeconds);
      return transition->abbrev;
    }

    void printTo(Print& printer) const override;

    /** Used only for debugging. */
    void log() const {
      if (!mIsFilled) {
        logging::println("*not initialized*");
        return;
      }
      logging::println("mYear: %d", mYear);
      logging::println("mNumTransitions: %d", mNumTransitions);
      logging::println("---- PrevTransition");
      mPrevTransition.log();
      for (int i = 0; i < mNumTransitions; i++) {
        logging::println("---- Transition: %d", i);
        mTransitions[i].log();
      }
    }

  private:
    friend class ::BasicZoneSpecifierTest_init_primitives;
    friend class ::BasicZoneSpecifierTest_init;
    friend class ::BasicZoneSpecifierTest_createAbbreviation;
    friend class ::BasicZoneSpecifierTest_calcStartDayOfMonth;
    friend class ::BasicZoneSpecifierTest_calcRuleOffsetCode;
    friend class ExtendedZoneSpecifier; // calcStartDayOfMonth()

    static const uint8_t kMaxCacheEntries = 4;

    /**
     * The smallest Transition.startEpochSeconds which represents -Infinity.
     * Can't use INT32_MIN because that is used internally to indicate
     * "invalid".
     */
    static const acetime_t kMinEpochSeconds = INT32_MIN + 1;

    // Disable copy constructor and assignment operator.
    BasicZoneSpecifier(const BasicZoneSpecifier&) = delete;
    BasicZoneSpecifier& operator=(const BasicZoneSpecifier&) = delete;

    bool equals(const ZoneSpecifier& other) const override {
      const auto& that = (const BasicZoneSpecifier&) other;
      return getZoneInfo() == that.getZoneInfo();
    }

    /** Return the Transition at the given epochSeconds. */
    const zonedb::Transition* getTransition(acetime_t epochSeconds) const {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      init(ld);
      return findMatch(epochSeconds);
    }

    /**
     * Initialize the zone rules cache, keyed by the "current" year.
     *
     * If the UTC date is 1/1, the local date could be the previous year.
     * Unfortunately, there are some countries that decided to make a time
     * change on 12/31, e.g. Dhaka). So, let's assume that there are no DST
     * transitions on 1/1, consider the "current year" to be the previous year,
     * extract the various rules based upon that year, and determine the DST
     * offset using the matching rules of the previous year.
     */
    void init(const LocalDate& ld) const {
      int16_t year = ld.year();
      if (ld.month() == 1 && ld.day() == 1) {
        year--;
      }

      if (!isFilled(year)) {
        mYear = year;
        mNumTransitions = 0; // clear cache

        addRulePriorToYear(year);
        addRulesForYear(year);
        calcTransitions();
        calcAbbreviations();
        mIsFilled = true;
      }
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Add the last matching rule just prior to the given year. This determines
     * the offset at the beginning of the current year.
     */
    void addRulePriorToYear(int16_t year) const {
      int8_t yearTiny = year - LocalDate::kEpochYear;
      int8_t priorYearTiny = yearTiny - 1;

      // Find the prior Era.
      const zonedb::ZoneEra* const era = findZoneEraPriorTo(year);

      // If the prior ZoneEra is a simple Era (no zone policy), then create a
      // Transition using a rule==nullptr. Otherwise, find the latest rule
      // within the ZoneEra.
      const zonedb::ZonePolicy* const zonePolicy = era->zonePolicy;
      const zonedb::ZoneRule* latest = nullptr;
      if (zonePolicy != nullptr) {
        // Find the latest rule for the matching ZoneEra whose
        // ZoneRule::toYearTiny < yearTiny. Assume that there are no more than
        // 1 rule per month.
        for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
          const zonedb::ZoneRule* const rule = &zonePolicy->rules[i];
          // Check if rule is effective prior to the given year
          if (rule->fromYearTiny < yearTiny) {
            if ((latest == nullptr)
                || compareZoneRule(year, rule, latest) > 0) {
              latest = rule;
            }
          }
        }
      }
      mPrevTransition = {
        era,
        latest /*rule*/,
        priorYearTiny /*yearTiny*/,
        0 /*epochSeconds*/,
        0 /*offsetCode*/,
        {0} /*abbrev*/
      };
    }

    /** Compare two ZoneRules which are valid prior to the given year. */
    static int8_t compareZoneRule(int16_t year,
        const zonedb::ZoneRule* a, const zonedb::ZoneRule* b) {
      int16_t aYear = effectiveRuleYear(year, a);
      int16_t bYear = effectiveRuleYear(year, b);
      if (aYear < bYear) return -1;
      if (aYear > bYear) return 1;
      if (a->inMonth < b->inMonth) return -1;
      if (a->inMonth > b->inMonth) return 1;
      return 0;
    }

    /**
     * Return the largest effective year of the rule, prior to given year.
     * Return 0 if rule is greater than the given year.
     */
    static int16_t effectiveRuleYear(int16_t year,
        const zonedb::ZoneRule* rule) {
      int8_t yearTiny = year - LocalDate::kEpochYear;
      if (rule->toYearTiny < yearTiny) {
        return rule->toYearTiny + LocalDate::kEpochYear;
      }
      if (rule->fromYearTiny < yearTiny) {
        return year - 1;
      }
      return 0;
    }

    /** Add all matching rules from the current year. */
    void addRulesForYear(int16_t year) const {
      const zonedb::ZoneEra* const era = findZoneEra(year);

      // If the ZonePolicy has no rules, then we need to add a Transition which
      // takes effect at the start time of the current year.
      const zonedb::ZonePolicy* const zonePolicy = era->zonePolicy;
      if (zonePolicy == nullptr) {
        addRule(year, era, nullptr);
        return;
      }

      // Find all matching transitions, and add them to mTransitions, in sorted
      // order according to the ZoneRule::inMonth field.
      int8_t yearTiny = year - LocalDate::kEpochYear;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const zonedb::ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYearTiny <= yearTiny) &&
            (yearTiny <= rule->toYearTiny)) {
          addRule(year, era, rule);
        }
      }
    }

    /**
     * Add (era, rule) to the cache, in sorted order according to the
     * 'ZoneRule::inMonth' field. This assumes that there are no more than one
     * transition per month.
     *
     * Essentially, this is doing an Insertion Sort of the Transition elements.
     * Even through it is O(N^2), for small number of Transition elements, this
     * is faster than than the O(N log(N)) algorithms such as Merge Sort, Heap
     * Sort, Quick Sort. The nice property of this Insertion Sort is that if
     * the ZoneInfoEntries are already sorted, then the loop terminates early
     * and the total sort time is O(N).
     */
    void addRule(int16_t year, const zonedb::ZoneEra* era,
          const zonedb::ZoneRule* rule) const {
      if (mNumTransitions >= kMaxCacheEntries) return;

      // insert new element at the end of the list
      int8_t yearTiny = year - LocalDate::kEpochYear;
      mTransitions[mNumTransitions] = {
        era,
        rule,
        yearTiny,
        0 /*epochSeconds*/,
        0 /*offsetCode*/,
        {0} /*abbrev*/
      };
      mNumTransitions++;

      // perform an insertion sort
      for (uint8_t i = mNumTransitions - 1; i > 0; i--) {
        zonedb::Transition& left = mTransitions[i - 1];
        zonedb::Transition& right = mTransitions[i];
        // assume only 1 rule per month
        if ((left.rule != nullptr && right.rule != nullptr &&
              left.rule->inMonth > right.rule->inMonth)
            || (left.rule != nullptr && right.rule == nullptr)) {
          zonedb::Transition tmp = left;
          left = right;
          right = tmp;
        }
      }
    }

    /**
     * Find the ZoneEra which applies to the given year. The era will
     * satisfy (year < ZoneEra.untilYearTiny + kEpochYear). Since the
     * largest untilYearTiny is 127, the largest supported 'year' is 2126.
     */
    const zonedb::ZoneEra* findZoneEra(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const zonedb::ZoneEra* era = &mZoneInfo->eras[i];
        if (year < era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      return nullptr;
    }

    /**
     * Find the most recent ZoneEra which was in effect just before the
     * beginning of the given year, in other words, just before {year}-01-01
     * 00:00:00. It will be first era just after the latest era whose untilYear
     * < year. Since the ZoneEras are in increasing order of untilYear, this is
     * the same as matching the first ZoneEra whose untilYear >= year.
     *
     * This should never return nullptr because the code generator for
     * zone_infos.cpp verified that the final ZoneEra contains an empty
     * untilYear, interpreted as 'max', and set to 127.
     */
    const zonedb::ZoneEra* findZoneEraPriorTo(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const zonedb::ZoneEra* era = &mZoneInfo->eras[i];
        if (year <= era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      return nullptr;
    }

    /** Calculate the epochSeconds and offsetCode of each Transition. */
    void calcTransitions() const {
      mPrevTransition.startEpochSeconds = kMinEpochSeconds;
      int8_t deltaCode = (mPrevTransition.rule == nullptr)
            ? 0 : mPrevTransition.rule->deltaCode;
      mPrevTransition.offsetCode = mPrevTransition.era->offsetCode + deltaCode;
      const zonedb::Transition* prevTransition = &mPrevTransition;

      // Loop through Transition items to calculate:
      // 1) Transition::startEpochSeconds
      // 2) Transition::offsetCode
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        zonedb::Transition& transition = mTransitions[i];
        const int16_t year = transition.yearTiny + LocalDate::kEpochYear;

        if (transition.rule == nullptr) {
          // TODO: Double-check this algorithm, something doesn't seem right.
          const int8_t offsetCode = calcRuleOffsetCode(
              prevTransition->offsetCode, transition.era->offsetCode, 'w');
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, 1, 1, 0, 0, 0,
              UtcOffset::forOffsetCode(offsetCode));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();
          transition.offsetCode = transition.era->offsetCode;
        } else {
          // Determine the start date of the rule.
          const uint8_t startDayOfMonth = calcStartDayOfMonth(
              year, transition.rule->inMonth, transition.rule->onDayOfWeek,
              transition.rule->onDayOfMonth);

          // Determine the offset of the 'atTimeModifier'. The 'w' modifier
          // requires the offset of the previous transition.
          const int8_t offsetCode = calcRuleOffsetCode(
              prevTransition->offsetCode,
              transition.era->offsetCode,
              transition.rule->atTimeModifier);

          // startDateTime
          const uint8_t atHour = transition.rule->atTimeCode / 4;
          const uint8_t atMinute = (transition.rule->atTimeCode % 4) * 15;
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, transition.rule->inMonth, startDayOfMonth,
              atHour, atMinute, 0 /*second*/,
              UtcOffset::forOffsetCode(offsetCode));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();

          // Determine the effective offset code
          transition.offsetCode =
              transition.era->offsetCode + transition.rule->deltaCode;
        }

        prevTransition = &transition;
      }
    }

    /**
     * Calculate the actual dayOfMonth of the expresssion
     * (onDayOfWeek >= onDayOfMonth). The "last{dayOfWeek}" expression is
     * expressed by onDayOfMonth being 0. An exact match on dayOfMonth is
     * expressed by setting onDayOfWeek to 0.
     */
    static uint8_t calcStartDayOfMonth(int16_t year, uint8_t month,
        uint8_t onDayOfWeek, uint8_t onDayOfMonth) {
      if (onDayOfWeek == 0) return onDayOfMonth;


      // Convert "last{Xxx}" to "last{Xxx}>={daysInMonth-6}".
      if (onDayOfMonth == 0) {
        onDayOfMonth = LocalDate::daysInMonth(year, month) - 6;
      }

      LocalDate limitDate = LocalDate::forComponents(
          year, month, onDayOfMonth);
      uint8_t dayOfWeekShift = (onDayOfWeek - limitDate.dayOfWeek() + 7) % 7;
      return onDayOfMonth + dayOfWeekShift;
    }

    /**
     * Determine the offset of the 'atTimeModifier'. If 'w', then we
     * must use the offset of the *previous* zone rule. If 's', use the
     * current base offset. If 'u', 'g', 'z', then use 0 offset.
     */
    static int8_t calcRuleOffsetCode(int8_t prevEffectiveOffsetCode,
        int8_t currentBaseOffsetCode, uint8_t modifier) {
      if (modifier == 'w') {
        return prevEffectiveOffsetCode;
      } else if (modifier == 's') {
        // TODO: Is this right, use the current matched base offset code?
        return currentBaseOffsetCode;
      } else { // 'u', 'g' or 'z'
        return 0;
      }
    }

    /** Determine the time zone abbreviations. */
    void calcAbbreviations() const {
      calcAbbreviation(&mPrevTransition);
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        calcAbbreviation(&mTransitions[i]);
      }
    }

    /** Calculate the time zone abbreviation of the current transition. */
    static void calcAbbreviation(zonedb::Transition* transition) {
      createAbbreviation(
          transition->abbrev,
          zonedb::Transition::kAbbrevSize,
          transition->era->format,
          transition->rule != nullptr ? transition->rule->deltaCode : 0,
          transition->rule != nullptr ? transition->rule->letter : '\0');
    }

    /**
     * Create the time zone abbreviation in dest from the format string
     * (e.g. "P%T", "E%T"), the time zone deltaCode (!= 0 means DST), and the
     * replacement letter (e.g. 'S', 'D', or '-').
     *
     * 1) If the RULES column (transition->rule) is empty '-', then FORMAT
     * cannot contain a '/' or a '%' because the ZoneEra specifies only a
     * single transition rule. (Verified in transformer.py). This condition is
     * indicated by (deltaCode == 0) and (letter == '\0').
     *
     * 2) If RULES column is not empty, then the FORMAT should contain either
     * a '/' or a '%'. The deltaCode will determine whether the time interval
     * is in DST.
     * This is verified by transformer.py to be true for all
     * Zones except Africa/Johannesburg which fails this for 1942-1944 where
     * the RULES contains a reference to named RULEs with DST transitions but
     * there is no '/' or '%' to distinguish between the 2. Technically, since
     * this occurs before year 2000, we don't absolutely need to suppor this,
     * but for robustness sake, we do.
     *
     * 2a) If the FORMAT contains a '%', use the LETTER to substitute the '%'.
     * If the 'letter' is '-', an empty character is substituted.
     *
     * 2b) If the FORMAT contains a '/', then pick the first (no DST) or the
     * second component (DST). The deltaCode selects between the two. The
     * letter is ignored, but cannot be '\0' because that would trigger Case
     * (1). The recommended value is '-'.
     *
     * @param dest destination string buffer
     * @param destSize size of buffer
     * @param format encoded abbreviation, '%' is a character substitution
     * @param deltaCode the offsetCode (0 for standard, != 0 for DST)
     * @param letter during standard or DST time ('S', 'D', '-' for no
     *        substitution, or '\0' when transition.rule == nullptr)
     */
    static void createAbbreviation(char* dest, uint8_t destSize,
        const char* format, uint8_t deltaCode, char letter) {
      // Check if RULES column empty.
      if (deltaCode == 0 && letter == '\0') {
        strncpy(dest, format, destSize);
        dest[destSize - 1] = '\0';
        return;
      }

      // Check if FORMAT contains a '%'.
      if (strchr(format, '%') != nullptr) {
        copyAndReplace(dest, destSize, format, '%', letter);
      } else {
        // Check if FORMAT contains a '/'.
        const char* slashPos = strchr(format, '/');
        if (slashPos != nullptr) {
          if (deltaCode == 0) {
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
          // Just copy the FORMAT disregarding the deltaCode and letter.
          strncpy(dest, format, destSize);
          dest[destSize - 1] = '\0';
        }
      }
    }

    /**
     * Copy at most dstSize characters from src to dst, while replacing all
     * occurance of oldChar with newChar. If newChar is '-', then replace with
     * nothing. The resulting dst string is always NUL terminated.
     */
    static void copyAndReplace(char* dst, uint8_t dstSize, const char* src,
        char oldChar, char newChar) {
      while (*src != '\0' && dstSize > 0) {
        if (*src == oldChar) {
          if (newChar == '-') {
            src++;
          } else {
            *dst = newChar;
            dst++;
            src++;
            dstSize--;
          }
        } else {
          *dst++ = *src++;
          dstSize--;
        }
      }

      if (dstSize == 0) {
        --dst;
      }
      *dst = '\0';
    }

    /** Search the cache and find closest Transition. */
    const zonedb::Transition* findMatch(acetime_t epochSeconds) const {
      const zonedb::Transition* closestMatch = &mPrevTransition;
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        const zonedb::Transition* m = &mTransitions[i];
        if (m->startEpochSeconds <= epochSeconds) {
          closestMatch = m;
        }
      }
      return closestMatch;
    }

    const zonedb::ZoneInfo* const mZoneInfo;

    mutable int16_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumTransitions = 0;
    mutable zonedb::Transition mTransitions[kMaxCacheEntries];
    mutable zonedb::Transition mPrevTransition; // previous year's transition
};

}

#endif
