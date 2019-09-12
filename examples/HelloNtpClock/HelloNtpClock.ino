#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

static BasicZoneProcessor parisProcessor;
NtpClock ntpClock, ntpClock2;
acetime_t nowSeconds;

const char SSID[] = "...";
const char PASSWORD[] = "...";


void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  SERIAL_PORT_MONITOR.println("");

  auto parisTz = TimeZone::forZoneInfo(&zonedb::kZoneEurope_Paris, &parisProcessor);

  // Example of setting up clock without Wi-Fi configured in advance
  SERIAL_PORT_MONITOR.println(F("+++ NTP clock 1: no Wi-Fi configured in advance +++"));
  ntpClock.setup(SSID, PASSWORD);
  if (!ntpClock.isSetup()) {
    Serial.println("WiFi connection failed... try again.");
    return;
  }
  nowSeconds = ntpClock.getNow();
  SERIAL_PORT_MONITOR.print(F("Now Seconds: "));
  SERIAL_PORT_MONITOR.println(nowSeconds);
  auto parisTime = ZonedDateTime::forEpochSeconds(nowSeconds, parisTz);
  SERIAL_PORT_MONITOR.print(F("=== Paris Time: "));
  parisTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Example of setting up clock with Wi-Fi configured in advance
  delay(1000);
  SERIAL_PORT_MONITOR.println(F("+++ NTP clock 2: Wi-Fi configured in advance +++"));
  ntpClock2.setup();
  if (!ntpClock2.isSetup()) {
    Serial.println("Something went wrong.");
    return;
  }
  nowSeconds = ntpClock2.getNow();
  SERIAL_PORT_MONITOR.print(F("Now Seconds: "));
  SERIAL_PORT_MONITOR.println(nowSeconds);
  parisTime = ZonedDateTime::forEpochSeconds(nowSeconds, parisTz);
  SERIAL_PORT_MONITOR.print(F("=== Paris Time: "));
  parisTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();
}

void loop() {}
