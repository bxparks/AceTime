/*
 * A program to demonstrate the use of AceTime classes. It should print the
 * following on the SERIAL_PORT_MONITOR port:
 *
 * Epoch Seconds: 605527200
 * Unix Seconds: 1552212000
 * === Los_Angeles:
 * Time: 2019-03-10T03:00:00-07:00[America/Los_Angeles]
 * Day of Week: Sunday
 * Total UTC Offset: -07:00
 * Zone: America/Los_Angeles
 * Abbreviation: PDT
 * === London:
 * Time: 2019-03-10T10:00:00+00:00[Europe/London]
 * Zone: Europe/London
 * Abbreviation: GMT
 * === Compare ZonedDateTime
 * losAngelesTime.compareTo(londonTime): 0
 * losAngelesTime == londonTime: false
 *
 */

#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// ESP32 does not define SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

// ZoneProcessor instances should be created statically at initialization time.
static BasicZoneProcessor losAngelesProcessor;
static BasicZoneProcessor londonProcessor;

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  // TimeZone objects are light-weight and can be created on the fly.
  TimeZone losAngelesTz = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles,
      &losAngelesProcessor);
  TimeZone londonTz = TimeZone::forZoneInfo(
      &zonedb::kZoneEurope_London,
      &londonProcessor);

  // Create from components. 2019-03-10T03:00:00 is just after DST change in
  // Los Angeles (2am goes to 3am).
  ZonedDateTime startTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, losAngelesTz);

  SERIAL_PORT_MONITOR.print(F("Epoch Seconds: "));
  acetime_t epochSeconds = startTime.toEpochSeconds();
  SERIAL_PORT_MONITOR.println(epochSeconds);

  SERIAL_PORT_MONITOR.print(F("Unix Seconds: "));
  int64_t unixSeconds = startTime.toUnixSeconds64();
  SERIAL_PORT_MONITOR.println((int32_t) unixSeconds);

  SERIAL_PORT_MONITOR.println(F("=== Los Angeles"));
  auto losAngelesTime = ZonedDateTime::forEpochSeconds(
      epochSeconds, losAngelesTz);
  SERIAL_PORT_MONITOR.print(F("Time: "));
  losAngelesTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  SERIAL_PORT_MONITOR.print(F("Day of Week: "));
  SERIAL_PORT_MONITOR.println(
      DateStrings().dayOfWeekLongString(losAngelesTime.dayOfWeek()));

  // Print info about UTC offset
  TimeOffset offset = losAngelesTime.timeOffset();
  SERIAL_PORT_MONITOR.print(F("Total UTC Offset: "));
  offset.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Print info about the current time zone
  SERIAL_PORT_MONITOR.print(F("Zone: "));
  losAngelesTz.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  SERIAL_PORT_MONITOR.print(F("Abbreviation: "));
  SERIAL_PORT_MONITOR.print(losAngelesTz.getAbbrev(epochSeconds));
  SERIAL_PORT_MONITOR.println();

  // Create from epoch seconds. London is still on standard time.
  auto londonTime = ZonedDateTime::forEpochSeconds(epochSeconds, londonTz);

  SERIAL_PORT_MONITOR.println(F("=== London"));
  SERIAL_PORT_MONITOR.print(F("Time: "));
  londonTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Print info about the current time zone
  SERIAL_PORT_MONITOR.print(F("Zone: "));
  londonTz.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Print the current time zone abbreviation, e.g. "GMT" or "BST"
  SERIAL_PORT_MONITOR.print(F("Abbreviation: "));
  SERIAL_PORT_MONITOR.print(londonTz.getAbbrev(epochSeconds));
  SERIAL_PORT_MONITOR.println();

  SERIAL_PORT_MONITOR.println(F("=== Compare ZonedDateTime"));
  SERIAL_PORT_MONITOR.print(F("losAngelesTime.compareTo(londonTime): "));
  SERIAL_PORT_MONITOR.println(losAngelesTime.compareTo(londonTime));
  SERIAL_PORT_MONITOR.print(F("losAngelesTime == londonTime: "));
  SERIAL_PORT_MONITOR.println(
      (losAngelesTime == londonTime) ? "true" : "false");

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
