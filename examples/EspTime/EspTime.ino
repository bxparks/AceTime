/*
Use the built-in SNTP client on the ESP8266 and ESP32 platforms to configure
the C-library `time()` function to return the number of seconds since Unix epoch
(1970-01-01T00:00:00). The epochSeconds is converted to human readable date-time
strings in 4 ways:

1) UTC using old-school C-library functions, `gmtime_r()` and `struct tm`
2) UTC using ace_time::LocalDateTime
3) America/Los_Angeles using ace_time::ZonedDateTime
4) Europe/Paris using ace_time::ZonedDateTime

The SNTP client apparently performs automatic synchronization of the time()
function every 1h, but the only documentation for this that I can find is in
this example file:
https://github.com/esp8266/Arduino/tree/master/libraries/esp8266/examples/NTP-TZ-DST

The app should print something like this on the Serial console, updating every 5
seconds:

Connecting to WiFi......
Configuring SNTP..
2022-01-14T01:51:06 Friday (C lib)
2022-01-14T01:51:06 Friday (AceTime)
2022-01-13T17:51:06-08:00[America/Los_Angeles] Thursday
2022-01-14T02:51:06+01:00[Europe/Paris] Friday

2022-01-14T01:51:11 Friday (C lib)
2022-01-14T01:51:11 Friday (AceTime)
2022-01-13T17:51:11-08:00[America/Los_Angeles] Thursday
2022-01-14T02:51:11+01:00[Europe/Paris] Friday

2022-01-14T01:51:16 Friday (C lib)
2022-01-14T01:51:16 Friday (AceTime)
2022-01-13T17:51:16-08:00[America/Los_Angeles] Thursday
2022-01-14T02:51:16+01:00[Europe/Paris] Friday
*/

#include <Arduino.h>
#include <time.h> // gmtime_r()

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <AceTime.h>
using namespace ace_time;

#if ! defined(WIFI_SSID)
#define WIFI_SSID "****"
#endif

#if ! defined(WIFI_PASSWORD)
#define WIFI_PASSWORD "****"
#endif

#if ! defined(NTP_SERVER)
#define NTP_SERVER "pool.ntp.org"
#endif

// Value of time_t for 2000-01-01 00:00:00, used to detect invalid SNTP
// responses.
static const time_t EPOCH_2000_01_01 = 946684800;

// Number of millis to wait for a WiFi connection before doing a software
// reboot.
static const unsigned long REBOOT_TIMEOUT_MILLIS = 15000;

//-----------------------------------------------------------------------------

// C library day of week uses Sunday==0.
static const char* const DAYS_OF_WEEK[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

// Print the UTC time from time_t, using C library functions.
void printNowUsingCLibrary(time_t now) {
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  int year = timeinfo.tm_year + 1900; // tm_year starts in year 1900 (!)
  int month = timeinfo.tm_mon + 1; // tm_mon starts at 0 (!)
  int day = timeinfo.tm_mday; // tm_mday starts at 1 though (!)
  int hour = timeinfo.tm_hour;
  int mins = timeinfo.tm_min;
  int sec = timeinfo.tm_sec;
  int day_of_week = timeinfo.tm_wday; // tm_wday starts with Sunday=0
  const char* dow_string = DAYS_OF_WEEK[day_of_week];

  Serial.printf("%04d-%02d-%02dT%02d:%02d:%02d %s",
      year, month, day, hour, mins, sec, dow_string);
  Serial.println(F(" (C lib)"));
}

//-----------------------------------------------------------------------------

// Define 2 zone processors to handle 2 timezones (America/Los_Angeles,
// Europe/Paris) efficiently. It is possible to use only 1 to save memory, at
// the cost of slower performance. These are heavy-weight objects so should be
// created during the initialization of the app.
ExtendedZoneProcessor zoneProcessorLosAngeles;
ExtendedZoneProcessor zoneProcessorParis;

// Print the UTC time, America/Los_Angeles time, and Europe/Paris time using
// the AceTime library. TimeZone objects are light-weight and can be created on
// the fly.
void printNowUsingAceTime(time_t now) {
  // Utility to convert ISO day of week with Monday=1 to human readable string.
  DateStrings dateStrings;

  // Convert to UTC time.
  LocalDateTime ldt = LocalDateTime::forUnixSeconds64(now);
  ldt.printTo(Serial);
  Serial.print(' ');
  Serial.print(dateStrings.dayOfWeekLongString(ldt.dayOfWeek()));
  Serial.println(F(" (AceTime)"));

  // Convert Unix time to Los Angeles time.
  TimeZone tzLosAngeles = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessorLosAngeles);
  ZonedDateTime zdtLosAngeles = ZonedDateTime::forUnixSeconds64(
      now, tzLosAngeles);
  zdtLosAngeles.printTo(Serial);
  Serial.print(' ');
  Serial.println(dateStrings.dayOfWeekLongString(zdtLosAngeles.dayOfWeek()));

  // Convert Los Angeles time to Paris time.
  TimeZone tzParis = TimeZone::forZoneInfo(
      &zonedbx::kZoneEurope_Paris,
      &zoneProcessorParis);
  ZonedDateTime zdtParis = zdtLosAngeles.convertToTimeZone(tzParis);
  zdtParis.printTo(Serial);
  Serial.print(' ');
  Serial.println(dateStrings.dayOfWeekLongString(zdtParis.dayOfWeek()));
}

//-----------------------------------------------------------------------------

// Connect to WiFi. Sometimes the board will connect instantly. Sometimes it
// will struggle to connect. I don't know why. Performing a software reboot
// seems to help, but not always.
void configureWiFi() {
  Serial.print(F("Connecting to WiFi"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');

    // Detect timeout and reboot.
    if ((unsigned long) (millis() - startMillis) >= REBOOT_TIMEOUT_MILLIS) {
    #if defined(ESP8266)
      Serial.println(F("FAILED! Rebooting.."));
      delay(1000);
      ESP.reset();
    #elif defined(ESP32)
      Serial.println(F("FAILED! Rebooting.."));
      delay(1000);
      ESP.restart();
    #else
      Serial.println(F("FAILED! But cannot reboot.. continuing.."));
      delay(1000);
      startMillis = millis();
    #endif
    }
  }
  Serial.println();

}

// Configure the SNTP. Set the local time zone to be UTC, with no DST offset,
// because we will be using AceTime to perform the timezone conversions. The
// built-in timezone support provided by the ESP8266/ESP32 API has a number of
// deficiencies, and the API can be quite confusing.
//
// Sometimes the SNTP client never finishes initialization. In a production
// system, you may want a timeout. But if the sole purpose of the app is to get
// the time, then maybe there is no point in continuing if the SNTP client
// cannot be configured?
void configureSntp() {
  Serial.print(F("Configuring SNTP"));
  configTime(0 /*timezone*/, 0 /*dst_sec*/, NTP_SERVER);

  // Wait until SNTP stabilizes by ignoring values before year 2000.
  time_t now = 0;
  while (now < EPOCH_2000_01_01) {
    now = time(nullptr);
    delay(500);
    Serial.print('.');
  }
  Serial.println();
}

//-----------------------------------------------------------------------------

void setup() {
  delay(1000);
  Serial.begin(115200);

  configureWiFi();
  configureSntp();
}

void loop() {
  time_t now = time(nullptr);
  printNowUsingCLibrary(now);
  printNowUsingAceTime(now);
  Serial.println();

  delay(5000);
}
