/*
 * A program to demonstrate using the ExtendedZoneManager with the entire
 * zoneinfo database loaded, creating 3 timezones in 3 different ways
 * (createForZoneInfo(), createForZoneName() and createForZoneId()),
 * then displaying the time in those zones. Should print the following:
 *
 * 2019-03-10T03:00:00-07:00[America/Los_Angeles]
 * 2019-03-10T10:00:00+00:00[Europe/London]
 * 2019-03-10T21:00:00+11:00[Australia/Sydney]
 */

#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// ESP32 does not define SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

// Create an ExtendedZoneManager with the entire TZ Database of Zone entries.
// Cache size of 3 means that it can support 3 concurrent timezones without
// performance penalties.
//
// Using an ExtendedZoneManager with the entire zonedbx::kZoneAndLinkRegistry
// consumes ~44kB of flash memory (AVR) and ~51kB (ESP8266). This program no
// longer fits on an Arduino Nano. It may be possible to make it fit using the
// BasicZoneManager and using zonedb::kZoneAndLinkRegistry (~24kB on AVR), or
// zonedb::kZoneRegistry (~19kB on AVR).
static const int CACHE_SIZE = 3;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  // Create America/Los_Angeles timezone by ZoneInfo.
  TimeZone losAngelesTz = manager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  ZonedDateTime losAngelesTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, losAngelesTz);
  losAngelesTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create Europe/London timezone by ZoneName.
  TimeZone londonTz = manager.createForZoneName("Europe/London");
  ZonedDateTime londonTime = losAngelesTime.convertToTimeZone(londonTz);
  londonTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create Australia/Sydney timezone by ZoneId.
  TimeZone sydneyTz = manager.createForZoneId(zonedb::kZoneIdAustralia_Sydney);
  ZonedDateTime sydneyTime = losAngelesTime.convertToTimeZone(sydneyTz);
  sydneyTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
