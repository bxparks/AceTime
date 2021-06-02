/*
 * Identical to HelloSystemClock, but using AceRoutine coroutines.
 * Should print the following on the SERIAL_PORT_MONITOR port every 2 seconds:
 *
 *   2019-06-17T19:50:00-07:00[America/Los_Angeles]
 *   2019-06-17T19:50:02-07:00[America/Los_Angeles]
 *   2019-06-17T19:50:04-07:00[America/Los_Angeles]
 *   ...
 */

#include <AceRoutine.h>  // activates SystemClockCoroutine
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_routine;

// ZoneProcessor instance should be created statically at initialization time.
static BasicZoneProcessor pacificProcessor;

// The 'referenceClock' is set to nullptr, so systemClockCoroutine does
// not actually do anything. The purpose of this program is to show how
// to structure the code if the 'referenceClock' was actually defined.
static SystemClockCoroutine systemClock(
    nullptr /*reference*/, nullptr /*backup*/);

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro

  systemClock.setup();

  // Creating timezones is cheap, so we can create them on the fly as needed.
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);

  // Set the SystemClock using these components.
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 6, 17, 19, 50, 0, pacificTz);
  systemClock.setNow(pacificTime.toEpochSeconds());

  CoroutineScheduler::setup();
}

void printCurrentTime() {
  acetime_t now = systemClock.getNow();

  // Create Pacific Time and print.
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);
  auto pacificTime = ZonedDateTime::forEpochSeconds(now, pacificTz);
  pacificTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();
}

COROUTINE(print) {
  COROUTINE_LOOP() {
    printCurrentTime();
    COROUTINE_DELAY(2000);
  }
}

void loop() {
  CoroutineScheduler::loop();
}
