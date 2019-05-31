#line 2 "CommonTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time::common;

// --------------------------------------------------------------------------

test(decToBcd) {
  assertEqual(0x00, decToBcd(0));
  assertEqual(0x10, decToBcd(10));
  assertEqual(0x23, decToBcd(23));
  assertEqual(0x99, decToBcd(99));
}

test(bcdToDec) {
  assertEqual(0, bcdToDec(0x00));
  assertEqual(10, bcdToDec(0x10));
  assertEqual(23, bcdToDec(0x23));
  assertEqual(99, bcdToDec(0x99));
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
