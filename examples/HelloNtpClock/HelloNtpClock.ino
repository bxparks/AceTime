#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

static BasicZoneProcessor parisProcessor;
NtpClock ntpClock;

const char SSID[] = "...";
const char PASSWORD[] = "...";


void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  ntpClock.setup(SSID, PASSWORD);
  if (ntpClock.isSetup()) {
    Serial.println("WiFi connection failed... try again.");
  }
}

void loop() {
  acetime_t nowSeconds = ntpClock.getNow();

  SERIAL_PORT_MONITOR.print(F("Now Seconds: "));
  SERIAL_PORT_MONITOR.println(nowSeconds);

  auto parisTz = TimeZone::forZoneInfo(&zonedb::kZoneEurope_Paris,
        &parisProcessor);

  auto parisTime = ZonedDateTime::forEpochSeconds(nowSeconds, parisTz);

  SERIAL_PORT_MONITOR.println(F("=== Paris"));
  SERIAL_PORT_MONITOR.print(F("Time: "));
  parisTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  delay(10000); // wait 10 seconds
}
