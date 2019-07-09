# Changelog

* Unreleased
    * Support the less-than-or-equal syntax `{dayOfWeek}<={dayOfMonth}`
      appearing in version 2019b of the TZ Database which contains `Rule Zion,
      2005 to 20012, IN Apr, ON Fri<=1`.
    * Add BasicZoneManager and ExtendedZoneManager to retrieve ZoneInfo
      from TZ Database string identifier (e.g. "America/Los_Angeles").
    * Add compile-time option (`ACE_TIME_USE_BASIC_PROGMEM`) to put zoneinfo
      files for BasicZoneSpecifier into PROGMEM.
* 0.3.1 (2019-06-30, TZ DB version 2019a, beta)
    * Add copyright notices on source files.
    * Fix typos and formatting of `README.md` and `USER_GUIDE.md`.
    * No functional change from 0.3.
* 0.3 (2019-06-28, TZ DB version 2019a, beta)
    * Support `Link` entries from TZ Database files as C++ references to
      corresponding `Zone` entries.
    * Add `backward` and `etctera` files from TZ Database to the tzcompiler.py
      processing. `ExtendedZoneSpecifier` now supports *every* Zone and Link
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
    * Fix `BasicZoneSpecifier::getOffsetDateTime()` to handle gaps and overlaps
      in a reasonable way, and perform some amount of normalization.
* 0.1 (2019-06-15, TZ DB version 2019a, alpha)
    * Initial release on GitHub to establish a reference point.
    * Upgraded to TZ Database version 2019a.
* (2018-08-20)
    * Start of library in private repo.
