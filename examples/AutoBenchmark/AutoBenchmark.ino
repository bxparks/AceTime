/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of DateTime:
 *
 *  - DateTime(seconds) constructor
 *  - DateTime::toDaysSinceEpoch()
 *  - DateTime::toSecondsSinceEpoch()
 *
 * This should run on all microcontrollers supported by the Arduino IDE.
 */

// AceRoutine.h Needed to activate SystemTimeSyncCoroutine and
// SystemTimeHeartbeatCoroutine.
#include <AceRoutine.h>
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  pinMode(LED_BENCHMARK, OUTPUT);

  Serial.print("sizeof(DateTime): ");
  Serial.println(sizeof(DateTime));

  Serial.print("sizeof(TimeZone): ");
  Serial.println(sizeof(TimeZone));

  Serial.print("sizeof(TimePeriod): ");
  Serial.println(sizeof(TimePeriod));

  Serial.print("sizeof(SystemTimeKeeper): ");
  Serial.println(sizeof(SystemTimeKeeper));

  Serial.print("sizeof(DS3231TimeKeeper): ");
  Serial.println(sizeof(DS3231TimeKeeper));

#ifdef ESP8266
  Serial.print("sizeof(NtpTimeProvider): ");
  Serial.println(sizeof(NtpTimeProvider));
#endif

  Serial.print("sizeof(SystemTimeSyncCoroutine): ");
  Serial.println(sizeof(SystemTimeSyncCoroutine));

  Serial.print("sizeof(SystemTimeHeartbeatCoroutine): ");
  Serial.println(sizeof(SystemTimeHeartbeatCoroutine));

  runBenchmarks();
}

void loop() {
}
