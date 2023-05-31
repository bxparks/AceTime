# Changelog

* Unreleased
    * Update `ace_time/testing/*` classes to support splitting the test data
      into 2 separate lists: `transitions` and `samples`.
        * Required to support new `validation_data.json` from AceTimeValidation.
    * Update `AceTimeValidation` test names to `tests/Xxx{Basic,Extended}Test`.
    * Add `ZonedExtra::kAbbrevSize` to define the `char` buffer size needed to
      hold an abbreviation.
    * Change `ZonedExtra::kInvalidMinutes` from public to private.
        * This is an implementation detail.
        * Use `ZonedExtra::isError()` instead.
    * Rename AceTimePython to acetimepy.
    * ZoneProcessor transition cache
        * Save the epoch year, and automatically invalidate and regenerate the
          cache when the `Epoch::currentEpochYear()` is modified.
        * Remove cache invalidation methods which are no longer needed:
            * `ZoneProcessor::resetTransitionCache()`
            * `ZoneProcessorCache::resetZoneProcessors()`
            * `ZoneManager::resetZoneProcessors()`
    * Rename "Converter Epoch" to "Internal Epoch".
        * Change `daysToCurrentEpochFromConverterEpoch()` to
          `daysToCurrentEpochFromInternalEpoch()`.
        * This is an internal implementation detail, exposed only for testing
          purposes.
* 2.2.2 (2023-04-01, TZDB version 2023c)
    * Upgrade TZDB from 2023b to 2023c.
        * https://mm.icann.org/pipermail/tz-announce/2023-March/000079.html
            * "This release's code and data are identical to 2023a.  In other
              words, this release reverts all changes made in 2023b other than
              commentary, as that appears to be the best of a bad set of
              short-notice choices for modeling this week's daylight saving
              chaos in Lebanon."
    * AceTime is forced to upgrade to 2023c, because we skipped 2023a and went
      directly to 2023b, which is being rolled back by 2023c.
* 2.2.1 (2023-03-24, TZDB version 2023b)
    * Actually regenerate the `zonedb*` files to 2023b.
* 2.2.0 (2023-03-24, TZDB version 2023b)
    * Upgrade TZDB from 2022g to 2023b
        * 2023a skipped because it came out only a day earlier.
        * 2023a: https://mm.icann.org/pipermail/tz-announce/2023-March/000077.html
            * Egypt now uses DST again, from April through October.
            * This year Morocco springs forward April 23, not April 30.
            * Palestine delays the start of DST this year.
            * Much of Greenland still uses DST from 2024 on.
            * America/Yellowknife now links to America/Edmonton.
            * tzselect can now use current time to help infer timezone.
            * The code now defaults to C99 or later.
            * Fix use of C23 attributes.
        * 2023b: https://mm.icann.org/pipermail/tz-announce/2023-March/000078.html
            * Lebanon delays the start of DST this year.
    * **Bug Fix**
        * Change arguments for `TimeZone::forMinutes()` from `uint8_t` to
          `uint16_t`.
    * **Breaking Change**
        * Make `TimeZone` effectively immutable, by removing `setStdOffset()`
          and `setDstOffset()` methods.
        * Client applications should create a new `TimeZone` object using
          `TimeZone::forTimeOffset(std, dst)` and overwrite the old one.
        * See [Migrating to v2.2](MIGRATING.md#MigratingToVersion220).
    * Add support for Seeed Studio XIAO M0 (SAMD21).
        * Required updating `zoneino/compat.h` to clobber the broken definition
          of `FPSTR()` in Seeeduino 1.8.3.
    * Simplify the handling of `Rule.LETTER`.
        * Encode all letters as an index into `ZoneContext.letters` array, not
          just `LETTER` which are only a single character.
        * On 8-bit AVR:
            * Increases `BasicZoneProcessor` by ~200 bytes when 1-2 zones are
              used. But flash remains the same when the full TZ database is
              used.
            * No change to `ExtendedZoneProcessor` for 1-2 zones. But decreases
              flash usage by ~300 bytes when the full TZ database is used.
        * The small increase in flash is worth it because this greatly
          simplifies the complicated code surrounding LETTER that was difficult
          to understand and maintain.
    * Unify Basic and Extended zoneinfo encoding.
        * Change encoding of `basic::ZoneRule.deltaCode`,
          `basic::ZoneEra.deltaCode`, and `basic::ZoneEra.offsetCode` to be
          identical their counterparts in `extended::ZoneXxx`.
        * Merge `BasicBrokers.h` and `ExtendedBrokers.h` into just `Brokers.h`.
    * Restructure zonedb directories
        * Lift `ace_time/internal/testing/tzonedb[x]` database files 
          to `ace_time/tzonedb[x]`.
        * Lift `ace_time/zonedb[x]` database files to `ace_time/zonedb[x]`.
        * These changes are transparent to client apps because the C++
          namespaces of these files are unchanged.
        * Lift `ace_time/internal/Zone*.h` files into new `src/zoneinfo/`
          directory. These are the classes that describe the `*zonedb*`
          databases.
    * Add `ZonedDateTime::offsetDateTime()`
        * Returns the underlying `OffsetDateTime` inside the `ZonedDateTime`.
        * Analogous to `ZonedDateTime::localDateTime()`.
    * Always generate anchor rules in zonedb.
        * Allows `ExtendedZoneProcessor` to work over all years `[0,10000)`
          even with truncated zonedb (e.g. `[2000,2100)`).
        * Accuracy is guaranteed only for the requested interval (e.g.
          `[2000,2100)`.
        * But the code won't crash outside of that interval.
* 2.1.1 (2023-02-02, TZDB version 2022g)
    * `ZonedExtra`
        * Add `ZonedExtra::forComponents()` factory method, for consistency with
          `ZonedDateTime` class.
    * `examples/AutoBenchmark`
        * Add `ZonedExtra` benchmarks.
    * `README.md`
        * Update `AceTimeValidation` validation years to `[2000,2100)`.
        * Update memory and cpu benchmarks.
* 2.1.0 (2023-01-29, TZDB version 2022g)
    * There are a handful API breaking changes in this release in the pursuit of
      simpler and cleaner code. See the following for more info:
        * [Migrating to v2.1](MIGRATING.md#MigratingToVersion210)
        * [ZonedExtra](USER_GUIDE.md#ZonedExtra) in the User Guide
        * [Unified Links](USER_GUIDE.md#UnifiedLinks) in the User Guide
    * **Potentially Breaking**: zonedb,zonedbx
        * Rename `kPolicyXxx` to `kZonePolicyXxx` for consistency. These are
          expected to be used only internally, so shouldn't cause external
          breakage.
    * **Breaking**: `TimeZone.h`
        * Replace 3 separate extraction methods in `TimeZone` with a new
          `ZonedExtra` class
        * Removed: `TimeZone::getUtcOffset()`
            * Replaced by: `ZonedExtra::timeOffset()`
        * Removed: `TimeZone::getDeltaOffset()`
            * Replaced by: `ZonedExtra::dstOffset()`
        * Removed `TimeZone::getAbbrev()`
            * Replaced by: `ZonedExtra::abbrev()`
            * `ZonedExtra::abbrev()` returns pointer to a local string buffer
              instead of a transient buffer deep inside `Transition` object.
            * `TimeZone` becomes closer to being thread-safe
    * **New Class**: `ZonedExtra.h`
        * `ZonedExtra::forEpochSeconds(epochSeconds, tz)`
            * Create instance from epochSeconds and time zone.
        * `ZonedExtra::forLocalDateTime(ldt, tz)`
            * Create instance from LocalDateTime and time zone.
    * **Potentially Breaking**: Unified Links
        * Links are now first-class citizens, exactly the same as Zones.
        * Unify "fat links" and "symbolic links" into a single implementation.
        * Remove "thin links" to simplify the code.
        * `TimeZone` class simplified
            * Removed `followLink` flag on various methods.
            * Only 2 methods apply to Links: `isLink()` and
              `printTargetNameTo()`.
    * Simplify ZoneProcessors
        * `ZoneProcessor.h`, `ExtendedZoneProcessor.h`, `BasicZoneProcessor.h`
        * Remove: `getUtcOffset()`, `getDeltaOffset()`, `getAbbrev()`
        * Replaced by: `findByLocalDateTime()`, `findByEpochSeconds()`
        * These are internal helper methods not intended for public consumption.
    * Unit tests
        * Migrate most unit tests to use the smaller, testing zone databases at
          `testing/tzonedb/` and `testing/tzonedbx/`.
            * Reduces maintenance cost of various hand-crafted ZoneInfo and
              ZonePolicy entries for unit tests.
            * Can test against real timezones with predictable behavior.
* 2.0.1 (2022-12-04, TZDB 2022g)
    * Prevent `ExtendedZoneProcssor::generateStartUntilTimes()` from
      dereferencing uninitialized memory if there are no matching transitions.
        * This may happen if `zonedbx` is accidentally corrupted (e.g. by using
          one with `int8` year fields instead of `int16` year fields).
    * Incorporate notable `zone_policies.h` comments into notable
      `zone_infos.h`.
    * Upgrade TZDB from 2022f to 2022g
        * https://mm.icann.org/pipermail/tz-announce/2022-November/000076.html
            * The northern edge of Chihuahua changes to US timekeeping.
            * Much of Greenland stops changing clocks after March 2023.
            * Fix some pre-1996 timestamps in northern Canada.
            * C89 is now deprecated; please use C99 or later.
            * Portability fixes for AIX, libintl, MS-Windows, musl, z/OS
            * In C code, use more C23 features if available.
            * C23 timegm now supported by default
            * Fixes for unlikely integer overflows
* 2.0 (2022-11-04, TZDB 2022f) **Breaking Change** See
      [Migrating to 2.0.0](MIGRATING.md#MigratingToVersion200)
    * Change internal storage type of `year` component from `int8_t` to
      `int16_t`, extending the range of valid years from [-1873,2127] to
      [1,9999].
        * Remove `yearTiny()` getters and setters from `LocalDate`,
          `LocalDateTime`, `OffsetDateTime`, and `ZonedDateTime`.
            * They were not documented except in doxygen docs.
        * Remove from `LocalDate`:
            * `kInvalidYearTiny`, replaced with `kInvalidYear`
            * `kMinYearTiny`, replaced with `kMinYear`
            * `kMaxYearTiny`, replaced with `kMaxYear`
            * `forTinyComponents()`
        * Remove from `LocalDateTime`
            * `forTinyComponents()`
        * Update [AceTimeTools](https://github.com/bxparks/AceTimeTools)
          to generate `src/zonedb` and `src/zonedbx` using `int16_t` year types.
    * Extend `untilYear` of [zonedb](src/ace_time/zonedb) and
      [zonedbx](src/ace_time/zonedbx) databases to 10000
        * databases now valid over the years `[2000,10000)`
        * `zonedbx` adds 75 additional Rules for `kZonePolicyMorocco` (e.g.
          zone "Africe/Casablanca") due to the precalculated DST shifts which
          are listed in the IANA TZ DB up to the year 2087.
        * `zonedb` remains unchanged
    * Change epoch seconds conversion algorithm
        * Extract different epoch date conversion algorithms to be used/tested.
          Two of them are `EpochConverterJulian` and `EpochConverterHinnant`
            * `EpochConverterJulian` implements the algorithms found in
              wikipedia article https://en.wikipedia.org/wiki/Julian_day.
            * `EpochConverterHinnant` implements the algorithms found in
              https://howardhinnant.github.io/date_algorithms.html.
        * Migrate `LocalDate` to use the `EpochConverterHinnant` instead of
          `EpochConverterJulian`.
            * The primary reason is that I am able to fully understand the
              algorithms described in `EpochConverterHinnant`.
            * In contrast, I have almost no understanding of the algorithms
              implemented by `EpochConverterJulian`.
    * Configurable epoch year using new `Epoch` utility class
        * Add `Epoch::currentEpochYear()` which allows customization of the
          internal epoch year at startup.
            * Expected to be rarely used in user applications, but somewhat
              common in unit testing.
        * Add `Epoch::epochValidYearLower()` and `Epoch::epochValidYearUpper()`
            * Defines the 100-year interval which is +/- 50 years from the
              `currentEpochYear()` where the epoch seconds and time zone
              transition algorithms are guaranteed to be valid.
        * Add cache invalidation methods which must be called if
          `currentEpochYear()` is changed at runtime.
            * `ZoneProcessor::resetTransitionCache()`
            * `ZoneProcessorCache::resetZoneProcessors()`
            * `ZoneManager::resetZoneProcessors()`
    * Remove `toUnixSeconds()` and `forUnixSeconds()` which use the 32-bit
      versions of unix epoch seconds.
        * They will become invalid in the year 2038, and it's now the year 2022
          so it does not seem worth maintaining these.
        * The 64-bit versions `toUnixSeconds64()` and `forUnixSeconds64()` are
          retained.
    * Flash usage increases (see [MemoryBenchmark](examples/MemoryBenchmark) for
      more details:
        * AVR:
            * BasicZoneManager increases ~200 bytes
            * ExtendedZoneManager increases ~500 bytes
            * `zonedb` increases ~1.5 kiB
            * `zonedbx` increases ~3 kiB
        * ESP8266
            * BasicZoneManager increases ~50 bytes
            * ExtendedZoneManager increases ~150 bytes
            * `zonedb` increases ~300 bytes
            * `zonedbx` increases ~1.5 kiB
* 1.11.7 (2022-11-02, TZDB 2022f)
    * Upgrade TZDB from 2022e to 2022f
        * https://mm.icann.org/pipermail/tz-announce/2022-October/000075.html
			* Mexico will no longer observe DST except near the US border.
			* Chihuahua moves to year-round -06 on 2022-10-30.
			* Fiji no longer observes DST.
			* Move links to 'backward'.
			* In vanguard form, GMT is now a Zone and Etc/GMT a link.
			* zic now supports links to links, and vanguard form uses this.
			* Simplify four Ontario zones.
			* Fix a Y2438 bug when reading TZif data.
			* Enable 64-bit time_t on 32-bit glibc platforms.
			* Omit large-file support when no longer needed.
			* In C code, use some C23 features if available.
			* Remove no-longer-needed workaround for Qt bug 53071.
* 1.11.6 (2022-10-22, TZDB 2022e)
    * Upgrade TZDB from 2022d to 2022e
        * https://mm.icann.org/pipermail/tz-announce/2022-October/000074.html
            * Jordan and Syria switch from +02/+03 with DST to year-round +03.
        * BasicZoneProcessor can no longer support Asia/Amman (Jordan) and
          Asia/Damascus because the transition to permanent +03 occurs
          at an instant which cannot be handled by the simple algorithm in
          BasicZoneProcessor. Use ExtendedZoneProcessor instead.
* 1.11.5 (2022-10-06, TZDB 2022d)
    * Upgrade TZDB from 2022b to 2022d
        * 2022c
            * https://mm.icann.org/pipermail/tz-announce/2022-August/000072.html
                * Work around awk bug in FreeBSD, macOS, etc.
                * Improve tzselect on intercontinental Zones.
            * Skipped because there were no changes that affected AceTime.
        * 2022d
            * https://mm.icann.org/pipermail/tz-announce/2022-September/000073.html
                * Palestine transitions are now Saturdays at 02:00.
                * Simplify three Ukraine zones into one.
    * Upgrade tool chain
        * Arduino CLI from 0.19.2 to 0.27.1
        * Arduino AVR Core from 1.8.4 to 1.8.5
        * STM32duino from 2.2.0 to 2.3.0
        * ESP32 Core from 2.0.2 to 2.0.5
        * Teensyduino from 1.56 to 1.57
* 1.11.4 (2022-08-13, TZDB 2022b)
    * Add `ace_time::daysUntil(localDate, month, day)` utility function that
      returns the number of days until the next (month, day) date. Useful for
      calculating the number of days until the next Christmas for example.
    * Upgrade to TZDB 2022b.
        * https://mm.icann.org/pipermail/tz-announce/2022-August/000071.html
            * Chile's DST is delayed by a week in September 2022.
            * Iran no longer observes DST after 2022.
            * Rename Europe/Kiev to Europe/Kyiv.
            * Finish moving duplicate-since-1970 zones to 'backzone'.
        * zonedb
            * Number of zones decreases from 258 to 237.
            * Number of links increases from 193 to 215.
        * zonedbx
            * Number of zones decreases from 377 to 356.
            * Number of links increases from 217 to 239.
* 1.11.3 (2022-03-20, TZDB 2022a)
    * Update to TZDB 2022a.
        * https://mm.icann.org/pipermail/tz-announce/2022-March.txt
        * "Palestine will spring forward on 2022-03-27, not -03-26."
    * No changes to code.
* 1.11.2 (2022-02-24, TZDB 2021e)
    * Fix crash triggered by certain subclasses of `BasicZoneProcessor` and
      `ExtendedZoneProcessor`.
        * The code did not handle nullptr for the `BrokerFactory` properly.
        * These particular classes do not know their `BrokerFactory` at
          compile-time, so they are set to `nullptr` initially. The
          `setBrokerFactory()` is expected to be called at runtime later.
        * No effect on the AceTime library itself which does not trigger this
          condition.
* 1.11.1 (2022-02-16, TZDB 2021e)
    * Update `ZoneInfoBroker::targetZoneInfo()` to return a `ZoneInfoBroker`
      instead of a raw `ZoneInfo*` pointer.
        * Internal wiring change, no change to external API.
        * Allows alternative `ZoneInfoBroker` classes to implement a consistent
          API.
* 1.11.0 (2022-02-14, TZDB 2021e)
    * Regenerate `zonedb/` and `zonedbx/` using latest AceTimeTool which
      identifies notable Zones and Policies whose DST shifts are not exactly
      0:00 or 1:00. No actual data change. Notable policies relevant from 2000
      until 2050 are:
        * ZonePolicy Eire: DST shift -1:00
        * ZonePolicy LH: DST shift 0:30
        * ZonePolicy Morocco: DST shift -1:00
        * ZonePolicy Namibia: DST shift -1:00
        * ZonePolicy StJohns: DST shift 2:00
        * ZonePolicy Troll: DST shift 2:00
    * Add support for overflow and underflow to `TimePeriod`.
        * When `isError()` returns `true`, the `sign()` method discriminates
          3 error conditions, and the `printTo()` prints the following:
          * 0: generic error, "<Error>"
          * +1: overflow, "<+Inf>"
          * -1: underflow, "<-Inf>"
          * See [USER_GUIDE.md#TimePeriod](USER_GUIDE.md#TimePeriod).
    * Change Link entries from "hard links" to "symbolic links".
        * This allows a TimeZone object to know whether it is a Zone entry
          or a Link entry.
        * Add `TimeZone::isLink()` and `ZoneProcessor::isLink()` methods.
        * Add `bool followLink = true` parameter to various other methods
          (`getZoneId()`, `printTo()`, `printShortTo()`) which determines
          whether the method resolves to the current Link entry or follows the
          link to the destination Zone entry.
        * [MemoryBenchmark](examples/MemoryBenchmark) says that this increases
          flash consumption by 100-300 bytes, due to the extra code needed to
          follow the symlink. But the ability to determine whether the TimeZone
          is a link or not will be necessary for an upcoming feature, so this
          slight increase in flash consumption seems worth it.
        * Regenerate `zonedb/` and `zonedbx` to convert Link entries from
          hard links to symbolic links.
        * See [Symbolic Links](USER_GUIDE.md#SymbolicLinks) for more info.
    * Add `offset_date_time_mutation` methods.
        * Similar to `zoned_date_time_mutation` methods.
        * Provides some convenient mutation methods for `OffsetDateTime`.
    * Upgrade Arduino CLI from 0.19.2 to 0.20.2.
* 1.10.0 (2022-01-18, TZDB 2021e)
    * MemoryBenchmark: Add memory consumption for `ZoneSorterByName` and
      `ZoneSorterByOffsetAndName`.
        * AVR: 180-530 bytes of flash
        * 32-bit: 120-600 bytes of flash
    * Rename internal `TransitionStorage::findTransition()` to
      `findTransitionForSeconds()` for better self-documentation.
    * Move third party SAMD21 boards to new Tier 3 (May work, but not supported)
      level.
        * I can no longer upload binaries to these boards using Arduino IDE
          1.8.19 and SparkFun SAMD Core 1.8.6.
    * Add support for `fold` parameter to control behavior around DST gaps
      and overlaps.
        * The semantics of the `fold` parameter is intended to be identical to
          [Python PEP 495](https://www.python.org/dev/peps/pep-0495).
        * Add `LocalTime::fold()`, `LocalDateTime::fold()`,
          `OffsetDateTime::fold()`, `ZonedDateTime::fold()`.
        * Update `ExtendedZoneProcessor::getOffsetDateTime(acetime_t)` to
          calculate the `OffsetDateTime::fold()` as an output parameter.
        * Update `ExtendedZoneProcessor::getOffsetDateTime(const
          LocalDateTime&)` to handle `LocalDateTime::fold()` as an input
          parameter.
        * Increases flash usage of `ExtendedZoneProcessor` by around 600 bytes
          on AVR, and 400-600 bytes on 32-bit processors.
    * Add `toUnixSeconds64()` and `forUnixSeconds64()` methods to
      `LocalDate`, `LocalDateTime`, `OffsetDateTime`, `ZonedDateTime`.
        * These use 64-bit `int64_t` integers, which allows Unix seconds to be
          used up to 2068-01-19T03:14:07Z (which is the limit of these
          various classes due to the internal use of 32-bit `acetime_t`).
        * These methods make it easier to interoperate with the `time_t` typedef
          for `int64_t` on the ESP8266 and ESP32 platforms.
    * Add [EspTime](examples/EspTime) app that shows how to integrate
      the SNTP client on the ESP8266 and ESP32 platforms with AceTime.
* 1.9.0 (2021-12-02, TZDB 2021e)
    * Add `ZoneSorterByName` and `ZoneSorterByOffsetAndName` classes
      to sort zone indexes, ids, or names according to 2 pre-defined sorting
      criteria: (1) by name, or (2) by UTC offset and then by name.
        * See the [Zone Sorting](USER_GUIDE.md#ZoneSorting) section in the
          `USER_GUIDE.md`.
        * Adds a new dependency to
          [AceSorting](https://github.com/bxparks/AceSorting) library.
    * Add `examples/CompareAceTimeToHinnantDate` to compare the performance of
      AceTime compared to Hinnant date library.
        * AceTime seems to be about 90X faster for converting date-time
          components to epoch seconds.
    * Add `MaxBufSize` comment field into `zonedb[x]/zone_infos.h` which is the
      maximum over all zones in that file. Must be less than or equal to
      `ExtendedZoneProcessor::kMaxTransitions`.
    * **Potential Breaking Change**: `class TransitionStorage`
        * Rename `getHighWater()` to `getAllocSize()`. This now returns the
          maximum number of transitions that has been allocated so far, which
          happens to be `getHighWater() + 1`.
        * Rename `resetHighWater()` to `resetAllocSize()`.
        * Rename `ExtendedZoneProcessor::resetTransitionHighWater()` to
          `resetTransitionAllocSize()`.
        * All of these methods were intended for internal debugging so these
          changes are not considered to be an API change.
        * The semantics of these methods are now closer to the algorithm in
          `acetimepy/zone_processor.py`.
    * **Breaking Change**: Extract `BasicZoneProcessorCache` and
      `ExtendedZoneProcessorCache` out of `BasicZoneManager` and
      `ExtendedZoneManager`. Remove all pure `virtual` methods from
      `ZoneManager`, making the class hierarchy non-polymorphic.
        * Saves 1100-1300 bytes of flash on AVR processors.
        * See [Migrating to v1.9](MIGRATING.md#MigratingToVersion190) for
          migration info.
    * **Breaking Change**: Remove pure `virtual` methods from `LinkManager`,
      analogous to their removal from `ZoneManager`.
        * Saves 68 bytes of flash on AVR processors.
        * See [Migrating to v1.9](MIGRATING.md#MigratingToVersion190) for
          migration info.
* 1.8.2 (2021-10-28, TZDB 2021e)
    * Update to TZDB 2021e.
        * https://mm.icann.org/pipermail/tz-announce/2021-October/000069.html
        * Palestine will fall back 10-29 (not 10-30) at 01:00.
* 1.8.1 (2021-10-18, TZDB 2021d)
    * Add `make -C examples/MemoryBenchmark epoxy` to GitHub actions.
    * Upgrade to TZDB 2021d.
        * https://mm.icann.org/pipermail/tz-announce/2021-October/000068.html
        * Fiji suspends DST for the 2021/2022 season.
* 1.8.0 (2021-10-15, TZDB 2021c)
    * **Breaking Change**: Move clock classes under `ace_time::clock` and
      implementation classes under `ace_time::hw` to the new
      [AceTimeClock](https://github.com/bxparks/AceTimeClock) repo.
        * Classes remain in the same C++ namespace.
        * Client code needs to add `#include <AceTimeClock.h>`.
        * See [Migrating to v1.8](MIGRATING.md#MigratingToVersion180) for
          migration info.
    * **Breaking Change**: Convert `DS3231.h` into a template class with an
      indirect dependency to `<AceWire.h>`, replacing direct dependency on
      `<Wire.h>`.
        * Just including the `<Wire.h>` header causes flash memory to be
          consumed, even if `Wire` object is never used.
        * Saves 1000-1500 bytes of flash on AVR, and up to 4000 bytes on STM32.
        * See [Migrating to v1.8](MIGRATING.md#MigratingToVersion180) for
          migration info.
    * **Breaking Change**: Extract thin link functionality from
      `BasicZoneManager` and `ExtendedZoneManager` into new `BasicLinkManager`
      and `ExtendedLinkManager`.
        * Saves 200-500 bytes of flash memory if the feature is not used.
        * Client application can determine whether to pay for this
          functionality, instead of automatically being included into the
          `ZoneManager`.
        * See the [Thin Links](USER_GUIDE.md#ThinLinks) section in the User
          Guide and [Migrating to v1.8](MIGRATING.md#MigratingToVersion180) for
          migration info.
    * Simplify documentation
        * Merge `docs/installation.md` into README.md.
        * Move `docs/date_time_timezone.md` to `USER_GUIDE.md`.
        * Remove `docs/clock_system_clock.md` after migrating it to
          the AceTimeClock project.
        * Merge `docs/comparisons.md` into README.md.
* 1.7.5 (2021-10-06, TZDB 2021c)
    * **Bug Fix**: Update `ExtendedZoneProcessor.h` to implement better
      detection of Transitions that occur at the exact same time as the switch
      to a different `ZoneEra`.
        * They are now considered to happen at the same time if any of the 'w'
          time, 's' time, or 'u' time are equal.
        * The behavior of `ExtendedZoneProcessor.h` should now be identical
          to `zone_processor.py` in the `acetimepy` library.
        * Seems to affect only `Europe/Lisbon` in 1992, which is not a part of
          the predefined `zonedb` or `zonedbx` database, which normally include
          only 2000 until 2050.
    * Testing
        * Create [acetimepy](https://github.com/bxparks/acetimepy)
          library extracted from the previously split
          [AceTimeTools](https://github.com/bxparks/AceTimeTools) project.
        * Update `ace_time/testing/ExtendedTransitionTest.h` to validate
          the exact equality between the observed maximum buffer size of
          TransitionStorage as observed by `ExtendedZoneProcessor` and the
          buffer size calculated by AceTimeTools using `zone_processor.py` of
          `acetimepy` library.
        * Create `examples/DebugZoneProcessor` for debugging the internal logic
          of `ExtendedZoneProcessor`.
    * Tool Chain
        * Upgrade ESP8266 Core from 2.7.4 to 3.0.2.
            * Flash consumption increases by 3-5 kB across the boad.
        * Upgrade Arduino CLI from 0.19.1 to 0.19.2.
        * Upgrade Arduino IDE from 1.8.13 to 1.8.16.
        * Upgrade Teensyduino from 1.54 to 1.55.
        * Upgrade SparkFun SAMD Core from 1.8.3 to 1.8.4.
    * **TZDB** Upgrade TZDB from 2021a to 2021c.
        * TZDB 2021b is skipped because of controversial changes which were
          reverted in 2021c.
        * 2021c announcement:
          https://mm.icann.org/pipermail/tz-announce/2021-October/000067.html
        * 2021b announcement:
          https://mm.icann.org/pipermail/tz-announce/2021-September/000066.html
        * Jordan now starts DST on February's last Thursday.
        * Samoa no longer observes DST.
        * 10 Zones from 2021a were converted to Links:
            * `Africa/Accra -> Africa/Abidjan`
            * `America/Atikokan -> America/Panama`
            * `America/Blanc-Sablon -> America/Puerto_Rico`
            * `America/Creston -> America/Phoenix`
            * `America/Curacao -> America/Puerto_Rico`
            * `America/Nassau -> America/Toronto`
            * `America/Port_of_Spain -> America/Puerto_Rico`
            * `Antarctica/DumontDUrville -> Pacific/Port_Moresby`
            * `Antarctica/Syowa -> Asia/Riyadh`
            * `Pacific/Enderbury -> Pacific/Kanton`
        * 1 new Zone was created:
            * `Pacific/Kanton`
        * BasicZoneManager now supports 258 Zones and 193 Links using the
          `zonedb` database
        * ExtendedZoneManager now supports 377 Zones and 217 Links using the
          `zonedbx` database
* 1.7.4 (2021-08-26, TZDB 2021a)
    * Move `./tools` directory into new
      [AceTimeTools](https://github.com/bxparks/AceTimeTools) repo.
    * Move `./tests/validation` directory into new
      [AceTimeValidation](https://github.com/bxparks/AceTimeValidation) repo.
        * Update `.github/workflows/validation.yml` to use AceTimeValidation
          instead of `./tests/validation`.
    * This is a maintenance release. No changes to the core library code.
* 1.7.3 (2021-08-25, TZDB 2021a)
    * Fix numerous broken links in documents moved to `docs/*.md` in an earlier
      refactoring.
    * Add experimental `DS3231Module` class that uses `AceWire` library.
    * This is a maintenance release before some major refactoring. No changes to
      core library code.
* 1.7.2 (2021-06-02, TZDB 2021a)
    * **Bug Fix**: Add `ZonedDateTime::normalize()`, which must be called by
      the client code after calling a `ZonedDateTime` mutation function.
        * See [ZonedDateTime Normalization](docs/date_time_timezone.md#ZonedDateTimeNormalization).
        * Increases flash usage by 222 bytes by making this single call on an
          AVR unfortunately.
    * Migrate `PrintStr::getCstr()` in AceCommon <=1.4.4 to the shorter
      `PrintStr::cstr()` in AceCommon >= 1.4.5.
    * Migrate to AceRoutine v1.3.1, which changes
      `AceRoutine::coroutineMillis()` into non-virtual.
    * Change `SystemClock` to instantiate from `SystemClockTemplate`, which
      allows `SystemClock::clockMillis()` to also become non-virtual. Saves
      20-40 bytes of flash. No discernible changes in CPU time.
* 1.7.1 (2021-04-02, TZDB 2021a)
    * Simplify calculation of `SystemClock::getSecondsSinceSyncAttempt()`
      and `SystemClock::getSecondsToSyncAttempt()`, which substantially
      simplifies the implementation of `SystemClockLoop` and
      `SystemClockCoroutine`.
    * Update `tests/auniter.ini` to be consistent with my other libraries. Add
      entries for `env:teensy32` (which had been supported for a while) and
      `env:mkr1000` (which I just received).
* 1.7 (2021-03-24, TZ DB version 2021a)
    * AceTime now has a
      [GitHub Discussion](https://github.com/bxparks/AceTime/discussions).
        * Use that for general questions and discussions.
        * Reserve [GitHub Issues](https://github.com/bxparks/AceTime/issues)
          for bugs and feature requests.
    * Add `AceTimeTools/compare_noda` to compare Noda Time against AceTime.
        * Add `--nzd_file` flag to `compare_noda` to allow custom NodaZoneData
          files.
        * Run the Noda Time `TzdbCompiler` manually to generate custom
          `tzdata$(TZ_VERSION).nzd` for the specific TZDB version specified in
          the Makefile.
        * Add `tests/validation/tests/NodaBasicTest` which matches AceTime
          completely from year 2000 until 2050.
        * Add  `tests/validation/ExtendedNodaTest` which maches AceTime
          completely from year 1974 until 2050.
        * Identical results to `HinnantBasicTest` and `HinnantExtendedTest`.
    * Add `ace_time::clock::Stm32F1Clock` and `ace_time::hw::Stm32F1Rtc`
        * Specialized classes for the STM32F1 chip, particularly the Blue Pill
          board, using the `LSE_CLOCK` (low speed external clock).
            * Blue Pill already includes the external 32.768 kHz
              crystal on pins C14 and C15.
        * Alternative to the `ace_time::clock::StmRtcClock` class, which uses
          the generic `STM32RTC` library, which does not fully work on the
          STM32F1.
            * `STM32RTC` forgets the date fields upon power reset, preserving
              only the time fields.
        * These classes write directly into the 32-bit RTC register on the F1,
          allowing AceTime to preserve both date fields and time fields.
    * Massive refactoring of `USER_GUIDE.md` and `README.md` for clarity
        * Extract subsections of `USER_GUIDE.md` into separate docs,
          making `USER_GUIDE.md` shorter and hopefully more digestable.
        * Rename most of `USER_GUIDE.md` into `docs/date_time_timezone.md`.
        * Extract `Clock` classes into `docs/clock_system_clock.md`
        * Extract Installation into `docs/installation.md`.
        * Extract Validation and Testing into `docs/validation.md`.
        * Extract Comparisons into `docs/comparisons.md`.
        * Add documentation for `StmRtcClock` and `Stm32F1Clock`.
    * Remove virtual destructor from `ace_time::clock::Clock` class.
        * Saves 618 bytes of flash on 8-bit AVR processors, 328 bytes on SAMD21,
          but only 50-60 bytes on other 32-bit processors.
    * Finish a working implementation of `acetz.py`.
        * Includes support for `fold`.
        * Create `AcetzBasicTest` and `AcetzExtendedTest` and verify all zones
          validate.
    * Time zone short names are printed with spaces instead of underscore.
        * Various `printShortNameTo()` and `printShortTo()` methods now print
          the short names with the underscore replaced with a space. Example,
          instead of "Los_Angeles", it is now "Los Angeles".
        * It seems to be more reasonable for most time zones.
        * The time zone full name continues to print the entire canonical
          timezone identifier, e.g. "America/Los_Angeles".
        * Applications that need finer control will have to provide their own
          rendering logic.
    * `SystemClock`
        * Fix `SystemClock::forceSync()` that crashes if the referenceClock is
          null.
            * Used mostly for debugging and testing, so I doubt anyone ran into
              this.
        * Add methods to retrieve the sync status of `SystemClock`.
            * `getSecondsSinceSyncAttempt()`
            * `getSecondsToSyncAttempt()`
            * `getClockSlew()
            * `getSyncStatusCode()`
            * See [System Clock
              Status](docs/clock_system_clock.md#SystemClockStatus)
              for details.
        * **Potentially Breaking**: Move various internal constants in
          `SystemClockLoop` and `SystemClockCoroutine` to `private`.
            * Examples `kStatusReady`, kStatusSent`, `kStatusOk`k
            * These were all related to the internal finite state machine which
              should not have been exposed publically.
            * The sync status of `SystemClock` is now exposed through
              documented public methods described above.
    * `NtpClock`
        * Add warning that calling `analogRead()` too often (e.g. using buttons
          on a resistor ladder using AceButton library) on ESP8266 causes the
          WiFi connection to drop after 5-10 seconds.
* 1.6 (2021-02-17, TZ DB version 2021a)
    * Remove `TimeZone::kTypeBasicManaged` and `TimeZone::kTypeExtendedManaged`
      and merge them into just regular `TimeZone::kTypeBasic` and
      `TimeZone::kTypeExtended`.
        * Significantly simplifies the implementation of `TimeZone`.
        * `TimeZone` no longer holds a reference to a `ZoneProcessorCache`, it
          holds only a reference to `ZoneProcessor`.
        * The binding of `TimeZone` to its `BasicZoneProcessor` or
          `ExtendedZoneProcessor` now happens early, inside the
          `BasicZoneManager` or the `ExtendedZoneManager`, instead of delaying
          it to various methods inside the `TimeZone` through the
          `ZoneProcessorCache`.
        * This change should be invisible to library clients.
    * Large internal refactoring of ZoneProcessor, no external change
        * Fully templatize `BasicZoneProcessor` into
          `BasicZoneProcessorTemplate`, and `ExtendedZoneProcessor` to
          `ExtendedZoneProcessorTemplate`.
        * Remove sentinel static `ZoneEra` anchor record which prevented
          easy templatization.
        * Remove direct dependency to `const ZoneInfo*`, replacing it with
          generic `uintptr_t zoneKey`.
        * Insert `BrokerFactory` indirection layer to provide mapping from a
          generic `uintptr_t zoneKey` to the corresponding `ZoneInfoBroker`.
        * Templatized classes now depend only on their respective
          `Zone*Broker` classes.
        * This change should be invisible to library clients.
    * Fix stale `ZoneProcessor` binding to `TimeZone`.
        * A dereferenced `nullptr` could crash the program if
          `TimeZone::toTimeZoneData()` was called immediately after calling the
          `TimeZone::forZoneInfo()` factory method.
        * Some accessor methods in `TimeZone` (`getZoneId()`, `printTo()`,
          `printShortTo()`) could return incorrect values if the number of
          unique TimeZones used by an application is greater than the cache
          `SIZE` template parameter given to the `ZoneManager`.
            * The problem occurs because the `ZoneProcessorCache` will rebind
              a previously allocated `ZoneProcessor` to another `TimeZone` when
              it runs out of available processors in the cache.
    * **Unlikely Breaking Change**: Move `ZoneRegistrar.h` into `internal/`.
        * Rename `BasicZoneRegistrar` to `basic::ZoneRegistrar`.
        * Rename `ExtendedZoneRegistrar` to `extended::ZoneRegistrar`.
        * The class is an implementation detail which is used only by
          `BasicZoneManager` and `ExtendedZoneManager`. It was not exposed to
          the end user and should not cause any breaking changes.
    * Add support for Thin Links using optional `linkRegistry[]` parameter in
      the constructors of `BasicZoneManager` and `ExtendedZoneManager`.
        * The `zonedb/zone_registry.h` and `zonedbx/zone_registry.h` files
          now contain a `kLinkRegistrySize` and a `LinkEntry kLinkRegistry[]`
          array. Each record in the array contains a mapping of `linkId` to its
          `zoneId`.
        * The `ZoneManager::createForZoneId()` method will search the Thin Link
          registry if a `zoneId` is not found in the Zone registry.
        * See [Zones and Links](USER_GUIDE.md#ZonesAndLinks) section in the
          [USER_GUIDE.md](USER_GUIDE.md).
    * **Breaking Change**: Rename `ZoneManager::registrySize()` to
      `zoneRegistrySize()`.
        * Add `ZoneManager::linkRegistrySize()` method.
        * A `ZoneManager` can now hold 2 different registries: the Zone (and Fat
          Link) registry, and the Thin Link registry. So we need to
          distinguish between the 2 registries.
        * See the [Default Registries](USER_GUIDE.md#DefaultRegistries) section
          in the [USER_GUIDE.md](USER_GUIDE.md) for an explanation of the Zone
          and Link registries.
* 1.5 (2021-01-26, TZDB 2021a)
    * Use binary search for both `ZoneManager::createForZoneName()` and
      `ZoneManager::createForZoneId()`.
        * Previously, the `zone_registry.cpp` was sorted by zoneName, so only
          the `createForZoneName()` could use the binary search. The new
          solution sorts the `zone_registry.cpp` entries by `zoneId` instead of
          `zoneName`. The `createForZoneId()` can use the binary search
          algorith.
        * The `createForZoneName()` can also use the binary search because
          the `zoneName` is converted dynamically to its `zoneId` using the same
          djb2 hash algorithm used by the `tzcompiler.py`. If there is a match,
          a final verification against the exact `zoneName` is performed to make
          sure that there was no hash collision.
        * Updated `AutoBenchmark.ino` to determine that a binary search on the
          266 zones in `zonedb/zone_registry.cpp` is 9-10X faster (on average)
          than a linear search through the same list. (Linear search takes ~190
          iterations; binary search takes ~9 iterations.)
    * Upgrade Link entries to be Fat Links".
        * Links become essentially identical to Zone entries, with references to
          the same underlying `ZoneEra` records.
        * Add `kZoneAndLinkRegistry[]` array in `zone_registry.h` that contains
          all Links as well as Zones.
        * Add "Zones and Links" section in `USER_GUIDE.md`.
    * Implement zoneName compression using `ace_common::KString`.
        * Saves about 1500-2300 bytes for basic `zonedb` info files, and
          2500-3400 bytes for extended `zonedbx` info files.
    * **Potentially Breaking Change**: Remove `transitionBufSize` from
      `ZoneInfo` struct, and migrate to `kZoneBufSize{xxx}` constants in the
      `zone_infos.h` files.
        * This was used only in validation tests under `tests/validation` and
          only for `Extended{xxx}` tests. Saves 1 byte per Zone on 8-bit
          processors, but none on 32-bit processors due to 4-byte alignment.
        * This should have no impact on client code since this field was used
          only for validation testing.
    * **API Breaking Change**: Replace `BasicZone::name()` and `shortName()`
      with `printNameTo()` and `printShortNameTo()`. Same with
      `ExtendedZone::name()` and `shortName()`, replaced with `printNameTo()`
      and `printShortNameTo()`.
        * After implementing zoneName compression, it was no longer possible to
          return a simple pointer to the `name` and `shortName` without using
          static memory buffers.
        * I expect almost no one to be using the `BasicZone` and `ExtendedZone`
          classes, since they are mostly useful for internal algorithms.
        * Client code that needs the old functionality can use
          `BasicZone::printNameTo(Print&)`,
          `BasicZone::printShortNameTo(Print&)` (similarly for `ExtendedZone`)
          to print to a `ace_common::PrintStr<>` object, then extract the
          c-string using `PrintStr::getCstr()`.
    * Update UnixHostDuino 0.4 to EpoxyDuino 0.5.
    * Explicitly blacklist megaAVR boards, and SAMD21 boards using
      `arduino:samd` Core >= 1.8.10.
        * This allows a helpful message to be shown to the user, instead of the
          pages and pages of compiler errors.
    * Update TZ Database to 2021a.
        * https://mm.icann.org/pipermail/tz-announce/2021-January/000065.html
        * "South Sudan changes from +03 to +02 on 2021-02-01 at 00:00."
    * Officially support STM32 and STM32duino after testing on STM32 Blue Pill.
* 1.4.1 (2020-12-30, TZDB version 2020f for real)
    * Actually update `src/ace_time/zonedb` and `src/ace_time/zonedbx`
      zone info files to 2020f. Oops.
* 1.4 (2020-12-30, TZ DB version 2020f)
    * Add entry for `ManualZoneManager` in
      [examples/MemoryBenchmark](examples/MemoryBenchmark). It seems to need
      between 0'ish to 250 bytes of flash.
    * Add support for creating and handling a `TimePeriod` error object.
        * Add `kMaxPeriodSeconds` and `kInvalidPeriodSeconds` constants.
        * Add `forError()` factory method.
        * Add error checking to `toSeconds()` and `TimePeriod(seconds)`
          constructor.
        * Printing an error object prints `<Invalid TimePeriod>`.
    * Add support for the STM32RTC clock on an STM32 through the
      `ace_time::clock::StmRtcClock` class.
        * Currently **experimental** and untested.
        * I do not have any STM32 boards right now, so I cannot test this code.
        * See [#39](https://github.com/bxparks/AceTime/pull/39) for details.
        * Thanks to Anatoli Arkhipenko (arkhipenko@).
    * Add convenience factory methods for creating manual `TimeZone` objects.
      Saves a lot of typing by avoiding the `TimeOffset` objects:
        * `TimeZone::forHours()`
        * `TimeZone::forMinutes()`
        * `TimeZone::forHourMinute()`
    * Fix incorrect `kTypeXxx` constants in `ZoneManager.h`. Fortunately, the
      numerical values overlapped perfectly, so didn't cause any bugs in actual
      code.
    * `USER_GUIDE.md`
        * Add documentation about accessing the meta information about the
          `zonedb` and `zonedbx` databases:
            * `zonedb::kTzDatabaseVersion`
            * `zonedb::kZoneContext.startYear`
            * `zonedb::kZoneContext.untilYear`
            * `zonedbx::kTzDatabaseVersion`
            * `zonedbx::kZoneContext.startYear`
            * `zonedbx::kZoneContext.untilYear`
        * Add documentation that the `ZonedDateTime` must always be within
          `startYear` and `untilYear`. An error object will be returned outside
          of that range.
    * Update TZ Database from 2020d to version 2020f
        * 2020e
            * https://mm.icann.org/pipermail/tz-announce/2020-December/000063.html
            * "Volgograd switches to Moscow time on 2020-12-27 at 02:00."
        * 2020f
            * https://mm.icann.org/pipermail/tz-announce/2020-December/000064.html
            * "'make rearguard_tarballs' no longer generates a bad rearguard.zi,
              fixing a 2020e bug.  (Problem reported by Deborah Goldsmith.)"
        * AceTime skips 2020e
    * Update `examples/AutoBenchmark` to allow auto-generation of ASCII tables,
      which allows auto-generation of the `README.md` file. Update CPU
      benchmarks for v1.4 from v0.8, since it is much easier to update these
      numbers now. No significant performance change from v0.8.
    * Huge amounts of Python `tools` refactoring
        * Convert all remaining `%` string formatting to f-strings.
        * Convert all internal camelCase dictionary keys to snake_case for
          consistency.
        * Finish adding typing info to pass strict mypy checking.
        * Centralize most typing info into `data_types/at_types.py`.
        * Move various global constants into `data_types/at_types.py`.
        * Migrate most Arduino specific transformations into `artransformer.py`.
        * Move offsetCode, deltaCode, atTimeCode, untilTimeCode, rulesDeltaCode
          calculations and various bit-packing rules (e.g. `at_time_modifier`,
          `ntil_time_modifier`) into `artransformer.py` instead of the
          `argenerator.py`.
        * Include general and platform specific transformations in
          the JSON (`zonedb.json` or `zonedbx.json`) files.
        * Make `argenerator.py` use only the JSON output instead of making
          its own transformations.
        * Produce both `LettersMap` and `LettersPerPolicy` in the JSON file.
        * Unify `CommentsCollection` into `CommentsMap` using an `Iterable`
          in `Dict[str, Iterable[str]]`.
        * Unify all transformer results into `TransformerResult`.
        * Check hash collisions for Link names, in addition to Zone names.
        * Rename test data generator executables to `generate_data.*` or some
          variations of it, to avoid confusion with unit tests which are often
          named `test_xxx.py`.
        * Add `--input_dir` flag to `generate_data.cpp` to allow the TZ database
          directory to be specified.
        * Add `--ignore_buf_size_too_big` to workaround a mismatch between
          the estimated Transition buffer size calculated by `bufestimator.py`
          and the actual buffer size required by `ExtendedZoneProcessor.cpp`.
        * Add `--skip_checkout` flag to `tzcompiler.py` to allow local
          modifications of the TZ database files to be used for code generation.
        * Add `--delta_granularity` flag to `transformer.py` to decouple it
          from `--offset_granularity`, so that the `SAVE` and `RULES`
          granularity can be controlled independently from the `STDOFF`
          granularity.  The `--until_at_granularity` continues to control `AT`
          and `UNTIL`.
        * Make output of `zinfo.py --debug` to be more readable.
        * Remove `--generate_zone_strings` from `tzcompiler.py` which removes
          the ability to create `zone_strings.{h,cpp}`. The storage and
          optimization of strings are implementation details which seem to be
          better handled later in the pipeline.
        * Increase the range of `zonedbpy` database from the default
          `[2000,2050)` to `[1974, 2050)` to allow `zinfo.py` to handle a larger
          range of queries.
        * Merge `--action` and `--language` flags for `tzcompiler.py`; only
          `--language` flag needed right now.
        * Rename `RulesMap` to `PoliciesMap`, `rules_map` to `policies_map`,
          `rule_name` to `policy_name` etc. For consistency with `ZonesMap`,
          `zones_map`, and `zone_name`.
        * Add `ZoneId` hashes to JSON output files.
* 1.3 (2020-11-30, TZ DB version 2020d)
    * Minor tweaks to silence clang++ warnings.
    * Create new `ZoneManager` interface (pure virtual) which is now the
      non-templatized parent to both `BasicZoneManager` and
      `ExtendedZoneManager`. Allows `ZoneManager` to be passed around
      poloymorphically as a pointer or reference.
    * Fix broken `ZoneManager::indexForZoneName()` and
      `ZoneManager::indexForZoneId()` caused by incorrect implementations in
      `BasicZoneRegistrar` and `ExtendedZoneRegistrar`.
    * Generate compile-time zoneIds for all zones in the form of
      `zonedb::kZoneId{Zone_Name}` and `zonedbx::kZoneId{Zone_Name}` (e.g.
      `zonedb::kZoneIdAmerica_Los_Angeles`). Can be given directly to
      `ZoneManager::createForZoneId()`.
    * Add constructors to `TimeZoneData` to allow initializers to set
      union members. Useful for initializing arrays of `TimeZoneData`.
    * Add `ManualZoneManager` implementation of `ZoneManager` which implements
      only `createForTimeZoneData()`. Useful in applications which support only
      `TimeZone::kTypeManual` (fixed std and dst offsets) due to memory
      constaints.
    * Add documentation of `TimeZoneData`, `TimeZone::toTimeZoneData()`, and
      `ZoneManager::createFromTimeZoneData()` to `USER_GUIDE.md`. Looks like I
      added the class in v0.5 but forgot to document it.
    * Implement `LocalDateTime::compareTo()` using only its components instead
      of internally converting to epochSeconds. Not all `LocalDateTime` can be
      represented by an epochSeconds, so this change makes the algorithm more
      robust. The semantics of the method should remain unchanged.
    * Update the doxygen docs of the `compareTo()` methods of `LocalDateTime`,
      `LocalTime`, `LocalDate`, `OffsetDateTime` and `ZonedDateTime` to clarify
      the semantics of those operations.
* 1.2.1 (2020-11-12, TZ DB version 2020d)
    * No functional change in this release. Mostly documentation.
    * Update `examples/MemoryBenchmark` numbers from v0.8 to v1.2 with
      new auto-generator scripts.
    * Add Table of Contents to `USER_GUIDE.md` to help navigate the long
      document.
* 1.2 (2020-10-31, TZ DB version 2020d)
    * **Potentially Breaking**: AceTime library now depends on the AceCommon
      library (https://github.com/bxparks/AceCommon) to avoid having to maintain
      multiple copies of various utility functions and classes. The API for most
      (if not all) public classes have not changed. A number of internal helper
      classes have moved to the AceCommon library. If you happen to directly use
      some of these, you need to use the AceCommon library instead.
        * Add dependency to AceCommon to all Makefiles.
        * Add a `depends` attribute to `library.properties`.
    * Replace various utlity functions and class with those from AceCommon:
        * `class TimingStats`
        * `incrementMod()`, `incrementModOffset()`, `printPad2To()`,
          `printPad3To()`
        * `strcmp_PP()`
        * `strchr_P()`, `strrchr_P()` for ESP8266 and ESP32
        * `PrintStr`
    * Move `common/CrcEeprom.h` to AceUtils
      (https://github.com/bxparks/AceUtils) library.
* 1.1.2 (2020-10-25, TZ DB version 2020d)
    * Move examples/WorldClock, examples/OneZoneClock and
      examples/CommandLineClock to a new repo
      (https://github.com/bxparks/clocks).
    * Update `src/ace_time/zonedb` and `src/ace_time/zonedbx` to TZDB 2020d
      (https://mm.icann.org/pipermail/tz-announce/2020-October/000062.html).
        * "Palestine ends DST earlier than predicted, on 2020-10-24."
* 1.1.1 (2020-10-18, TZ DB version 2020c)
    * Add documentation for using the `PrintStr<N>` class from the AceUtils
      library (https://github.com:bxparks/AceUtils) on the various `printTo()`
      methods. The string content can be retrieved using the
      `PrintStr::getCstr()` method which returns a normal `const char*`
      C-string. The `PrintStr<N>` class replaces an earlier, unreleased version
      that was called `CstrPrint<N>`.
    * Add 'Validation Tests' GitHub workflow for running
      `tests/validation/*Test`.
    * Create `blacklist.json` file for each `compare_xxx` tools, to disable
      validation checks for DST or abbreviations due to bugs in the 3rd party
      libraries.
    * Add `ValidationScope` enum to provide better control over whether the
      DST or abbrev fields are validated.
    * Print better diagnostic messages when `tests/validation` fails in
      `BasicTransitionTest` and `ExtendedTransitionTest` classes.
    * Upgrade target version numbers of 3rd party libraries used for
      tests/validation: pytz from 2019.3 to 2020.1, JDK 11.0.6 to 11.0.8.
    * Upgrade to TZDB 2020c
      (https://mm.icann.org/pipermail/tz-announce/2020-October/000060.html)
        * "Fiji starts DST later than usual, on 2020-12-20."
    * Restrict GitHub Actions workflow to run just HinnantBasicTest and
      HinnantExtendedTest, because the other Python and Java tests break
      every time a new TZDB version comes out.
    * Add `DEVELOPER.md` file containing notes mostly for myself.
* 1.1 (2020-04-25, TZ DB version 2020a)
    * Fix broken links in `README.md`.
    * Fix typos in `USER_GUIDE.md` and update its version number to 1.0 as it
      should have been back in October.
    * Massive refactor of `./tools` processing pipeline and update
      `validation/tests`.
        * Add mypy strict type checking for Python scripts under `tools`.
        * Funnel `validation*.{h,cpp}` code generation through a single program
          using a `validation_data.json` intermediate file.
        * Funnel processing of TZDB output from `transformer.py` into a single
          `tzdbcollector.py` which can produce a `tzdb.json` output.
        * Separate `validator.py` processing into a distinct section.
    * Add validation tests against Python `dateutil` library (similar to
      `pytz`).
    * Update TZ Database version to 2020a that was released on 2020-04-23.
* 1.0 (2019-10-02, TZ DB version 2019c)
    * Add initial support for GitHub actions to implement continuous integration
      using the unit tests that run under UnitHostDuino.
    * Allow NtpClock to use an existing WiFi connection. Add
      `examples/HelloNtpClock/` to demonstrate this. (#24, thanks
      @denis-stepanov).
    * Fix compiler warning about duplicate `FPSTR()` macro for ESP32 Core
      version >=1.0.3.
    * Generate the zonedb files for the various `validation/*Test` integration
      tests on-demand, instead of using the zonedb files checked into
      `src/ace_time/zonedb[x]`. This allows us to match the version of the TZ
      Database used by AceTime to the version used by Java 11, pytz, and Hinnant
      Date, independently of the version that is generated into
      `src/ace_time/zonedb[x]`.
    * Update `src/ace_time/zonedb[x]` files to TZ version 2019c.
    * Graduate to version 1.0.
* 0.8.1 (2019-08-26, TZ DB version 2019b)
    * Update `SystemClockCoroutine` to be compatible with
      `COROUTINE_DELAY_SECONDS()` API changed in AceRoutine v0.3.
    * Fix typos and grammar errors in `USER_GUIDE.md` and `README.md`.
    * Remove `YearMonth` abstraction in `BasicZoneProcessor`, saving 12 bytes
      of flash in WorldClock.
* 0.8 (2019-08-19, TZ DB version 2019b)
    * Handle `Fri<=1` correctly in various python scripts. (#17)
    * Improve resolution of zonedb files and ZoneProcessor classes. (#18)
        * Both BasicZoneProcessor and ExtendedZoneProcessor support 1-minute
          resolutions for the AT and UNTIL fields.
        * BasicZoneProcessor (and zonedb files) support a 15-minute resolution
          for both STDOFF and DST offset fields.
        * ExtendedZoneProcessor (and zonedbx files) support one-minute
          resolution for STDOFF field and 15-minute resolution for DST offset
          (with a range of -01:00 to 02:45). (4 bits of the `deltaCode`
          field were given to the `offsetCode` field to give it the
          1-minute resolution.)
        * Regenerate zonedbx using 1-minute resolution.
    * Fix broken BasicZoneProcessor on some timezones between 1975 and
      2000. Did not handle transitions from fixed ZoneEra (RULES='-') to named
      ZoneEra (RULES=reference) or vise versa. Verified against pytz and
      Hinnant date from 1975 to 2050.
* 0.7.2 (2019-08-14, TZ DB version 2019b)
    * Support timezones whose FORMAT contains a '/' with a fixed RULES column.
      Seems to make BasicZoneProcessor slightly smaller (20-80 bytes) and
      ExtendedZoneProcessor slightly bigger (50-100 bytes).
    * Split `--granularity` into `--until_at_granularity` and
      `offset_granularity`. Current zonedb files use values of 60 and 900
      respectively.
* 0.7.1 (2019-08-13, TZ DB version 2019b)
    * Replace `TimeZone::printAbbrevTo()` with more flexible and useful
      `TimeZone::getAbbrev()`.
* 0.7 (2019-08-13, TZ DB version 2019b)
    * Change TimeZoneData to store mStdOffset and mDstOffset in units of
      one minute (instead of 15-minute increments, "code") in the off chance
      that the library supports timezones with one-minute shifts in the future.
    * Implement TimeOffset using 2 bytes (`int16_t`) instead of one byte
      (`int8_t`) to give it a resolution of one minute instead of 15 minutes.
    * Generate zoneinfo files containing AT and UNTIL timestamps with
      one-minute resolution (instead of 15-minute resolution). ZoneInfo files
      (`zonedb/` and `zonedbx/`) remain identical in size. Flash memory
      consumption usually increases by 130 to 500 bytes, but sometimes decreases
      by 50-100 bytes. Timezones whose DST transitions occur at 00:01
      (America/Goose_Bay, America/Moncton, America/St_Johns, Asia/Gaza,
      Asia/Hebron) no longer truncate to 00:00.
    * Rename `TimeOffset::forHour()` to `forHours()` for consistency with
      `forMinutes()`.
    * Make `ExtendedZoneProcessor` more memory efficient for 32-bit processors
      by packing internal fields to 4-byte boundaries.
    * Integrate C++11/14/17
      [Hinnant Date](https://github.com/HowardHinnant/date) library by
      creating additional `tests/validation` tests.
    * Upgrade `zonedb` and `zonedbx` zoneinfo files to version 2019b,
      after validating against the Hinnant date library.
    * Upgrade to `pytz` version 2019.2 to pickup TZ Database 2019b.
* 0.6.1 (2019-08-07, TZ DB version 2019a)
    * Create a second Jenkins continuous build pipeline file
      `tests/JenskinfileUnitHost` to use UnitHostDuino to run the unit tests
      natively on Linux. The entire set of unit tests builds and runs in 20
      seconds under UnixHostduino, compared to about 8 minutes for the single
      Nano environemnt, and 32 minutes against 4 boards (Nano, ESP8266, ESP32,
      SAMD21).
    * Fix Doxygen PREPROCESSOR so that it picks up classes which are enabled
      only on some environments (e.g. ESP8266, ESP32).
    * Add circuit schematics to OneZoneClock and WorldClock examples.
    * Simplify logging::printf() used internally for debugging.
    * No functional change from 0.6.
* 0.6 (2019-08-02, TZ DB version 2019a)
    * Update tests to use `UnixHostDuino`.
    * Fix broken restore functionality in `CommandLineClock`. Make it work
      on Unix using UnixHostDuino. Make it work on ESP8266 and ESP32 again.
    * Update flash memory consumption numbers in `zonedb/zone_infos.cpp` and
      `zonedbx/zone_infos.cpp`.
    * Lift `ace_time::common::DateStrings` to just `ace_time::DateStrings`
      because it was the only data/time class in the `common::` namespace so
      it seemed inconsistent and out of place.
    * **Breaking Change** Large refactoring and simplification of the
      `ace_time::clock` classes. Merged `TimeKeeper` and `TimeProvider` into a
      single `Clock` class hierarcy. Merged `SystemClockSyncLoop` (separate
      class) into `SystemClockLoop` (subclass of `SystemClock`) and
      `SystemClockSyncCoroutine` (separate class) into `SystemClockCoroutine`
      (subclass of `SystemClock` with mixin of `ace_routine::Coroutine`). Merged
      `keepAlive()` into `::loop()` and `::runCoroutine()` methods, so we don't
      need to worry about it separately anymore. Made `SystemClockLoop` use the
      non-blocking methods of `Clock`, making it as responsive as
      `SystemClockCoroutine`.
    * Add `UnixClock.h` which provides access to the internal Unix clock
      when using UnixHostDuino.
* 0.5.2 (2019-07-29, TZ DB Version 2019a)
    * Create `HelloZoneManager` and add it to the `README.md`.
    * Recommend using "Arduino MKR ZERO" board or "SparkFun SAMD21 Mini
      Breakout" board for the "SAMD21 M0 Mini" boards.
    * Pack `basic::ZoneInfo`, `extended:ZoneInfo` and related structs tighter on
      32-bit processors, saving 2.5kB on the Basic zoneinfo files and 5.6kB on
      Extended zoneinfo files.
    * Pack `basic::Transition` and `extended::Transition` tighter on 32-bit
      processors, saving 20-32 bytes on `BasicZoneProcessor` and
      `ExtendedZoneProcessor`.
    * Test and support ATmega2560 AVR processor.
    * Replace all uses of `Serial` with `SERIAL_PORT_MONITOR` for compatibility
      with boards (e.g. SAMD boards) which use `SerialUSB` as the serial
      monitor.
* 0.5.1 (2019-07-24, TZ DB version 2019a, beta)
    * Add documentation about the ZoneManager into `USER_GUIDE.md`.
    * Move `DateStrings` string pointers into PROGMEM, saving 42 bytes of RAM.
    * Use `SERIAL_PORT_MONITOR` instead of `Serial` everywhere for portability
      across different Arduino boards.
    * Support SAMD21 boards except for EEPROM which SAMD21 does not support.
* 0.5 (2019-07-21, TZ DB version 2019a, beta)
    * Remove over-engineered `SystemClockHeartbeatLoop` and
      `SystemClockHeartbeatLoop` and replace with just a call to
      `SystemClock::keepAlive()`.
    * Remove overly complex `ManualZoneProcessor` and merge most of its
      functionality directly into the `TimeZone` using `kTypeManual`. We lose
      the manual abbreviations provided by `ManualZoneProcessor` but the
      simplification of using just the `TimeZone` object without an extra object
      seems worth it.
    * Add a stable `zoneId` to `ZoneInfo` that identifies a zone. It is
      formed using a hash of the fully qualified zone `name`. The
      `tzcompiler.py` generator script will detect hash collisions and create an
      alternate hash.
    * Rename old `ZoneManager` as the `ZoneRegistrar`, and
      repurpose `ZoneManager` as the `TimeZone` factory, which keeps an internal
      cache of `ZoneProcessor`. `TimeZone` objects can be dynamically bound to
      `ZoneProcessor` objects using `createForZoneInfo()`,
      `createForZoneName()`, `createForZoneId().
    * Add `TimeZoneData` data struct to allow serialization of a TimeZone object
      as a zoneId so that it can be reconstructed using
      `ZoneManger::createForTimeZoneData()`.
    * Rename `ZoneSpecifier` to `ZoneProcessor` to describe its functionality
      better. `ZoneInfo` is now passed directly into the TimeZone object using
      the `TimeZone::forZoneInfo()` factory method, with the `ZoneProcessor`
      acting as a helper object.
* 0.4 (2019-07-09, TZ DB version 2019a, beta)
    * Support the less-than-or-equal syntax `{dayOfWeek}<={dayOfMonth}`
      appearing in version 2019b of the TZ Database which contains `Rule Zion,
      2005 to 20012, IN Apr, ON Fri<=1`.
    * Add BasicZoneManager and ExtendedZoneManager to retrieve ZoneInfo
      from TZ Database string identifier (e.g. "America/Los_Angeles").
    * Add configuration options (`ACE_TIME_USE_BASIC_PROGMEM` and
      `ACE_TIME_USE_EXTENDED_PROGMEM`) to place zoneinfo
      files into PROGMEM to save static RAM.
* 0.3.1 (2019-06-30, TZ DB version 2019a, beta)
    * Add copyright notices on source files.
    * Fix typos and formatting of `README.md` and `USER_GUIDE.md`.
    * No functional change from 0.3.
* 0.3 (2019-06-28, TZ DB version 2019a, beta)
    * Support `Link` entries from TZ Database files as C++ references to
      corresponding `Zone` entries.
    * Add `backward` and `etctera` files from TZ Database to the tzcompiler.py
      processing. `ExtendedZoneProcessor` now supports *every* Zone and Link
      entry in the TZ Database (except those in `backzone` and `systemv`).
    * Add better zone and link name normalization. Convert `+` into `_PLUS_`,
      all other non-alphanumeric (0-9a-zA-Z_) converted to underscore `_`.
    * Move validation unit tests into separate `tests/validation` directory.
      Use Makefiles to generate `validation_data.*` files dynamically at compile
      time.
* 0.2 (2019-06-26, TZ DB version 2019a, alpha)
    * Reduce flash memory size of WorldClock by removing extra font.
    * Split `USER_GUIDE.md` from `README.md`.
    * Rename `ace_time::provider` to `ace_time::clock` and rename
      `SystemTimeProvider` to `SystemClock`.
    * Add `HelloSystemClock` example code.
    * Add `isValidYear()` into various `forComponents()` methods to check
      int8_t range of year component.
    * Rename `DateStrings::weekDay*()` methods to `dayOfWeek*()` for
      consistency.
    * Change `ZonedDateTime::printTo()` format to match Java Time format.
    * Remove `friend` declarations not related to unit tests.
    * Remove redundant definitions of `kInvalidEpochSeconds`, standardize on
      `LocalDate::kInvalidEpochSeconds`.
    * Make `timeOffset` a required parameter for constructors and factory
      methods `OffsetDateTime` instead of defaulting to `TimeOffset()`.
    * Make `timeZone` a required parameter in constructors and factory methods
      of `ZonedDateTime`.
    * Fix `BasicZoneProcessor::getOffsetDateTime()` to handle gaps and overlaps
      in a reasonable way, and perform some amount of normalization.
* 0.1 (2019-06-15, TZ DB version 2019a, alpha)
    * Initial release on GitHub to establish a reference point.
    * Upgraded to TZ Database version 2019a.
* (2018-08-20)
    * Start of library in private repo.
