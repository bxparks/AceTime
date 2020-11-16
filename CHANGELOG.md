# Changelog

* Unreleased
    * Minor tweaks to silence clang++ warnings.
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
    * Move examples/WorldClock, examples/OledClock and examples/CommandLineClock
      to a new repo (https://github.com/bxparks/clocks).
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
    * Restrict GitHub Actions workflow to run just BasicHinnantDateTest and
      ExtendedHinnantDateTest, because the other Python and Java tests break
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
    * Add circuit schematics to OledClock and WorldClock examples.
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
