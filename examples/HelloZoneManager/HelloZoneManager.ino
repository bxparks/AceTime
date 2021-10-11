/*
 * A program to demonstrate using the ZoneManager with the entire
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

// Create a BasicZoneManager with the entire TZ Database of ZONE entries. Use
// kZoneAndLinkRegistrySize and kZoneAndLinkRegistry to include LINK entries as
// well, at the cost of additional flash consumption. Cache size of 3 means that
// it can support 3 concurrent timezones without performance penalties.
static const int CACHE_SIZE = 3;
static BasicZoneManager<CACHE_SIZE> manager(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait Serial is ready - Leonardo/Micro

  // Create Los Angeles by ZoneInfo
  auto pacificTz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, pacificTz);
  pacificTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create London by ZoneName
  auto londonTz = manager.createForZoneName("Europe/London");
  auto londonTime = pacificTime.convertToTimeZone(londonTz);
  londonTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create Sydney by ZoneId
  auto sydneyTz = manager.createForZoneId(zonedb::kZoneIdAustralia_Sydney);
  auto sydneyTime = pacificTime.convertToTimeZone(sydneyTz);
  sydneyTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
