#ifndef ACE_TIME_BASIC_ZONE_SPECIFIER_H
#define ACE_TIME_BASIC_ZONE_SPECIFIER_H

#include <Arduino.h>
#include <string.h> // strchr()
#include <stdint.h>
#include "common/ZonePolicy.h"
#include "common/ZoneInfo.h"
#include "common/logger.h"
#include "TimeOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneSpecifier.h"

class BasicZoneSpecifierTest_init_primitives;
class BasicZoneSpecifierTest_init;
class BasicZoneSpecifierTest_createAbbreviation;
class BasicZoneSpecifierTest_calcStartDayOfMonth;
class BasicZoneSpecifierTest_calcRuleOffsetCode;

namespace ace_time {

namespace basic {

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
   * as of TZ database version 2018g: Europe/Istanbul and
   * America/Argentina/San_Luis.
   */
  const ZoneRule* rule;

  /** Year which applies to the ZoneEra or ZoneRule. */
  int8_t yearTiny;

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /**
   * The total effective UTC offsetCode at the start of transition, *including*
   * DST offset. (Maybe rename this effectiveOffsetCode?) The DST offset can be
   * recovered from rule->deltaCode.
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

} // namespace basic

/**
 * An implementation of ZoneSpecifier that supports a subset of the zones
 * containing in the TZ Database. The supported list of zones, as well as the
 * list of unsupported zones, are is listed in the zonedb/zone_info.h header
 * file. The constructor expects a pointer to one of the ZoneInfo structures
 * declared in the zone_infos.h file.
 *
 * The internal ZoneRule and ZoneEra records that match the year of the given
 * epochSeconds are cached for performance. The expectation is that repeated
 * calls to the various methods will have epochSeconds which do not vary too
 * greatly and will occur in the same year.
 *
 * The Rule records are transition points which look like this:
 * @verbatim
 * Rule  NAME  FROM    TO  TYPE    IN     ON        AT      SAVE    LETTER/S
 * @endverbatim
 *
 * Each record is represented by basic::ZoneRule and the entire collection is
 * represented by basic::ZonePolicy.
 *
 * The Zone records define the region which follows a specific set of Rules
 * for certain time periods (given by UNTIL below):
 * @verbatim
 * Zone NAME              GMTOFF    RULES FORMAT  [UNTIL]
 * @endverbatim
 *
 * Each record is represented by basic::ZoneEra and the entire collection is
 * represented by basic::ZoneInfo.
 *
 * This class assumes that the various components of ZoneInfo, ZoneEra, and
 * ZonePolicy, ZoneRule have a number of limitations and constraints which
 * simplify the implementation of this class. The tzcompiler.py script will
 * remove zones which do not meet these constraints when generating the structs
 * defined by zonedb/zone_infos.h. The constraints are at least the following
 * (see tools/transformer.py for the authoratative algorithm):
 *
 *  - ZoneInfo UNTIL field must contain only the full year;
 *    cannot contain month, day, or time components
 *  - ZoneInfo untilTimeModifier can contain only 'w' (not 's' or 'u')
 *  - ZoneInfo RULES column must be empty ("-"), OR refer to a
 *    named Zone Rule (e.g. "US"); cannot contain an explicit offset (hh:mm)
 *  - ZonePolicy can contain only 1 ZoneRule in a single month
 *  - ZoneRule AT time cannot occur on Jan 1
 *  - ZoneRule atTimeModifier can be any of ('w', 's', and 'u')
 *  - ZoneRule LETTER must contain only a single letter (not "WAT" or "CST")
 *
 * Even with these limitations, zonedb/zone_info.h shows that 231 out of a
 * total of 359 zones are supported by BasicZoneSpecifier.
 *
 * Not thread-safe.
 */
class BasicZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Constructor.
     * @param zoneInfo pointer to a ZoneInfo. Must not be nullptr.
     */
    explicit BasicZoneSpecifier(const basic::ZoneInfo* zoneInfo):
        ZoneSpecifier(kTypeBasic),
        mZoneInfo(zoneInfo) {}

    /** Return the underlying ZoneInfo. */
    const basic::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    TimeOffset getUtcOffset(acetime_t epochSeconds) const override {
      const basic::Transition* transition = getTransition(epochSeconds);
      int8_t code = (transition)
          ? transition->offsetCode : TimeOffset::kErrorCode;
      return TimeOffset::forOffsetCode(code);
    }

    TimeOffset getDeltaOffset(acetime_t epochSeconds) const override {
      const basic::Transition* transition = getTransition(epochSeconds);
      int8_t code;
      if (!transition) {
        code = TimeOffset::kErrorCode;
      } else if (transition->rule == nullptr) {
        code = 0;
      } else {
        code = transition->rule->deltaCode;
      }
      return TimeOffset::forOffsetCode(code);
    }

    const char* getAbbrev(acetime_t epochSeconds) const override {
      const basic::Transition* transition = getTransition(epochSeconds);
      return (transition) ? transition->abbrev : "";
    }

    /**
     * @copydoc ZoneSpecifier::getUtcOffsetForDateTime()
     *
     * The Transitions calculated by BasicZoneSpecifier contain only the
     * epochSeconds when each transition occurs. They do not contain the local
     * date/time components of the transition. This design reduces the amount
     * of memory required by BasicZoneSpecifier, but this means that the
     * information needed to implement this method correctly does not exist.
     *
     * The implementation of this method is therefore a hack. First pass, we
     * extract the TimeOffset on Jan 1 of the year given by the localDateTime,
     * and guess its epochSecond using that TimeOffset. Second pass, we use the
     * epochSecond from the previous pass to calculate the next best guess of
     * the actual TimeOffset. We return the second pass guess as the result.
     */
    OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt) const override {
      TimeOffset offset;
      bool success = init(ldt.localDate());
      if (success) {
        // First guess at the TimeOffset using Jan 1 of the given year.
        acetime_t initialEpochSeconds =
            LocalDate::forComponents(ldt.year(), 1, 1).toEpochSeconds();
        TimeOffset initialTimeOffset = getUtcOffset(initialEpochSeconds);

        // Second guess at the TimeOffset using the first TimeOffset.
        auto odt = OffsetDateTime::forLocalDateTimeAndOffset(
            ldt, initialTimeOffset);
        acetime_t epochSeconds = odt.toEpochSeconds();
        offset = getUtcOffset(epochSeconds);

        // FIXME: This is inaccurate if ldt falls in the DST gap where the ldt is
        // invalid. Add a normalization step.
      } else {
        offset = TimeOffset::forError();
      }
      return OffsetDateTime::forLocalDateTimeAndOffset(ldt, offset);
    }

    /** Print the TD database zone identifier e.g "America/Los_Angeles". */
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

    /**
     * Calculate the actual dayOfMonth of the expresssion
     * (onDayOfWeek >= onDayOfMonth). The "last{dayOfWeek}" expression is
     * expressed by onDayOfMonth being 0. An exact match on dayOfMonth is
     * expressed by setting onDayOfWeek to 0.
     *
     * Not private, used by ExtendedZoneSpecifier.
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

  private:
    friend class ::BasicZoneSpecifierTest_init_primitives;
    friend class ::BasicZoneSpecifierTest_init;
    friend class ::BasicZoneSpecifierTest_createAbbreviation;
    friend class ::BasicZoneSpecifierTest_calcStartDayOfMonth;
    friend class ::BasicZoneSpecifierTest_calcRuleOffsetCode;

    /** Maximum size of Transition cache across supported zones. */
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
    const basic::Transition* getTransition(acetime_t epochSeconds) const {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      bool success = init(ld);
      return (success) ? findMatch(epochSeconds) : nullptr;
    }

    /**
     * Initialize the transition cache, keyed by the "current" year. The
     * current year is not always the year determined by the UTC time of
     * the epoch seconds. If the UTC date is 1/1 (Jan 1), the "current" year
     * is set to be the previous year as explained below.
     *
     * There are some countries that decided to make a time zone change at on
     * 12/31 (e.g. Asia/Dhaka), which means that determining the correct DST
     * offset on 1/1 requires the Transitions from the previous year. To
     * support these zones, if the UTC date is 1/1, then we force the
     * transition cache to be generated using the *previous* year. This
     * workaround will fail for zones which have DST transitions on 1/1.
     * Therefore, the zone_info.h generator (tools/tzcompiler.py) removes all
     * zones which have time zone transitions on 1/1 from the list of supported
     * zones.
     *
     * The high level algorithm for determining the DST transitions is as
     * follows:
     *
     *  1. Find the last ZoneRule that was active just before the current year.
     *  2. Find the ZoneRules which are active in the current year.
     *  3. Calculate the Transitions given the above ZoneRules.
     *  4. Calculate the zone abbreviations (e.g. "PDT" or "BST") for each
     *  Transition.
     *
     * Returns success status: true if successful, false if an error occurred
     * (e.g. out of bounds).
     */
    bool init(const LocalDate& ld) const {
      int16_t year = ld.year();
      if (ld.month() == 1 && ld.day() == 1) {
        year--;
      }
      if (isFilled(year)) return true;

      mYear = year;
      mNumTransitions = 0; // clear cache

      if (year < mZoneInfo->zoneContext->startYear - 1
          || mZoneInfo->zoneContext->untilYear < year) {
        return false;
      }

      addRulePriorToYear(year);
      addRulesForYear(year);
      calcTransitions();
      calcAbbreviations();

      mIsFilled = true;
      return true;
    }

    /** Check if the Transition cache is filled for the given year. */
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
      const basic::ZoneEra* const era = findZoneEraPriorTo(year);

      // If the prior ZoneEra is a simple Era (no zone policy), then create a
      // Transition using a rule==nullptr. Otherwise, find the latest rule
      // within the ZoneEra.
      const basic::ZonePolicy* const zonePolicy = era->zonePolicy;
      const basic::ZoneRule* latest = nullptr;
      if (zonePolicy != nullptr) {
        // Find the latest rule for the matching ZoneEra whose
        // ZoneRule::toYearTiny < yearTiny. Assume that there are no more than
        // 1 rule per month.
        for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
          const basic::ZoneRule* const rule = &zonePolicy->rules[i];
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
        const basic::ZoneRule* a, const basic::ZoneRule* b) {
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
        const basic::ZoneRule* rule) {
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
      const basic::ZoneEra* const era = findZoneEra(year);

      // If the ZonePolicy has no rules, then add a Transition which takes
      // effect at the start time of the current year.
      const basic::ZonePolicy* const zonePolicy = era->zonePolicy;
      if (zonePolicy == nullptr) {
        addRule(year, era, nullptr);
        return;
      }

      // If the ZonePolicy has rules, find all matching transitions, and add
      // them to mTransitions, in sorted order according to the
      // ZoneRule::inMonth field.
      int8_t yearTiny = year - LocalDate::kEpochYear;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const basic::ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYearTiny <= yearTiny) &&
            (yearTiny <= rule->toYearTiny)) {
          addRule(year, era, rule);
        }
      }
    }

    /**
     * Add (era, rule) to the cache, in sorted order according to the
     * 'ZoneRule::inMonth' field. This assumes that there are no more than one
     * transition per month, so tzcompiler.py removes ZonePolicies which have
     * multiple transitions in one month (e.g. Egypt, Palestine, Spain,
     * Tunisia).
     *
     * Essentially, this method is doing an Insertion Sort of the Transition
     * elements. Even through it is O(N^2), for small number of Transition
     * elements, this is faster than the O(N log(N)) sorting algorithms. The
     * nice property of this Insertion Sort is that if the ZoneInfoEntries are
     * already sorted, then the loop terminates early and the total sort time
     * is O(N).
     */
    void addRule(int16_t year, const basic::ZoneEra* era,
          const basic::ZoneRule* rule) const {

      // If a zone needs more transitions than kMaxCacheEntries, the check below
      // will cause the DST transition information to be inaccurate, and it is
      // highly likely that this situation would be caught in the
      // BasicValidationTest unit test. Since BasicValidationTest passes, I
      // feel confident that those zones which need more than kMaxCacheEntries
      // are already filtered out by tzcompiler.py due to constraints of
      // BasicValidationTest which are checked by tzcompiler.py.
      //
      // Ideally, the tzcompiler.py script would explicitly remove those zones
      // which need more than kMaxCacheEntries Transitions. But this would
      // require a Python version of the BasicZoneSpecifier, and unfortunately,
      // zone_specifier.py implements only the ExtendedZoneSpecifier algorithm
      // An early version of zone_specifier.py may have implemented something
      // close to BasicZoneSpecifier, and it may be available in the git
      // history. But it seems like too much work right now to try to dig that
      // out, just to implement the explicit check for kMaxCacheEntries. It
      // would mean maintaining another version of zone_specifier.py.
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
        basic::Transition& left = mTransitions[i - 1];
        basic::Transition& right = mTransitions[i];
        // assume only 1 rule per month
        if ((left.rule != nullptr && right.rule != nullptr &&
              left.rule->inMonth > right.rule->inMonth)
            || (left.rule != nullptr && right.rule == nullptr)) {
          basic::Transition tmp = left;
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
    const basic::ZoneEra* findZoneEra(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const basic::ZoneEra* era = &mZoneInfo->eras[i];
        if (year < era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      // Return the last ZoneEra if we run off the end.
      return &mZoneInfo->eras[mZoneInfo->numEras - 1];
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
    const basic::ZoneEra* findZoneEraPriorTo(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const basic::ZoneEra* era = &mZoneInfo->eras[i];
        if (year <= era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      // Return the last ZoneEra if we run off the end.
      return &mZoneInfo->eras[mZoneInfo->numEras - 1];
    }

    /** Calculate the epochSeconds and offsetCode of each Transition. */
    void calcTransitions() const {
      // Calculate epochSeconds and offsetCode for the prevTransition.
      mPrevTransition.startEpochSeconds = kMinEpochSeconds;
      int8_t deltaCode = (mPrevTransition.rule == nullptr)
          ? 0 : mPrevTransition.rule->deltaCode;
      mPrevTransition.offsetCode = mPrevTransition.era->offsetCode + deltaCode;
      const basic::Transition* prevTransition = &mPrevTransition;

      // Loop through Transition items to calculate:
      // 1) Transition::startEpochSeconds
      // 2) Transition::offsetCode
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        basic::Transition& transition = mTransitions[i];
        const int16_t year = transition.yearTiny + LocalDate::kEpochYear;

        if (transition.rule == nullptr) {
          // If the transition is simple (has no named rule), then the
          // ZoneEra applies for the entire year (since BasicZoneSpecifier
          // supports only whole year in the UNTIL field). The whole year UNTIL
          // field has an implied 'w' modifier on 00:00, we don't need to call
          // calcRuleOffsetCode() with a 'w', we can just use the previous
          // transition's offset to calculate the startDateTime of this
          // transition.
          //
          // Also, when transition.rule == nullptr, the mNumTransitions should
          // be 1, since only a single transition is added by
          // addRulesForYear().
          const int8_t offsetCode = prevTransition->offsetCode;
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, 1, 1, 0, 0, 0,
              TimeOffset::forOffsetCode(offsetCode));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();
          transition.offsetCode = transition.era->offsetCode;
        } else {
          // In this case, the transition points to a named ZonePolicy, which
          // means that there could be multiple ZoneRules associated with the
          // given year. For each transition, determine the startEpochSeconds,
          // and the effective offset code.

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
              TimeOffset::forOffsetCode(offsetCode));
          transition.startEpochSeconds = startDateTime.toEpochSeconds();

          // Determine the effective offset code
          transition.offsetCode =
              transition.era->offsetCode + transition.rule->deltaCode;
        }

        prevTransition = &transition;
      }
    }

    /**
     * Determine the offset of the 'atTimeModifier'. If 'w', then we
     * must use the offset of the *previous* zone rule. If 's', use the current
     * base offset (which does not contain the extra DST offset). If 'u', 'g',
     * 'z', then use 0 offset.
     */
    static int8_t calcRuleOffsetCode(int8_t prevEffectiveOffsetCode,
        int8_t currentBaseOffsetCode, uint8_t atModifier) {
      if (atModifier == 'w') {
        return prevEffectiveOffsetCode;
      } else if (atModifier == 's') {
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
    static void calcAbbreviation(basic::Transition* transition) {
      createAbbreviation(
          transition->abbrev,
          basic::Transition::kAbbrevSize,
          transition->era->format,
          (transition->rule) ? transition->rule->deltaCode : 0,
          (transition->rule) ? transition->rule->letter : '\0');
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
    const basic::Transition* findMatch(acetime_t epochSeconds) const {
      const basic::Transition* closestMatch = &mPrevTransition;
      for (uint8_t i = 0; i < mNumTransitions; i++) {
        const basic::Transition* m = &mTransitions[i];
        if (m->startEpochSeconds <= epochSeconds) {
          closestMatch = m;
        }
      }
      return closestMatch;
    }

    const basic::ZoneInfo* const mZoneInfo;

    mutable int16_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumTransitions = 0;
    mutable basic::Transition mTransitions[kMaxCacheEntries];
    mutable basic::Transition mPrevTransition; // previous year's transition
};

}

#endif
