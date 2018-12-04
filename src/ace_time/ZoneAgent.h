#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <string.h> // strchr()
#include <stdint.h>
#include "common/ZonePolicy.h"
#include "common/ZoneInfo.h"
#include "UtcOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"

class ZoneAgentTest_init_primitives;
class ZoneAgentTest_init;
class ZoneAgentTest_createAbbreviation;
class ZoneAgentTest_calcStartDayOfMonth;
class ZoneAgentTest_calcRuleOffsetCode;

namespace ace_time {

/**
 * Data structure that captures the matching ZoneEntry and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 */
struct ZoneMatch {
  /**
   * Longest abbreviation seems to be 5 characters.
   * https://www.timeanddate.com/time/zones/
   */
  static const uint8_t kAbbrevSize = 5 + 1;

  /** The Zone entry that matched for the given year. NonNullable. */
  const common::ZoneEntry* entry;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-'. We do not support a RULES column that
   * contains a UTC offset. There are only 2 time zones that has this property
   * as of version 2018g: Europe/Istanbul and America/Argentina/San_Luis.
   */
  const common::ZoneRule* rule;

  /** The calculated transition time of the given rule. */
  uint32_t startEpochSeconds;

  /** The calculated effective UTC offsetCode at the start of transition. */
  int8_t offsetCode;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];
};

/**
 * Manages a given ZoneInfo. The ZoneRule and ZoneEntry records that match the
 * year of the given epochSeconds are cached internally for performance. The
 * expectation is that repeated calls to the various methods will have
 * epochSeconds which do not vary too greatly and will occur in the same year.
 *
 * Not thread-safe.
 */
class ZoneAgent {
  public:
    /**
     * Constructor.
     * @param zoneInfo pointer to a ZoneInfo. Can be nullptr which is
     * interpreted as UTC.
     */
    explicit ZoneAgent(const common::ZoneInfo* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    /** Copy constructor. */
    explicit ZoneAgent(const ZoneAgent& that):
      mZoneInfo(that.mZoneInfo),
      mIsFilled(false) {}

    /** Assignment operator. */
    ZoneAgent& operator=(const ZoneAgent& that) {
      mZoneInfo = that.mZoneInfo;
      mIsFilled = false;
      return *this;
    }

    /** Return the underlying ZoneInfo. */
    const common::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    /** Return if the time zone is observing DST. */
    bool isDst(uint32_t epochSeconds) {
      if (mZoneInfo == nullptr) return false;
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return zoneMatch->rule != nullptr && zoneMatch->rule->deltaCode != 0;
    }

    /** Return the current offset. */
    UtcOffset getUtcOffset(uint32_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return UtcOffset::forOffsetCode(zoneMatch->offsetCode);
    }

    /** Return the time zone abbreviation. */
    const char* getAbbrev(uint32_t epochSeconds) {
      if (mZoneInfo == nullptr) return "UTC";
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return zoneMatch->abbrev;
    }

  private:
    friend class ::ZoneAgentTest_init_primitives;
    friend class ::ZoneAgentTest_init;
    friend class ::ZoneAgentTest_createAbbreviation;
    friend class ::ZoneAgentTest_calcStartDayOfMonth;
    friend class ::ZoneAgentTest_calcRuleOffsetCode;

    static const uint8_t kMaxCacheEntries = 4;

    /** Return the ZoneMatch at the given epochSeconds. */
    const ZoneMatch* getZoneMatch(uint32_t epochSeconds) {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      init(ld);
      return findMatch(epochSeconds);
    }

    /**
     * Initialize the zone rules cache, keyed by the "current" year.
     *
     * If the UTC date is 12/31, the local date could be the next year. If we
     * assume that no DST transitions happen on 12/31, then we can pretend that
     * the current year is (UTC year + 1) and extract the various rules based
     * upon that year.
     */
    void init(const LocalDate& ld) {
      uint8_t year = ld.year();
      if (ld.month() == 12 && ld.day() == 31) {
        year++;
      }

      if (!isFilled(year)) {
        mYear = year;
        mNumMatches = 0; // clear cache

        addRulePriorToYear(year);
        addRulesForYear(year);
        calcTransitions();
        calcAbbreviations();
        mIsFilled = true;
      }
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(uint8_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Add the last matching rule just prior to the given year. This determines
     * the offset at the beginning of the current year.
     */
    void addRulePriorToYear(uint8_t year) {
      const common::ZoneEntry* const entry = findZoneEntryPriorTo(year);
      // TODO: Will never return nullptr if there is at least one Rule whose
      // toYearFull is ZoneRule::kMaxYear.

      const common::ZonePolicy* const zonePolicy = entry->zonePolicy;
      if (zonePolicy == nullptr) {
        mPreviousMatch = {
          entry,
          nullptr /*rule*/,
          0 /*epochSeconds*/,
          0 /*offsetCode*/,
          {0} /*abbrev*/
        };
        return;
      }

      // Find the latest rule for the matching Zone entry whose
      // ZoneRule::toYearFull < year. Assume that there are no more than 1 rule
      // per month.
      const common::ZoneRule* latest = nullptr;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const common::ZoneRule* const rule = &zonePolicy->rules[i];
        uint16_t yearFull = year + LocalDate::kEpochYear;
        // Check if rule is effective prior to the given year
        if (rule->fromYearFull < yearFull) {
          if ((latest == nullptr)
              || compareZoneRule(yearFull, rule, latest) > 0) {
            latest = rule;
          }
        }
      }
      mPreviousMatch = {
        entry,
        latest /*rule*/,
        0 /*epochSeconds*/,
        0 /*offsetCode*/,
        {0} /*abbrev*/
      };
    }

    /** Compare two ZoneRules which are valid prior to the given year. */
    static int8_t compareZoneRule(uint16_t yearFull,
        const common::ZoneRule* a, const common::ZoneRule* b) {
      uint16_t aYearFull = effectiveRuleYear(yearFull, a);
      uint16_t bYearFull = effectiveRuleYear(yearFull, b);
      if (aYearFull < bYearFull) return -1;
      if (aYearFull > bYearFull) return 1;
      if (a->inMonth < b->inMonth) return -1;
      if (a->inMonth > b->inMonth) return 1;
      return 0;
    }

    /** Return the largest effective year of the rule, prior to given year. */
    static int16_t effectiveRuleYear(uint16_t yearFull,
        const common::ZoneRule* rule) {
      if (rule->toYearFull < yearFull) return rule->toYearFull;
      if (rule->fromYearFull < yearFull) return yearFull - 1;
      return 0;
    }

    /** Add all matching rules from the current year. */
    void addRulesForYear(uint8_t year) {
      const common::ZoneEntry* const entry = findZoneEntry(year);

      const common::ZonePolicy* const zonePolicy = entry->zonePolicy;
      if (zonePolicy == nullptr) return;

      // Find all matching transition rules, and add them to the mMatches list,
      // in sorted order according to the ZoneRule::inMonth field.
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const common::ZoneRule* const rule = &zonePolicy->rules[i];
        uint16_t yearFull = year + LocalDate::kEpochYear;
        if ((rule->fromYearFull <= yearFull)
            && (yearFull <= rule->toYearFull)) {
          addRule(entry, rule);
        }
      }
    }

    /**
     * Add (entry, rule) to the cache, in sorted order according to the
     * 'ZoneRule::inMonth' field. This assumes that there are no more than one
     * transition per month.
     *
     * Essentially, this is doing an Insertion Sort of the ZoneMatch elements.
     * Even through it is O(N^2), for small number of ZoneMatch elements, this
     * is faster than than the O(N log(N)) algorithms such as Merge Sort, Heap
     * Sort, Quick Sort. The nice property of this Insertion Sort is that if
     * the ZoneInfoEntries are already sorted, then the loop terminates early
     * and the total sort time is O(N).
     */
    void addRule(const common::ZoneEntry* entry, const common::ZoneRule* rule)
        const {
      if (mNumMatches >= kMaxCacheEntries) return;

      // insert new element at the end of the list
      mMatches[mNumMatches] = {entry, rule, 0, 0, {0}};
      mNumMatches++;

      // perform an insertion sort
      for (uint8_t i = mNumMatches - 1; i > 0; i--) {
        ZoneMatch& left = mMatches[i - 1];
        ZoneMatch& right = mMatches[i];
        // assume only 1 rule per month
        if (left.rule->inMonth > right.rule->inMonth) {
          ZoneMatch tmp = left;
          left = right;
          right = tmp;
        }
      }
    }

    /**
     * Find the Zone entry which applies to the given year. The entry will have
     * an untilYear where (y < untilYear).
     *
     * Since the smallest year is 0 (i.e. 2000), this will return a Zone entry
     * with untilYear >= 1 (i.e. 2001). The largest supported year is 254 (i.e.
     * 2254) because no Zone entry will match (255 < untilYear) since untilYear
     * is stored as a uint8_t.
     */
    const common::ZoneEntry* findZoneEntry(uint8_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEntries; i++) {
        const common::ZoneEntry* entry = &mZoneInfo->entries[i];
        if (year < entry->untilYear) return entry;
      }
      return nullptr;
    }

    /**
     * Find the zone entry which applies to the year prior to the previous year.
     * The entry will have an untilYear where (y <= untilYear).
     */
    const common::ZoneEntry* findZoneEntryPriorTo(uint8_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEntries; i++) {
        const common::ZoneEntry* entry = &mZoneInfo->entries[i];
        if (year <= entry->untilYear) return entry;
      }
      return nullptr;
    }

    /** Calculate the transitional epochSeconds of each ZoneMatch rule. */
    void calcTransitions() {
      mPreviousMatch.startEpochSeconds = 0;
      mPreviousMatch.offsetCode = mPreviousMatch.entry->offsetCode
          + mPreviousMatch.rule->deltaCode;
      ZoneMatch* previousMatch = &mPreviousMatch;

      // Loop through ZoneMatch entries to calculate 2 things:
      // 1) ZoneMatch::startEpochSeconds
      // 2) ZoneMatch::offsetCode
      for (uint8_t i = 0; i < mNumMatches; i++) {
        ZoneMatch& match = mMatches[i];

        // Determine the start date of the rule.
        const uint8_t startDayOfMonth = calcStartDayOfMonth(
            mYear, match.rule->inMonth, match.rule->onDayOfWeek,
            match.rule->onDayOfMonth);

        // Determine the offset of the 'atHourModifier'. The 'w' modifier
        // requires the offset of the previous match.
        const int8_t ruleOffsetCode = calcRuleOffsetCode(
            previousMatch->offsetCode,
            match.entry->offsetCode,
            match.rule->atHourModifier);

        // startDateTime
        OffsetDateTime startDateTime = OffsetDateTime::forComponents(
            mYear, match.rule->inMonth, startDayOfMonth,
            match.rule->atHour, 0 /*minute*/, 0 /*second*/,
            UtcOffset::forOffsetCode(ruleOffsetCode));
        match.startEpochSeconds = startDateTime.toEpochSeconds();

        // Determine the effective offset code
        match.offsetCode = match.entry->offsetCode + match.rule->deltaCode;

        previousMatch = &match;
      }
    }

    /**
     * Calculate the actual dayOfMonth of the expresssion
     * (onDayOfWeek >= onDayOfMonth). The "last{dayOfWeek}" expression is
     * expressed by onDayOfMonth being 0. An exact match on dayOfMonth is
     * expressed by setting onDayOfWeek to 0.
     */
    static uint8_t calcStartDayOfMonth(uint8_t year, uint8_t month,
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
     * Determine the offset of the 'atHourModifier'. If 'w', then we
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
    void calcAbbreviations() {
      calcAbbreviation(&mPreviousMatch);
      for (uint8_t i = 0; i < mNumMatches; i++) {
        calcAbbreviation(&mMatches[i]);
      }
    }

    /** Calculate the time zone abbreviation of the current zoneMatch. */
    static void calcAbbreviation(ZoneMatch* zoneMatch) {
      createAbbreviation(
          zoneMatch->abbrev,
          ZoneMatch::kAbbrevSize,
          zoneMatch->entry->format,
          zoneMatch->rule != nullptr ? zoneMatch->rule->deltaCode : 0,
          zoneMatch->rule != nullptr ? zoneMatch->rule->letter : '\0');
    }

    /**
     * Create the time zone abbreviation in dest from the format string
     * (e.g. "P%T", "E%T"), the time zone deltaCode (!= 0 means DST), and the
     * replacement letter (e.g. 'S', 'D', or '-').
     *
     * @param dest destination string buffer
     * @param destSize size of buffer
     * @param format encoded abbreviation, '%' is a character substitution
     * @param deltaCode the offsetCode (0 for standard, != 0 for DST)
     * @param letter during standard or DST time ('S', 'D', '-' for no
     *        substitution, or '\0' when zoneMatch.rule == nullptr)
     */
    static void createAbbreviation(char* dest, uint8_t destSize,
        const char* format, uint8_t deltaCode, char letter) {
      if (deltaCode == 0 && letter == '\0') {
        strncpy(dest, format, destSize);
        dest[destSize - 1] = '\0';
        return;
      }

      if (strchr(format, '%') != nullptr) {
        copyAndReplace(dest, destSize, format, '%', letter);
      } else {
        char* slashPos = strchr(format, '/');
        if (slashPos != nullptr) {
          if (deltaCode == 0) {
            copyAndReplace(dest, destSize, format, '/', '\0');
          } else {
            memmove(dest, slashPos+1, strlen(slashPos+1) + 1);
          }
        } else {
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

    /** Search the cache and find closest ZoneMatch. */
    const ZoneMatch* findMatch(uint32_t epochSeconds) const {
      const ZoneMatch* closestMatch = &mPreviousMatch;
      for (uint8_t i = 0; i < mNumMatches; i++) {
        const ZoneMatch* m = &mMatches[i];
        if (m->startEpochSeconds <= epochSeconds) {
          closestMatch = m;
        }
      }
      return closestMatch;
    }

    const common::ZoneInfo* mZoneInfo;

    mutable uint8_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumMatches = 0;
    mutable ZoneMatch mMatches[kMaxCacheEntries];
    mutable ZoneMatch mPreviousMatch; // previous year's match
};

}

#endif
