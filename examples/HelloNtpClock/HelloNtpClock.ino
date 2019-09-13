/*
 * A program to demonstrate the use of NtpClock using 2 instances. One instance
 * calls the setup() method with the WiFi SSID and password. The other instance
 * calls setup() with no arguments, which assumes that the WiFi connection was
 * setup by something else.
 *
 * Tested on ESP8266 and ESP32.
 */

#if !defined(ESP32) && !defined(ESP8266)
  #error This sketch works only for the ESP8266 and ESP32
#endif

#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

// Replace AUNITER_SSID and AUNITER_PASSWORD with your WiFi SSID and password.
// (I have a wrapper script that replaces these with the the correct values.
// You will have to replace them manually.)
// WARNING: For security, do NOT commit your ssid and password into a public
// source repository.
static const char SSID[] = AUNITER_SSID;
static const char PASSWORD[] = AUNITER_PASSWORD;

static BasicZoneProcessor parisProcessor;
static NtpClock ntpClock, ntpClock2;

void setup() {
  delay(1000);
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until Serial is ready - Leonardo/Micro
  SERIAL_PORT_MONITOR.println();

  auto parisTz = TimeZone::forZoneInfo(
      &zonedb::kZoneEurope_Paris, &parisProcessor);
  acetime_t nowSeconds;

  // Example of setting up clock without Wi-Fi configured in advance by
  // passing the SSID and PASSWORD into setup().
  SERIAL_PORT_MONITOR.println(
      F("+++ NTP clock 1: no Wi-Fi configured in advance +++"));
  ntpClock.setup(SSID, PASSWORD);
  if (!ntpClock.isSetup()) {
    SERIAL_PORT_MONITOR.println(F("WiFi connection failed... try again."));
    return;
  }
  nowSeconds = ntpClock.getNow();
  SERIAL_PORT_MONITOR.print(F("Now Seconds: "));
  SERIAL_PORT_MONITOR.println(nowSeconds);
  auto parisTime = ZonedDateTime::forEpochSeconds(nowSeconds, parisTz);
  SERIAL_PORT_MONITOR.print(F("=== Paris Time: "));
  parisTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Example of setting up clock with Wi-Fi already configured in advance by
  // calling the no-argument setup().
  delay(1000);
  SERIAL_PORT_MONITOR.println(
      F("+++ NTP clock 2: Wi-Fi configured in advance +++"));
  ntpClock2.setup();
  if (!ntpClock2.isSetup()) {
    SERIAL_PORT_MONITOR.println(F("Something went wrong."));
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
