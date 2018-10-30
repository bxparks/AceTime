#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <stdint.h>
#include "ZoneOffset.h"
#include "ZoneInfo.h"

class ZoneManagerTest_init_2001;
class ZoneManagerTest_init_2018;

namespace ace_time {

/**
 * Data structure that captures the matching ZoneInfoEntry for a given year.
 * Can be cached based on the year.
 */
struct ZoneMatch {
  const ZoneInfoEntry* entry;
  const ZoneRule* rule;
  uint32_t startEpochSeconds; // transition time of given rule
  int8_t offsetCode; // effective offsetCode at the start of zone period
  // TODO: add abbreviation
};

/**
 * Manages a given ZoneInfo.
 */
class ZoneManager {
  public:
    ZoneManager(const ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    /** Return the zoneOffset of the Zone at the given epochSeconds. */
    ZoneOffset getZoneOffset(uint32_t epochSeconds) {
      // Find the year of the given epochSeconds. This could potentially fail if
      // epochSeconds is on Jan 1 or Dec 31 because we have a Catch 22 situation
      // where we don't know the actual local year until we find the
      // ZoneInfoEntry, but we can't find the ZoneInfoEntry until we know the
      // year. So we use the year assuming UTC zone offset.
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      if (!isFilled(ld.year())) {
        init(ld.year());
      }
      const ZoneMatch* match = findMatch(epochSeconds);
      return ZoneOffset::forOffsetCode(match->offsetCode);
    }

  private:
    friend class ::ZoneManagerTest_init_2001;
    friend class ::ZoneManagerTest_init_2018;

    static const uint8_t kMaxCacheEntries = 5;

    void init(uint8_t year) {
      mYear = year;
      mNumMatches = 0;
      addLastYear();
      addCurrentYear();
      calcTransitions();
    }

    bool isFilled(uint8_t year) const {
      return (year == mYear) && (mNumMatches != 0);
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

      // Find the latest rule for last year. Assume that there are no more than
      // 1 rule per month.
      const ZonePolicy* const zonePolicy = entry->zonePolicy;
      const ZoneRule* latest = nullptr;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYear <= lastYear) && (lastYear <= rule->toYear)) {
          if ((latest == nullptr) || (rule->inMonth > latest->inMonth)) {
            latest = rule;
          }
        }
      }

      // Add the last rule from the previous year.
      mPreviousMatch = {entry, latest, 0, 0};
    }

    /** Add all matching rules from the current year. */
    void addCurrentYear() {
      const ZoneInfoEntry* const entry = findEntry(mYear);

      const ZonePolicy* const zonePolicy = entry->zonePolicy;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYear <= mYear) && (mYear <= rule->toYear)) {
          addRule(entry, rule);
        }
      }
    }

    /**
     * Add (entry, rule) to the cache, in sorted order. Essentially, this is
     * doing an Insertion Sort of the ZoneMatch elements.
     */
    void addRule(const ZoneInfoEntry* entry, const ZoneRule* rule) const {
      if (mNumMatches >= kMaxCacheEntries) return;

      // insert
      mMatches[mNumMatches] = {entry, rule, 0, 0};
      mNumMatches++;

      // sort
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

    /** Calculate the epochSeconds of each ZoneMatch rule. */
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
        } else {
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
    mutable uint8_t mNumMatches = 0;
    mutable ZoneMatch mMatches[kMaxCacheEntries];
    mutable ZoneMatch mPreviousMatch; // previous year's match
};

}

#endif
