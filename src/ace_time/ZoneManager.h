#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <string.h> // strchr()
#include <stdint.h>
#include "ZoneOffset.h"
#include "ZoneInfo.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"

class ZoneManagerTest_init_primitives;
class ZoneManagerTest_init;
class ZoneManagerTest_createAbbreviation;
class ZoneManagerTest_calcStartDayOfMonth;
class ZoneManagerTest_calcRuleOffsetCode;

namespace ace_time {

/**
 * Data structure that captures the matching ZoneInfoEntry for a given year.
 * Can be cached based on the year.
 */
struct ZoneMatch {
  // Longest abbreviation seems to be 5 characters.
  // https://www.timeanddate.com/time/zones/
  static const uint8_t kAbbrevSize = 5 + 1;

  const ZoneInfoEntry* entry; // NonNull
  const ZoneRule* rule; // Nullable
  uint32_t startEpochSeconds; // transition time of given rule
  int8_t offsetCode; // effective offsetCode at the start of zone period
  char abbrev[kAbbrevSize]; // time zone abbreviation
};

/**
 * Manages a given ZoneInfo.
 */
class ZoneManager {
  public:
    /** Constructor */
    ZoneManager(const ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    /** Assignment operator. */
    ZoneManager& operator=(const ZoneManager& that) {
      mZoneInfo = that.mZoneInfo;
      mIsFilled = false;
      return *this;
    }

    /** Return the underlying ZoneInfo. */
    const ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    /** Return if the time zone is observing DST. */
    bool isDst(uint32_t epochSeconds) {
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return zoneMatch->rule != nullptr && zoneMatch->rule->deltaCode != 0;
    }

    /** Return the current offset. */
    ZoneOffset getZoneOffset(uint32_t epochSeconds) {
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return ZoneOffset::forOffsetCode(zoneMatch->offsetCode);
    }

    /** Return the time zone abbreviation. */
    const char* getAbbrev(uint32_t epochSeconds) {
      const ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return zoneMatch->abbrev;
    }

  private:
    friend class ::ZoneManagerTest_init_primitives;
    friend class ::ZoneManagerTest_init;
    friend class ::ZoneManagerTest_createAbbreviation;
    friend class ::ZoneManagerTest_calcStartDayOfMonth;
    friend class ::ZoneManagerTest_calcRuleOffsetCode;

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
        mNumMatches = 0;
        addLastYear();
        addCurrentYear();
        calcTransitions();
        calcAbbreviations();
        mIsFilled = true;
      }
    }

    bool isFilled(uint8_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Add the last matching rule from last year, to determine the offset
     * at the beginning of the current year.
     *  TODO: special handing for year 2000 (i.e. year == 0)
     */
    void addLastYear() {
      uint8_t lastYear = mYear - 1;
      const ZoneInfoEntry* const entry = findEntry(lastYear);
      // TODO: check for (entry == nullptr).

      // Some countries don't have a ZonePolicy, e.g. South Africa
      const ZonePolicy* const zonePolicy = entry->zonePolicy;
      if (zonePolicy == nullptr) {
        mPreviousMatch = {entry, nullptr, 0, 0, {0}};
        return;
      }

      // Find the latest rule for last year. Assume that there are no more than
      // 1 rule per month.
      const ZoneRule* latest = nullptr;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYear <= lastYear) && (lastYear <= rule->toYear)) {
          if ((latest == nullptr) || (rule->inMonth > latest->inMonth)) {
            latest = rule;
          }
        }
      }
      mPreviousMatch = {entry, latest, 0, 0, {0}};
    }

    /** Add all matching rules from the current year. */
    void addCurrentYear() {
      const ZoneInfoEntry* const entry = findEntry(mYear);

      const ZonePolicy* const zonePolicy = entry->zonePolicy;
      if (zonePolicy == nullptr) return;

      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYear <= mYear) && (mYear <= rule->toYear)) {
          addRule(entry, rule);
        }
      }
    }

    /**
     * Add (entry, rule) to the cache, in sorted order. Essentially, this is
     * doing an Insertion Sort of the ZoneMatch elements. Even through it is
     * O(N^2), for small number of ZoneMatch elements, this is faster than than
     * the O(N log(N)) algorithms such as Merge Sort, Heap Sort, Quick Sort, or
     * Shell Sort. The nice property of this Insertion Sort is that if the
     * ZoneInfoEntries are already sorted, then the total sort time is O(N).
     */
    void addRule(const ZoneInfoEntry* entry, const ZoneRule* rule) const {
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

    /** Find the matching entry for year. */
    const ZoneInfoEntry* findEntry(uint8_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEntries; i++) {
        const ZoneInfoEntry* entry = &mZoneInfo->entries[i];
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

        // Determine the offset of the 'atHourModifier'.
        const int8_t ruleOffsetCode = calcRuleOffsetCode(
            previousMatch->offsetCode,
            match.entry->offsetCode,
            match.rule->atHourModifier);

        // startDateTime
        OffsetDateTime startDateTime = OffsetDateTime::forComponents(
            mYear, match.rule->inMonth, startDayOfMonth,
            match.rule->atHour, 0 /*minute*/, 0 /*second*/,
            ZoneOffset::forOffsetCode(ruleOffsetCode));
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

    const ZoneInfo* mZoneInfo;

    mutable uint8_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumMatches = 0;
    mutable ZoneMatch mMatches[kMaxCacheEntries];
    mutable ZoneMatch mPreviousMatch; // previous year's match
};

}

#endif
