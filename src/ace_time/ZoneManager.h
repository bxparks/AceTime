#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <stdint.h>
#include "ZoneOffset.h"
#include "ZoneInfo.h"

class ZoneManagerTest_init_2001;
class ZoneManagerTest_init_2018;
class ZoneManagerTest_createAbbreviation;

namespace ace_time {

/**
 * Data structure that captures the matching ZoneInfoEntry for a given year.
 * Can be cached based on the year.
 */
struct ZoneMatch {
  // Longest abbreviation seems to be 5 characters.
  // https://www.timeanddate.com/time/zones/
  static const uint8_t kAbbrevSize = 5 + 1;

  const ZoneInfoEntry* entry;
  const ZoneRule* rule; // can be null
  uint32_t startEpochSeconds; // transition time of given rule
  int8_t offsetCode; // effective offsetCode at the start of zone period
  char abbrev[kAbbrevSize]; // time zone abbreviation
};

/**
 * Manages a given ZoneInfo.
 */
class ZoneManager {
  public:
    ZoneManager(const ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    /** Return the ZoneMatch at the given epochSeconds. */
    const ZoneMatch* getZoneMatch(uint32_t epochSeconds) {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      init(ld);
      return findMatch(epochSeconds);
    }

  private:
    friend class ::ZoneManagerTest_init_2001;
    friend class ::ZoneManagerTest_init_2018;
    friend class ::ZoneManagerTest_createAbbreviation;

    static const uint8_t kMaxCacheEntries = 5;

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
      return (year == mYear) && mIsFilled;
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

      for (uint8_t i = 0; i < mNumMatches; i++) {
        ZoneMatch& match = mMatches[i];

        // Determine the start date of the rule.
        LocalDate limitDate = LocalDate::forComponents(
            mYear, match.rule->inMonth, match.rule->onDayOfMonth);
        uint8_t dayOfWeekShift =
            (match.rule->onDayOfWeek - limitDate.dayOfWeek() + 7) % 7;
        uint8_t startDayOfMonth = match.rule->onDayOfMonth + dayOfWeekShift;

        // Determine the effective offset code
        match.offsetCode = match.entry->offsetCode + match.rule->deltaCode;

        // Determine the offset of the 'atHourModifier'. If 'w', then we
        // must use the offset of the *previous* zone rule.
        int8_t ruleOffsetCode;
        if (match.rule->atHourModifier == 'w') {
          ruleOffsetCode = previousMatch->entry->offsetCode
              + previousMatch->rule->deltaCode;
        } else if (match.rule->atHourModifier == 's') {
          ruleOffsetCode = previousMatch->entry->offsetCode;
        } else { /* e.g. 'u', 'g' or 'z' */
          ruleOffsetCode = 0;
        }

        // startDateTime
        OffsetDateTime startDateTime = OffsetDateTime::forComponents(
            mYear, match.rule->inMonth, startDayOfMonth,
            match.rule->atHour, 0, 0,
            ZoneOffset::forOffsetCode(ruleOffsetCode));
        match.startEpochSeconds = startDateTime.toEpochSeconds();

        previousMatch = &match;
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
      createAbbreviation(zoneMatch->abbrev,
          ZoneMatch::kAbbrevSize,
          zoneMatch->entry->format,
          zoneMatch->rule->deltaCode,
          zoneMatch->rule->letter);
    }

    /**
     * Create the time zone abbreviation in dest from the format string
     * (e.g. "P%T", "E%T"), the time zone deltaCode (!= 0 means DST), and the
     * replacement letter (e.g. 'S', 'D', or '-').
     */
    static void createAbbreviation(char* dest, uint8_t destSize,
        const char* format, uint8_t deltaCode, char letter) {
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

    const ZoneInfo* const mZoneInfo;

    mutable uint8_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumMatches = 0;
    mutable ZoneMatch mMatches[kMaxCacheEntries];
    mutable ZoneMatch mPreviousMatch; // previous year's match
};

}

#endif
