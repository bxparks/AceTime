/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of DateTime and LocalDate.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#include <AceRoutine.h> // activate SystemTime coroutines
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;
using namespace ace_time::provider;

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  pinMode(LED_BENCHMARK, OUTPUT);

  // ace_time primitives

  Serial.print("sizeof(LocalDate): ");
  Serial.println(sizeof(LocalDate));

  Serial.print("sizeof(ZoneOffset): ");
  Serial.println(sizeof(ZoneOffset));

  Serial.print("sizeof(OffsetDateTime): ");
  Serial.println(sizeof(OffsetDateTime));

  Serial.print("sizeof(DateTime): ");
  Serial.println(sizeof(DateTime));

  Serial.print("sizeof(TimeZone): ");
  Serial.println(sizeof(TimeZone));

  Serial.print("sizeof(TimePeriod): ");
  Serial.println(sizeof(TimePeriod));

  // ace_time::provider classes

  Serial.print("sizeof(SystemTimeKeeper): ");
  Serial.println(sizeof(SystemTimeKeeper));

  Serial.print("sizeof(DS3231TimeKeeper): ");
  Serial.println(sizeof(DS3231TimeKeeper));

#if defined(ESP8266) || defined(ESP32)
  Serial.print("sizeof(NtpTimeProvider): ");
  Serial.println(sizeof(NtpTimeProvider));
#endif

  Serial.print("sizeof(SystemTimeSyncLoop): ");
  Serial.println(sizeof(SystemTimeSyncLoop));

  Serial.print("sizeof(SystemTimeHeartbeatLoop): ");
  Serial.println(sizeof(SystemTimeHeartbeatLoop));

  Serial.print("sizeof(SystemTimeSyncCoroutine): ");
  Serial.println(sizeof(SystemTimeSyncCoroutine));

  Serial.print("sizeof(SystemTimeHeartbeatCoroutine): ");
  Serial.println(sizeof(SystemTimeHeartbeatCoroutine));

  runBenchmarks();
}

void loop() {
}
