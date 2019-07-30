#line 2 "UnixTimeTest.ino"

/**
 * @file UnixTimeTest.ino
 *
 * Quick and dirty validation of LocalDatEtime::toEpochSeconds() and
 * forEpochSeconds() against the "standard" <time.h> library provided by some
 * Arduino platforms. The standard C/Unix <time.h> is a big mess. See
 * http://www.catb.org/esr/time-programming/#_gmtime_3_and_localtime_3 and the
 * AVR <time.h> notes
 * https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html.
 *
 * None of the Arduino versions of <time.h> supports TZ Database timezones
 * (as far as I can tell).
 *
 *    * AVR
 *        * epoch starts at year 2000
 *        * epoch seconds is a uint32_t, not in32_t
 *        * provides mk_gmtime() as the inverse of gmtime()
 *    * SAMD (none)
 *    * ESP8266 - epoch starts at year 1970
 *        * no mk_gmtime() method, use mktime() instead
 *        * no actual inverse of gmtime(), use mktime()
 *          assuming local time zone is UTC
 *    * ESP32 - epoch starts at year 1970
 *        * no mk_gmtime() method, use mktime() instead
 *        * no actual inverse of gmtime(), use mktime()
 *          assuming local time zone is UTC
 *    * Teensy (none)
 *    * Linux & MacOS
 *        * provides timegm() as the inverse of gmtime()
 */

#if defined(AVR) || defined(ESP8266) || defined(ESP32) || defined(__linux__) \
    || defined(__APPLE__)
#include <time.h> // time.h
#endif

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

#if defined(AVR) || defined(ESP8266) || defined(ESP32) || defined(__linux__) \
    || defined(__APPLE__)

// --------------------------------------------------------------------------

test(UnixTimeTest, toEpochSeconds) {
  auto dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  acetime_t epochSeconds = dt.toEpochSeconds();

  struct tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_mday = 1;
  t.tm_mon = 0; // [Jan, Dec] = [0, 11] (sigh)
  t.tm_isdst = 0;
  t.tm_year = 2018 - 1900; // year since 1900 (!)

#if defined(AVR)
  //time_t avrSeconds = mk_gmtime(&t);
  time_t avrSeconds = mktime(&t);
  assertEqual(epochSeconds, (acetime_t) avrSeconds);
#elif defined(ESP8266) || defined(ESP32)
  time_t espSeconds = mktime(&t);
  assertEqual(epochSeconds,
    (acetime_t) (espSeconds - LocalDate::kSecondsSinceUnixEpoch));
#elif defined(__linux__) || defined(__APPLE__)
  time_t unixSeconds = timegm(&t);
  assertEqual(epochSeconds,
    (acetime_t) (unixSeconds - LocalDate::kSecondsSinceUnixEpoch));
#endif
}

test(UnixTimeTest, forEpochSeconds) {
  struct tm t;

  // 2029-12-31 23:59:59Z Monday
  acetime_t epochSeconds = 10958 * (acetime_t) 86400 - 1;
#if defined(AVR)
  time_t avrSeconds = (time_t) epochSeconds;
  gmtime_r(&avrSeconds, &t);
#elif defined(ESP8266) || defined(ESP32)
  time_t espSeconds = epochSeconds + LocalDate::kSecondsSinceUnixEpoch;
  gmtime_r(&espSeconds, &t);
#elif defined(__linux__) || defined(__APPLE__)
  time_t unixSeconds = epochSeconds + LocalDate::kSecondsSinceUnixEpoch;
  gmtime_r(&unixSeconds, &t);
#endif

  auto dt = LocalDateTime::forEpochSeconds(epochSeconds);
  assertEqual(dt.year(), (int16_t) (t.tm_year + 1900));
  assertEqual(dt.month(), t.tm_mon + 1);
  assertEqual(dt.day(), t.tm_mday);
  assertEqual(dt.hour(), t.tm_hour);
  assertEqual(dt.minute(), t.tm_min);
  assertEqual(dt.second(), t.tm_sec);
  assertEqual(dt.dayOfWeek(), (uint8_t) (t.tm_wday == 0 ? 7 : t.tm_wday));
}

#endif

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability to prevent garbage on SERIAL_PORT_MONITOR
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
